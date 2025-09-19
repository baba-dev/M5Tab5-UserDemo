/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdbool.h>
#include <stdint.h>

#ifdef __has_include
#    if __has_include("lvgl.h")
#        ifndef LV_LVGL_H_INCLUDE_SIMPLE
#            define LV_LVGL_H_INCLUDE_SIMPLE
#        endif
#    endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#    include "lvgl.h"
#else
#    include "lvgl/lvgl.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        UI_PAGE_SETTINGS_STATUS_UNKNOWN = 0,
        UI_PAGE_SETTINGS_STATUS_OK,
        UI_PAGE_SETTINGS_STATUS_WARNING,
        UI_PAGE_SETTINGS_STATUS_ERROR,
    } ui_page_settings_status_t;

    typedef struct ui_page_settings_actions_t
    {
        void (*run_connection_test)(const char* tester_id, void* user_data);
        void (*set_dark_mode)(bool enabled, void* user_data);
        void (*set_theme_variant)(const char* variant_id, void* user_data);
        void (*set_brightness)(uint8_t percent, void* user_data);
        void (*open_display_settings)(void* user_data);
        void (*open_network_settings)(void* user_data);
        void (*sync_time)(void* user_data);
        void (*check_for_updates)(void* user_data);
        void (*start_ota_update)(void* user_data);
        void (*open_diagnostics)(void* user_data);
        void (*export_logs)(void* user_data);
        void (*backup_now)(void* user_data);
        void (*restore_backup)(void* user_data);
    } ui_page_settings_actions_t;

    lv_obj_t* ui_page_settings_create(lv_obj_t* parent);
    void ui_page_settings_set_actions(const ui_page_settings_actions_t* actions, void* user_data);
    void ui_page_settings_set_connection_status(const char*               tester_id,
                                                ui_page_settings_status_t status,
                                                const char*               message);
    void ui_page_settings_set_update_status(const char* status_text);
    void ui_page_settings_set_diagnostics_status(const char* status_text);
    void ui_page_settings_set_backup_status(const char* status_text);
    void ui_page_settings_apply_theme_state(bool dark_mode_enabled, const char* variant_id);
    void ui_page_settings_set_brightness(uint8_t percent);
    const char* ui_page_settings_get_update_status(void);
    const char* ui_page_settings_get_diagnostics_status(void);
    const char* ui_page_settings_get_backup_status(void);

#ifdef __cplusplus
}
#endif
