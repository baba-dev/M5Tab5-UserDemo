/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "net_sntp/net_sntp.h"

#include "esp_log.h"
#include "esp_netif_sntp.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"

static const char* TAG = "net_sntp";

esp_err_t net_sntp_start(const char* server, bool wait_for_sync)
{
    const char* servers[1] = {server ? server : "pool.ntp.org"};

    esp_sntp_config_t config = ESP_NETIF_SNTP_DEFAULT_CONFIG_MULTIPLE(servers);
    config.sync_cb           = NULL;

    esp_err_t err = esp_netif_sntp_init(&config);
    if (err == ESP_ERR_INVALID_STATE)
    {
        ESP_LOGI(TAG, "SNTP already initialised");
        err = ESP_OK;
    }
    if (err != ESP_OK)
    {
        return err;
    }

    err = esp_netif_sntp_start();
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start SNTP: 0x%x", (unsigned int)err);
        return err;
    }

    if (wait_for_sync)
    {
        err = esp_netif_sntp_sync_wait(pdMS_TO_TICKS(5000));
        if (err != ESP_OK)
        {
            ESP_LOGW(TAG, "SNTP sync wait timed out: 0x%x", (unsigned int)err);
        }
    }
    wifi_mode_t mode = WIFI_MODE_NULL;
    esp_wifi_get_mode(&mode);
    ESP_LOGI(TAG, "SNTP running (wifi mode=%d)", mode);
    return err;
}

void net_sntp_stop(void)
{
    esp_netif_sntp_stop();
    esp_netif_sntp_deinit();
}
