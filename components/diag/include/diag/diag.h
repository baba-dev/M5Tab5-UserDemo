/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "esp_err.h"
#include "esp_http_server.h"
#include "mqtt_client.h"
#include "settings_core/app_cfg.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        httpd_handle_t           httpd;
        esp_mqtt_client_handle_t mqtt;
    } diag_handles_t;

    esp_err_t diag_start(const app_cfg_t* cfg, diag_handles_t* handles);
    void      diag_stop(diag_handles_t* handles);

#ifdef __cplusplus
}
#endif
