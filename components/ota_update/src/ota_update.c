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

static void emit_event(ota_update_event_cb_t   callback,
                       void*                   user_data,
                       ota_update_event_type_t type,
                       size_t                  bytes_downloaded,
                       size_t                  image_size,
                       esp_err_t               error)
{
    if (callback == NULL)
    {
        return;
    }
    ota_update_event_t event = {
        .type             = type,
        .bytes_downloaded = bytes_downloaded,
        .image_size       = image_size,
        .error            = error,
    };
    callback(&event, user_data);
}

esp_err_t ota_update_perform_with_callback(const char*           url,
                                           bool                  reboot_on_success,
                                           ota_update_event_cb_t callback,
                                           void*                 user_data)
{
    if (!url)
    {
        emit_event(callback, user_data, OTA_UPDATE_EVENT_ERROR, 0U, 0U, ESP_ERR_INVALID_ARG);
        return ESP_ERR_INVALID_ARG;
    }

    emit_event(callback, user_data, OTA_UPDATE_EVENT_START, 0U, 0U, ESP_OK);

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
        emit_event(callback, user_data, OTA_UPDATE_EVENT_ERROR, 0U, 0U, err);
        return err;
    }

    while ((err = esp_https_ota_perform(ota_handle)) == ESP_ERR_HTTPS_OTA_IN_PROGRESS)
    {
        size_t image_size       = esp_https_ota_get_image_size(ota_handle);
        size_t bytes_downloaded = esp_https_ota_get_image_len_read(ota_handle);
        emit_event(
            callback, user_data, OTA_UPDATE_EVENT_PROGRESS, bytes_downloaded, image_size, ESP_OK);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    if (err == ESP_OK && esp_https_ota_is_complete_data_received(ota_handle))
    {
        ESP_LOGI(TAG, "OTA download complete");
    }
    else
    {
        ESP_LOGE(TAG, "OTA perform failed: 0x%x", (unsigned int)err);
        emit_event(callback,
                   user_data,
                   OTA_UPDATE_EVENT_ERROR,
                   esp_https_ota_get_image_len_read(ota_handle),
                   esp_https_ota_get_image_size(ota_handle),
                   err);
    }

    esp_err_t finish_err = esp_https_ota_finish(ota_handle);
    if (finish_err != ESP_OK)
    {
        ESP_LOGE(TAG, "OTA finish failed: 0x%x", (unsigned int)finish_err);
        emit_event(callback,
                   user_data,
                   OTA_UPDATE_EVENT_ERROR,
                   esp_https_ota_get_image_len_read(ota_handle),
                   esp_https_ota_get_image_size(ota_handle),
                   finish_err);
        return finish_err;
    }

    if (err == ESP_OK && reboot_on_success)
    {
        ESP_LOGI(TAG, "Rebooting after OTA update");
        emit_event(callback,
                   user_data,
                   OTA_UPDATE_EVENT_COMPLETED,
                   esp_https_ota_get_image_len_read(ota_handle),
                   esp_https_ota_get_image_size(ota_handle),
                   ESP_OK);
        esp_restart();
    }
    else if (err == ESP_OK)
    {
        emit_event(callback,
                   user_data,
                   OTA_UPDATE_EVENT_COMPLETED,
                   esp_https_ota_get_image_len_read(ota_handle),
                   esp_https_ota_get_image_size(ota_handle),
                   ESP_OK);
    }

    return err;
}

esp_err_t ota_update_perform(const char* url, bool reboot_on_success)
{
    return ota_update_perform_with_callback(url, reboot_on_success, NULL, NULL);
}
