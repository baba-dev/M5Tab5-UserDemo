/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>

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

    typedef struct
    {
        const char* camera_id;
        const char* name;
        const char* status;
        const char* stream_url;
        const char* snapshot_url;
        bool        audio_supported;
    } ui_page_cctv_camera_t;

    typedef struct
    {
        const char* event_id;
        const char* camera_id;
        const char* title;
        const char* description;
        const char* timestamp;
        const char* clip_url;
        const char* snapshot_url;
    } ui_page_cctv_event_t;

    typedef struct
    {
        const ui_page_cctv_camera_t* cameras;
        size_t                       camera_count;
        size_t                       active_index;
        const ui_page_cctv_event_t*  events;
        size_t                       event_count;
        const char*                  quality_label;
        bool                         muted;
    } ui_page_cctv_state_t;

    typedef enum
    {
        UI_PAGE_CCTV_ACTION_PREVIOUS = 0,
        UI_PAGE_CCTV_ACTION_NEXT,
        UI_PAGE_CCTV_ACTION_QUALITY,
        UI_PAGE_CCTV_ACTION_TOGGLE_MUTE,
        UI_PAGE_CCTV_ACTION_OPEN_GATE,
        UI_PAGE_CCTV_ACTION_TALK,
        UI_PAGE_CCTV_ACTION_SNAPSHOT,
        UI_PAGE_CCTV_ACTION_TIMELINE,
    } ui_page_cctv_action_t;

    typedef struct
    {
        ui_page_cctv_action_t action;
        size_t                camera_index;
        size_t                camera_count;
        const char*           camera_id;
        const char*           stream_url;
        const char*           quality_label;
        bool                  muted;
    } ui_page_cctv_action_event_t;

    typedef struct
    {
        const ui_page_cctv_event_t* event;
        size_t                      index;
    } ui_page_cctv_clip_event_t;

#define UI_PAGE_CCTV_EVENT_ACTION    ((lv_event_code_t)(LV_EVENT_LAST + 10))
#define UI_PAGE_CCTV_EVENT_OPEN_CLIP ((lv_event_code_t)(LV_EVENT_LAST + 11))

    lv_obj_t* ui_page_cctv_create(lv_obj_t* parent);
    lv_obj_t* ui_page_cctv_get_obj(void);
    void      ui_page_cctv_set_state(const ui_page_cctv_state_t* state);
    void      ui_page_cctv_set_events(const ui_page_cctv_event_t* events, size_t event_count);

#ifdef __cplusplus
}
#endif
