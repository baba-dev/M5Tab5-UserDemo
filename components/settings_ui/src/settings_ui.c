/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "settings_ui/settings_ui.h"

#include "esp_log.h"
#include "esp_timer.h"

static const char* TAG = "settings_ui";

static esp_timer_handle_t s_dim_timer = NULL;

static void dim_timer_callback(void* arg)
{
    settings_ui_runtime_t* state = (settings_ui_runtime_t*)arg;
    if (!state)
    {
        return;
    }
    state->dimming_active = true;
    ESP_LOGI(
        TAG, "Screen dim timer fired (%u seconds)", state->last_applied.screen_timeout_seconds);
}

esp_err_t settings_ui_apply(const app_cfg_t* cfg, settings_ui_runtime_t* state)
{
    if (!cfg || !state)
    {
        return ESP_ERR_INVALID_ARG;
    }

    state->last_applied   = cfg->ui;
    state->dimming_active = false;

    switch (cfg->ui.theme)
    {
        case APP_CFG_UI_THEME_LIGHT:
            ESP_LOGI(
                TAG, "Applying light theme with brightness %u", (unsigned int)cfg->ui.brightness);
            break;
        case APP_CFG_UI_THEME_DARK:
            ESP_LOGI(
                TAG, "Applying dark theme with brightness %u", (unsigned int)cfg->ui.brightness);
            break;
        case APP_CFG_UI_THEME_AUTO:
        default:
            ESP_LOGI(
                TAG, "Applying auto theme with brightness %u", (unsigned int)cfg->ui.brightness);
            break;
    }
    return ESP_OK;
}

esp_err_t settings_ui_schedule_dim(settings_ui_runtime_t* state, uint32_t timeout_ms)
{
    if (!state)
    {
        return ESP_ERR_INVALID_ARG;
    }
    if (!s_dim_timer)
    {
        const esp_timer_create_args_t args = {
            .callback = &dim_timer_callback,
            .arg      = state,
            .name     = "ui_dim",
        };
        esp_err_t err = esp_timer_create(&args, &s_dim_timer);
        if (err != ESP_OK)
        {
            return err;
        }
    }

    esp_err_t err = esp_timer_set_arg(s_dim_timer, state);
    if (err != ESP_OK)
    {
        return err;
    }

    if (esp_timer_is_active(s_dim_timer))
    {
        esp_timer_stop(s_dim_timer);
    }
    state->dimming_active = false;
    return esp_timer_start_once(s_dim_timer, timeout_ms * 1000ULL);
}

void settings_ui_cancel_dim(settings_ui_runtime_t* state)
{
    (void)state;
    if (s_dim_timer && esp_timer_is_active(s_dim_timer))
    {
        esp_timer_stop(s_dim_timer);
    }
}
