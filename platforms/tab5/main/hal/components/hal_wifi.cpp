/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include <algorithm>
#include <bsp/m5stack_tab5.h>
#include <cstring>
#include <esp_check.h>
#include <esp_event.h>
#include <esp_hosted_api.h>
#include <esp_log.h>
#include <esp_netif.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <mooncake_log.h>
#include <nvs_flash.h>

#include "hal/hal_esp32.h"

#define TAG "wifi"

#define WIFI_SSID    "M5Tab5-UserDemo-WiFi"
#define WIFI_PASS    ""
#define MAX_STA_CONN 4

namespace
{

    struct WifiRuntimeState
    {
        bool attempted = false;
        bool started   = false;
        bool failed    = false;
    };

    WifiRuntimeState& wifi_state()
    {
        static WifiRuntimeState state;
        return state;
    }

    portMUX_TYPE& wifi_state_spinlock()
    {
        static portMUX_TYPE spinlock = portMUX_INITIALIZER_UNLOCKED;
        return spinlock;
    }

    struct NetifGuards
    {
        bool netif_ready      = false;
        bool event_loop_ready = false;
    };

    NetifGuards& netif_guards()
    {
        static NetifGuards guards;
        return guards;
    }

    esp_err_t ensure_nvs_initialised()
    {
        static bool initialised = false;
        if (initialised)
        {
            return ESP_OK;
        }

        esp_err_t err = nvs_flash_init();
        if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND)
        {
            ESP_RETURN_ON_ERROR(nvs_flash_erase(), TAG, "Failed to erase NVS");
            err = nvs_flash_init();
        }
        ESP_RETURN_ON_ERROR(err, TAG, "Failed to initialise NVS");

        initialised = true;
        return ESP_OK;
    }

    esp_err_t ensure_netif_ready()
    {
        auto& guards = netif_guards();

        if (!guards.netif_ready)
        {
            esp_err_t err = esp_netif_init();
            if (err == ESP_ERR_INVALID_STATE)
            {
                ESP_LOGW(TAG, "esp_netif already initialised");
            }
            else
            {
                ESP_RETURN_ON_ERROR(err, TAG, "Failed to initialise esp_netif");
            }
            guards.netif_ready = true;
        }

        if (!guards.event_loop_ready)
        {
            esp_err_t err = esp_event_loop_create_default();
            if (err == ESP_ERR_INVALID_STATE)
            {
                ESP_LOGW(TAG, "Event loop already created");
            }
            else
            {
                ESP_RETURN_ON_ERROR(err, TAG, "Failed to create default event loop");
            }
            guards.event_loop_ready = true;
        }

        return ESP_OK;
    }

    esp_err_t start_softap()
    {
        ESP_RETURN_ON_ERROR(ensure_nvs_initialised(), TAG, "NVS init failed");
        ESP_RETURN_ON_ERROR(ensure_netif_ready(), TAG, "netif init failed");

        esp_netif_t* netif = esp_netif_create_default_wifi_ap();
        if (netif == nullptr)
        {
            ESP_LOGE(TAG, "Failed to create AP netif");
            return ESP_FAIL;
        }

        wifi_init_config_t cfg                    = WIFI_INIT_CONFIG_DEFAULT();
        esp_err_t          err                    = esp_wifi_init(&cfg);
        bool               wifi_newly_initialised = false;
        if (err == ESP_ERR_WIFI_INIT_STATE)
        {
            ESP_LOGW(TAG, "Wi-Fi already initialised");
        }
        else if (err == ESP_OK)
        {
            wifi_newly_initialised = true;
        }
        else
        {
            esp_netif_destroy(netif);
            return err;
        }

        auto cleanup = [&](esp_err_t failure)
        {
            if (wifi_newly_initialised)
            {
                esp_wifi_stop();
                esp_wifi_deinit();
            }
            if (netif != nullptr)
            {
                esp_netif_destroy(netif);
                netif = nullptr;
            }
            return failure;
        };

        wifi_config_t wifi_config = {};
        std::fill(std::begin(wifi_config.ap.ssid), std::end(wifi_config.ap.ssid), 0);
        std::fill(std::begin(wifi_config.ap.password), std::end(wifi_config.ap.password), 0);
        std::strncpy(reinterpret_cast<char*>(wifi_config.ap.ssid),
                     WIFI_SSID,
                     sizeof(wifi_config.ap.ssid) - 1);
        wifi_config.ap.ssid_len       = std::strlen(WIFI_SSID);
        wifi_config.ap.authmode       = WIFI_AUTH_OPEN;
        wifi_config.ap.max_connection = MAX_STA_CONN;
        wifi_config.ap.channel        = 1;

        err = esp_wifi_set_mode(WIFI_MODE_AP);
        if (err == ESP_ERR_NOT_SUPPORTED)
        {
            ESP_LOGW(TAG, "SoftAP mode not supported by remote Wi-Fi firmware");
            return cleanup(ESP_ERR_NOT_SUPPORTED);
        }
        if (err != ESP_OK)
        {
            return cleanup(err);
        }

        err = esp_wifi_set_config(WIFI_IF_AP, &wifi_config);
        if (err != ESP_OK)
        {
            return cleanup(err);
        }

        err = esp_wifi_start();
        if (err == ESP_ERR_WIFI_CONN)
        {
            ESP_LOGW(TAG, "Wi-Fi already started");
            esp_netif_destroy(netif);
            netif = nullptr;
            return ESP_OK;
        }
        if (err != ESP_OK)
        {
            return cleanup(err);
        }

        // Ownership of the netif transfers to esp-netif after a successful start.
        netif = nullptr;

        ESP_LOGI(TAG, "SoftAP started (SSID:%s)", WIFI_SSID);
        return ESP_OK;
    }

}  // namespace

bool HalEsp32::wifi_init()
{
    auto&         state    = wifi_state();
    portMUX_TYPE& spinlock = wifi_state_spinlock();

    portENTER_CRITICAL(&spinlock);
    bool already_started   = state.started;
    bool previously_failed = state.failed;
    bool in_progress       = state.attempted && !state.started && !state.failed;
    if (!state.attempted)
    {
        state.attempted = true;
    }
    portEXIT_CRITICAL(&spinlock);

    if (already_started)
    {
        return true;
    }
    if (previously_failed)
    {
        ESP_LOGW(TAG, "Skipping Wi-Fi init after previous failure");
        return false;
    }
    if (in_progress)
    {
        ESP_LOGW(TAG, "Wi-Fi init already in progress");
        return false;
    }

    mclog::tagInfo(TAG, "wifi init");

    bsp_set_wifi_power_enable(true);

    // The hosted C6 coprocessor requires a guard time after power up before it
    // responds to the transport reset sequence.  Without the delay the first RPC
    // request races the SDIO link initialisation and the transport driver frees an
    // uninitialised buffer, crashing the TLSF heap.  Give the module time to boot
    // and then explicitly request a transport re-sync before touching the Wi-Fi
    // stack.
    constexpr TickType_t kPowerOnGuardDelay   = pdMS_TO_TICKS(250);
    constexpr TickType_t kPostResetGuardDelay = pdMS_TO_TICKS(400);
    constexpr TickType_t kRetryBackoffDelay   = pdMS_TO_TICKS(150);
    constexpr int        kMaxTransportRetries = 3;

    vTaskDelay(kPowerOnGuardDelay);

    esp_err_t host_err           = ESP_FAIL;
    esp_err_t softap_err         = ESP_FAIL;
    bool      softap_started     = false;
    bool      softap_unsupported = false;

    for (int attempt = 0; attempt < kMaxTransportRetries; ++attempt)
    {
        if (attempt > 0)
        {
            ESP_LOGW(TAG,
                     "Retrying ESP-Hosted transport bring-up (%d/%d)",
                     attempt + 1,
                     kMaxTransportRetries);
            vTaskDelay(kRetryBackoffDelay * static_cast<TickType_t>(attempt));
        }

        host_err = esp_hosted_slave_reset();
        if (host_err != ESP_OK)
        {
            ESP_LOGE(TAG,
                     "ESP-Hosted reset failed on attempt %d/%d: %s",
                     attempt + 1,
                     kMaxTransportRetries,
                     esp_err_to_name(host_err));
            continue;
        }

        vTaskDelay(kPostResetGuardDelay);

        softap_err = start_softap();
        if (softap_err == ESP_OK)
        {
            softap_started = true;
            break;
        }

        if (softap_err == ESP_ERR_NOT_SUPPORTED)
        {
            softap_unsupported = true;
            break;
        }

        ESP_LOGE(TAG,
                 "SoftAP start failed on attempt %d/%d: %s",
                 attempt + 1,
                 kMaxTransportRetries,
                 esp_err_to_name(softap_err));

        if (softap_err == ESP_ERR_TIMEOUT || softap_err == ESP_FAIL)
        {
            esp_wifi_stop();
            esp_wifi_deinit();
        }
    }

    if (!softap_started)
    {
        portENTER_CRITICAL(&spinlock);
        state.failed = true;
        portEXIT_CRITICAL(&spinlock);
        bsp_set_wifi_power_enable(false);

        if (softap_unsupported)
        {
            ESP_LOGW(TAG, "SoftAP unsupported on hosted slave; Wi-Fi will stay disabled");
        }
        else if (host_err != ESP_OK)
        {
            ESP_LOGE(TAG,
                     "Failed to bring ESP-Hosted transport up after %d attempts: %s",
                     kMaxTransportRetries,
                     esp_err_to_name(host_err));
        }
        else
        {
            ESP_LOGE(TAG,
                     "Failed to start Wi-Fi after %d attempts: %s",
                     kMaxTransportRetries,
                     esp_err_to_name(softap_err));
        }
        return false;
    }

    portENTER_CRITICAL(&spinlock);
    state.started = true;
    portEXIT_CRITICAL(&spinlock);
    return true;
}

void HalEsp32::setExtAntennaEnable(bool enable)
{
    _ext_antenna_enable = enable;
    mclog::tagInfo(TAG, "set ext antenna enable: {}", _ext_antenna_enable);
    bsp_set_ext_antenna_enable(_ext_antenna_enable);
}

bool HalEsp32::getExtAntennaEnable()
{
    return _ext_antenna_enable;
}

void HalEsp32::startWifiAp()
{
    if (!wifi_init())
    {
        mclog::tagWarn(TAG, "Wi-Fi AP init failed");
    }
}
