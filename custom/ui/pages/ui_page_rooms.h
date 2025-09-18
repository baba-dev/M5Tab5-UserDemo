/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

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

#include "ui_rooms_model.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        UI_PAGE_ROOMS_SIGNAL_TOGGLE = 0,
        UI_PAGE_ROOMS_SIGNAL_OPEN_SHEET,
    } ui_page_rooms_signal_t;

    typedef struct
    {
        ui_page_rooms_signal_t signal;
        const char*            room_id;
        const char*            entity_id;
    } ui_page_rooms_event_data_t;

#define UI_PAGE_ROOMS_EVENT_TOGGLE     ((lv_event_code_t)(LV_EVENT_LAST + 1))
#define UI_PAGE_ROOMS_EVENT_OPEN_SHEET ((lv_event_code_t)(LV_EVENT_LAST + 2))

    lv_obj_t* ui_page_rooms_create(lv_obj_t* parent);
    void      ui_page_rooms_set_state(const rooms_state_t* state);
    lv_obj_t* ui_page_rooms_get_card(const char* room_id);
    lv_obj_t* ui_page_rooms_get_toggle(const char* room_id);

#ifdef __cplusplus
}
#endif
