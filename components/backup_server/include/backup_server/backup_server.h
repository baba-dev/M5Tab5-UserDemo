/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "esp_err.h"
#include "esp_http_server.h"
#include "settings_core/app_cfg.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct
    {
        httpd_handle_t   httpd;
        const app_cfg_t* cfg;
    } backup_server_handle_t;

    esp_err_t backup_server_start(backup_server_handle_t* handle, const app_cfg_t* cfg);
    void      backup_server_stop(backup_server_handle_t* handle);

#ifdef __cplusplus
}
#endif
