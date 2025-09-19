/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "backup_server/backup_server.h"

#include <stdlib.h>

#include "backup_server/backup_format.h"
#include "esp_log.h"

static const char* TAG = "backup_server";

static esp_err_t backup_handler(httpd_req_t* req)
{
    backup_server_handle_t* handle = (backup_server_handle_t*)req->user_ctx;
    if (!handle || !handle->cfg)
    {
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Config unavailable");
    }

    size_t needed = backup_server_calculate_json_size(handle->cfg);
    if (needed == 0U)
    {
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Invalid config");
    }

    char* buffer = (char*)malloc(needed);
    if (!buffer)
    {
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "No memory");
    }

    esp_err_t err = backup_server_write_json(handle->cfg, buffer, needed);
    if (err != ESP_OK)
    {
        free(buffer);
        return httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Encoding failed");
    }

    httpd_resp_set_type(req, "application/json");
    esp_err_t resp_err = httpd_resp_send(req, buffer, HTTPD_RESP_USE_STRLEN);
    free(buffer);
    return resp_err;
}

esp_err_t backup_server_start(backup_server_handle_t* handle, const app_cfg_t* cfg)
{
    if (!handle || !cfg)
    {
        return ESP_ERR_INVALID_ARG;
    }

    httpd_config_t config   = HTTPD_DEFAULT_CONFIG();
    config.max_uri_handlers = 4;

    esp_err_t err = httpd_start(&handle->httpd, &config);
    if (err != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to start backup server: 0x%x", (unsigned int)err);
        handle->httpd = NULL;
        return err;
    }

    handle->cfg     = cfg;
    httpd_uri_t uri = {
        .uri      = "/backup.json",
        .method   = HTTP_GET,
        .handler  = backup_handler,
        .user_ctx = handle,
    };
    httpd_register_uri_handler(handle->httpd, &uri);
    ESP_LOGI(TAG, "Backup endpoint ready at /backup.json");
    return ESP_OK;
}

void backup_server_stop(backup_server_handle_t* handle)
{
    if (!handle)
    {
        return;
    }
    if (handle->httpd)
    {
        httpd_stop(handle->httpd);
        handle->httpd = NULL;
    }
    handle->cfg = NULL;
}
