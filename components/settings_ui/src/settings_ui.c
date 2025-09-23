/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "settings_ui/settings_ui.h"

#include <string.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "lvgl.h"

static const char* TAG = "settings_ui";

static esp_timer_handle_t s_dim_timer = NULL;

static settings_ui_runtime_t* s_dim_state = NULL;

typedef struct settings_ui_async_payload
{
    settings_ui_runtime_t* state;
    app_cfg_ui_t           ui_cfg;
} settings_ui_async_payload_t;

static void settings_ui_apply_cb(void* param)
{
    settings_ui_async_payload_t* payload = (settings_ui_async_payload_t*)param;
    if (payload == NULL)
    {
        return;
    }

    settings_ui_runtime_t* state = payload->state;
    app_cfg_ui_t           cfg   = payload->ui_cfg;

    lv_free(payload);

    if (state == NULL)
    {
        return;
    }

    const char* theme_label = "auto";
    switch (cfg.theme)
    {
        case APP_CFG_UI_THEME_LIGHT:
            theme_label = "light";
            break;
        case APP_CFG_UI_THEME_DARK:
            theme_label = "dark";
            break;
        case APP_CFG_UI_THEME_AUTO:
        default:
            theme_label = "auto";
            break;
    }

    ESP_LOGI(
        TAG, "Applying %s theme with brightness %u", theme_label, (unsigned int)cfg.brightness);

    state->theme_initialized = true;
    state->theme_pending     = false;
    state->last_applied      = cfg;
    state->dimming_active    = false;
}

static void dim_timer_callback(void* arg)
{
    (void)arg;
    settings_ui_runtime_t* state = s_dim_state;
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

    state->dimming_active = false;

    if (state->theme_pending && memcmp(&state->pending_config, &cfg->ui, sizeof(app_cfg_ui_t)) == 0)
    {
        return ESP_OK;
    }

    if (!state->theme_pending && state->theme_initialized
        && memcmp(&state->last_applied, &cfg->ui, sizeof(app_cfg_ui_t)) == 0)
    {
        return ESP_OK;
    }

    settings_ui_async_payload_t* payload =
        (settings_ui_async_payload_t*)lv_malloc(sizeof(settings_ui_async_payload_t));
    if (payload == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate theme payload");
        return ESP_ERR_NO_MEM;
    }

    payload->state  = state;
    payload->ui_cfg = cfg->ui;

    state->pending_config = cfg->ui;
    state->theme_pending  = true;
    state->last_applied   = cfg->ui;

    if (lv_async_call(settings_ui_apply_cb, payload) != LV_RES_OK)
    {
        state->theme_pending = false;
        ESP_LOGE(TAG, "Failed to schedule theme update");
        lv_free(payload);
        return ESP_FAIL;
    }

    return ESP_OK;
}

esp_err_t settings_ui_schedule_dim(settings_ui_runtime_t* state, uint32_t timeout_ms)
{
    if (!state)
    {
        return ESP_ERR_INVALID_ARG;
    }
    s_dim_state = state;

    if (!s_dim_timer)
    {
        const esp_timer_create_args_t args = {
            .callback = &dim_timer_callback,
            .arg      = NULL,
            .name     = "ui_dim",
        };
        esp_err_t err = esp_timer_create(&args, &s_dim_timer);
        if (err != ESP_OK)
        {
            return err;
        }
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
    if (s_dim_timer && esp_timer_is_active(s_dim_timer))
    {
        esp_timer_stop(s_dim_timer);
    }
    if (state == s_dim_state)
    {
        s_dim_state = NULL;
    }
}
