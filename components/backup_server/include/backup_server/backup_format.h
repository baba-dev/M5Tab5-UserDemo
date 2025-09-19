/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stddef.h>

#include "esp_err.h"
#include "settings_core/app_cfg.h"

#ifdef __cplusplus
extern "C"
{
#endif

    size_t    backup_server_calculate_json_size(const app_cfg_t* cfg);
    esp_err_t backup_server_write_json(const app_cfg_t* cfg, char* buffer, size_t length);

#ifdef __cplusplus
}
#endif
