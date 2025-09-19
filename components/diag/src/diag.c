/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "diag/diag.h"

#include <stdio.h>

#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "mdns.h"

static const char* TAG = "diag";

static void
emit_diag_event(diag_event_cb_t callback, void* user_data, diag_event_type_t type, esp_err_t error)
{
    if (callback == NULL)
    {
        return;
    }
    diag_event_t event = {
        .type  = type,
        .error = error,
    };
    callback(&event, user_data);
}

static esp_err_t diag_register_mdns(const app_cfg_t* cfg)
{
    esp_err_t err = mdns_init();
    if (err == ESP_ERR_INVALID_STATE)
    {
        err = ESP_OK;
    }
    if (err != ESP_OK)
    {
        return err;
    }

    mdns_hostname_set(cfg->network.hostname);
    mdns_instance_name_set("M5Tab5");
    mdns_service_add("M5Tab5", "_http", "_tcp", 80, NULL, 0);
    return ESP_OK;
}

static esp_err_t health_handler(httpd_req_t* req)
{
    int64_t uptime_ms = esp_timer_get_time() / 1000;
    char    payload[128];
    int     written = snprintf(payload,
                           sizeof(payload),
                           "{\"uptime_ms\":%lld,\"heap\":%u}",
                           (long long)uptime_ms,
                           esp_get_free_heap_size());
    if (written < 0)
    {
        return ESP_FAIL;
    }
    httpd_resp_set_type(req, "application/json");
    return httpd_resp_send(req, payload, HTTPD_RESP_USE_STRLEN);
}

esp_err_t
diag_start(const app_cfg_t* cfg, diag_handles_t* handles, diag_event_cb_t callback, void* user_data)
{
    if (!cfg || !handles)
    {
        return ESP_ERR_INVALID_ARG;
    }

    emit_diag_event(callback, user_data, DIAG_EVENT_STARTING, ESP_OK);

    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.uri_match_fn   = httpd_uri_match_simple;

    esp_err_t err = httpd_start(&handles->httpd, &config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start diagnostics server: 0x%x", (unsigned int)err);
        handles->httpd = NULL;
        emit_diag_event(callback, user_data, DIAG_EVENT_ERROR, err);
        return err;
    }

    emit_diag_event(callback, user_data, DIAG_EVENT_HTTP_READY, ESP_OK);

    httpd_uri_t health_uri = {
        .uri      = "/health",
        .method   = HTTP_GET,
        .handler  = health_handler,
        .user_ctx = NULL,
    };
    httpd_register_uri_handler(handles->httpd, &health_uri);

    esp_mqtt_client_config_t mqtt_config = {
        .broker.address.uri                  = cfg->mqtt.broker_uri,
        .credentials.username                = cfg->mqtt.username,
        .credentials.authentication.password = cfg->mqtt.password,
    };
    handles->mqtt = esp_mqtt_client_init(&mqtt_config);
    if (handles->mqtt)
    {
        esp_err_t mqtt_err = esp_mqtt_client_start(handles->mqtt);
        if (mqtt_err != ESP_OK)
        {
            ESP_LOGE(TAG, "MQTT start failed: 0x%x", (unsigned int)mqtt_err);
            emit_diag_event(callback, user_data, DIAG_EVENT_ERROR, mqtt_err);
            return mqtt_err;
        }
        emit_diag_event(callback, user_data, DIAG_EVENT_MQTT_STARTED, ESP_OK);
    }
    else
    {
        emit_diag_event(callback, user_data, DIAG_EVENT_WARNING, ESP_ERR_NO_MEM);
    }

    err = diag_register_mdns(cfg);
    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "mDNS registration failed: 0x%x", (unsigned int)err);
        emit_diag_event(callback, user_data, DIAG_EVENT_WARNING, err);
    }

    ESP_LOGI(TAG, "Diagnostics server ready");
    return ESP_OK;
}

void diag_stop(diag_handles_t* handles)
{
    if (!handles)
    {
        return;
    }
    if (handles->mqtt)
    {
        esp_mqtt_client_stop(handles->mqtt);
        esp_mqtt_client_destroy(handles->mqtt);
        handles->mqtt = NULL;
    }
    if (handles->httpd)
    {
        httpd_stop(handles->httpd);
        handles->httpd = NULL;
    }
    mdns_free();
}
