/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "esp_err.h"
#include "settings_core/app_cfg.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct settings_ui_runtime
    {
        app_cfg_ui_t last_applied;
        bool         dimming_active;
    } settings_ui_runtime_t;

    esp_err_t settings_ui_apply(const app_cfg_t* cfg, settings_ui_runtime_t* state);
    esp_err_t settings_ui_schedule_dim(settings_ui_runtime_t* state, uint32_t timeout_ms);
    void      settings_ui_cancel_dim(settings_ui_runtime_t* state);

#ifdef __cplusplus
}
#endif
