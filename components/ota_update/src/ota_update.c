/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ota_update/ota_update.h"

#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_https_ota.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "ota_update";

esp_err_t ota_update_perform(const char* url, bool reboot_on_success)
{
    if (!url)
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_http_client_config_t http_config = {
        .url               = url,
        .timeout_ms        = 10000,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &http_config,
    };

    esp_https_ota_handle_t ota_handle = NULL;
    esp_err_t              err        = esp_https_ota_begin(&ota_config, &ota_handle);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "OTA begin failed: 0x%x", (unsigned int)err);
        return err;
    }

    while ((err = esp_https_ota_perform(ota_handle)) == ESP_ERR_HTTPS_OTA_IN_PROGRESS)
    {
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    if (err == ESP_OK && esp_https_ota_is_complete_data_received(ota_handle))
    {
        ESP_LOGI(TAG, "OTA download complete");
    }
    else
    {
        ESP_LOGE(TAG, "OTA perform failed: 0x%x", (unsigned int)err);
    }

    esp_err_t finish_err = esp_https_ota_finish(ota_handle);
    if (finish_err != ESP_OK)
    {
        ESP_LOGE(TAG, "OTA finish failed: 0x%x", (unsigned int)finish_err);
        return finish_err;
    }

    if (err == ESP_OK && reboot_on_success)
    {
        ESP_LOGI(TAG, "Rebooting after OTA update");
        esp_restart();
    }
    return err;
}
