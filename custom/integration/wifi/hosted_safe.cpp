/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "integration/wifi/hosted_safe.h"

#include <driver/sdmmc_host.h>
#include <esp_err.h>
#include <esp_log.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "esp_hosted.h"

namespace custom::integration::wifi
{
    namespace
    {
        constexpr const char* kTag = "hosted-safe";

        constexpr TickType_t kSlaveBootGuardDelay = pdMS_TO_TICKS(150);

        void cleanup_transport()
        {
            const esp_err_t deinit_err = esp_hosted_deinit();
            if (deinit_err != ESP_OK && deinit_err != ESP_ERR_INVALID_STATE)
            {
                ESP_LOGW(kTag, "esp_hosted_deinit reported: %s", esp_err_to_name(deinit_err));
            }

            const esp_err_t sdmmc_err = sdmmc_host_deinit();
            if (sdmmc_err != ESP_OK && sdmmc_err != ESP_ERR_INVALID_STATE)
            {
                ESP_LOGW(kTag, "sdmmc_host_deinit reported: %s", esp_err_to_name(sdmmc_err));
            }
        }
    }  // namespace

    bool HostedSafeStart()
    {
        vTaskDelay(kSlaveBootGuardDelay);

        esp_err_t err = esp_hosted_init();
        if (err != ESP_OK)
        {
            ESP_LOGE(kTag, "esp_hosted_init failed: %s", esp_err_to_name(err));
            cleanup_transport();
            return false;
        }

        wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
        err                         = esp_wifi_remote_init(&wifi_cfg);
        if (err != ESP_OK)
        {
            ESP_LOGE(kTag, "esp_wifi_remote_init failed: %s", esp_err_to_name(err));
            const esp_err_t remote_err = esp_wifi_remote_deinit();
            if (remote_err != ESP_OK && remote_err != ESP_ERR_INVALID_STATE)
            {
                ESP_LOGW(kTag, "esp_wifi_remote_deinit reported: %s", esp_err_to_name(remote_err));
            }
            cleanup_transport();
            return false;
        }

        ESP_LOGI(kTag, "Hosted link established");
        return true;
    }

}  // namespace custom::integration::wifi
