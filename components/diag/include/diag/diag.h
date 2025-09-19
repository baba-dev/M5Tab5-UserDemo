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

    typedef enum
    {
        DIAG_EVENT_STARTING = 0,
        DIAG_EVENT_HTTP_READY,
        DIAG_EVENT_MQTT_STARTED,
        DIAG_EVENT_WARNING,
        DIAG_EVENT_ERROR,
    } diag_event_type_t;

    typedef struct
    {
        diag_event_type_t type;
        esp_err_t         error;
    } diag_event_t;

    typedef void (*diag_event_cb_t)(const diag_event_t* event, void* user_data);

    esp_err_t diag_start(const app_cfg_t* cfg,
                         diag_handles_t*  handles,
                         diag_event_cb_t  callback,
                         void*            user_data);
    void      diag_stop(diag_handles_t* handles);

#ifdef __cplusplus
}
#endif
