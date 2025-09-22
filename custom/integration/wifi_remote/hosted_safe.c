/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "integration/wifi_remote/hosted_safe.h"

#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

#if CONFIG_APP_ENABLE_WIFI_HOSTED
#    include "esp_hosted_api.h"
#    include "esp_wifi.h"
#    include "esp_wifi_remote.h"
#endif

static const char* TAG            = "hosted_safe";
static bool        s_hosted_ready = false;  // Guarded by CONFIG_APP_ENABLE_WIFI_HOSTED

#if CONFIG_APP_ENABLE_WIFI_HOSTED
static void slave_pulse_reset(void)
{
    const gpio_num_t    rst = 54;
    const gpio_config_t io  = {
         .pin_bit_mask = 1ULL << rst,
         .mode         = GPIO_MODE_OUTPUT,
         .pull_up_en   = 0,
         .pull_down_en = 0,
         .intr_type    = GPIO_INTR_DISABLE,
    };
    gpio_config(&io);
    gpio_set_level(rst, 0);
    vTaskDelay(pdMS_TO_TICKS(5));
    gpio_set_level(rst, 1);
}
#endif

bool hosted_try_init_with_retries(void)
{
#if !CONFIG_APP_ENABLE_WIFI_HOSTED
    ESP_LOGI(TAG, "ESP-Hosted disabled via Kconfig.");
    return false;
#else
    if (s_hosted_ready)
    {
        return true;
    }

    int retries = CONFIG_M5TAB5_HOSTED_BOOT_RETRIES;
    while (retries-- >= 0)
    {
        ESP_LOGI(
            TAG, "Resetting SDIO slave and waiting %d ms ...", CONFIG_M5TAB5_HOSTED_BOOT_DELAY_MS);
        slave_pulse_reset();
        vTaskDelay(pdMS_TO_TICKS(CONFIG_M5TAB5_HOSTED_BOOT_DELAY_MS));

        esp_err_t err = esp_hosted_init();
        if (err != ESP_OK)
        {
            ESP_LOGW(TAG, "esp_hosted_init failed (%s)", esp_err_to_name(err));
        }
        else
        {
            ESP_LOGI(TAG, "ESP-Hosted initialized.");
            s_hosted_ready = true;

            wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
            err                         = esp_wifi_remote_init(&wifi_cfg);
            if (err != ESP_OK)
            {
                ESP_LOGW(TAG,
                         "esp_wifi_remote_init failed (%s), continuing without Wi-Fi.",
                         esp_err_to_name(err));
                hosted_deinit_safe();
                s_hosted_ready = false;
            }
        }

        if (s_hosted_ready)
        {
            return true;
        }

        ESP_LOGW(TAG, "ESP-Hosted init failed; %d retry(ies) left.", retries);
        vTaskDelay(pdMS_TO_TICKS(250));
    }

    ESP_LOGW(TAG, "ESP-Hosted not available; booting without Wi-Fi.");
    return false;
#endif
}

void hosted_deinit_safe(void)
{
#if CONFIG_APP_ENABLE_WIFI_HOSTED
    if (!s_hosted_ready)
    {
        return;
    }

    esp_wifi_remote_deinit();
    esp_hosted_deinit();
    s_hosted_ready = false;
#endif
}
