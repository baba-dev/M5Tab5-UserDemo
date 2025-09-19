/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "connection_tester/connection_tester.h"

#include "esp_crt_bundle.h"
#include "esp_http_client.h"
#include "esp_log.h"

static const char* TAG = "conn_tester";

esp_err_t connection_tester_http_get(const char* url, int timeout_ms, int* status_code)
{
    if (!url)
    {
        return ESP_ERR_INVALID_ARG;
    }

    esp_http_client_config_t config = {
        .url               = url,
        .timeout_ms        = timeout_ms,
        .crt_bundle_attach = esp_crt_bundle_attach,
    };
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (!client)
    {
        return ESP_ERR_NO_MEM;
    }

    esp_err_t err = esp_http_client_perform(client);
    if (err == ESP_OK)
    {
        int http_status = esp_http_client_get_status_code(client);
        if (status_code)
        {
            *status_code = http_status;
        }
        ESP_LOGI(TAG, "HTTP GET %s -> %d", url, http_status);
    }
    else
    {
        ESP_LOGW(TAG, "HTTP GET %s failed: 0x%x", url, (unsigned int)err);
    }
    esp_http_client_cleanup(client);
    return err;
}
