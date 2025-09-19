/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        OTA_UPDATE_EVENT_START = 0,
        OTA_UPDATE_EVENT_PROGRESS,
        OTA_UPDATE_EVENT_COMPLETED,
        OTA_UPDATE_EVENT_ERROR,
    } ota_update_event_type_t;

    typedef struct
    {
        ota_update_event_type_t type;
        size_t                  bytes_downloaded;
        size_t                  image_size;
        esp_err_t               error;
    } ota_update_event_t;

    typedef void (*ota_update_event_cb_t)(const ota_update_event_t* event, void* user_data);

    esp_err_t ota_update_perform_with_callback(const char*           url,
                                               bool                  reboot_on_success,
                                               ota_update_event_cb_t callback,
                                               void*                 user_data);
    esp_err_t ota_update_perform(const char* url, bool reboot_on_success);

#ifdef __cplusplus
}
#endif
