/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
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

#define UI_PAGE_MEDIA_MAX_SCENES 4U

    typedef enum
    {
        UI_PAGE_MEDIA_SIGNAL_PREVIOUS = 0,
        UI_PAGE_MEDIA_SIGNAL_PLAY_PAUSE,
        UI_PAGE_MEDIA_SIGNAL_NEXT,
        UI_PAGE_MEDIA_SIGNAL_VOLUME,
        UI_PAGE_MEDIA_SIGNAL_TRIGGER_SCENE,
    } ui_page_media_signal_t;

    typedef struct
    {
        const char* scene_id;
        const char* title;
    } ui_page_media_scene_t;

    typedef struct
    {
        const char* media_id;
        const char* title;
        const char* artist;
        const char* source;
        bool        playing;
        uint8_t     volume;
    } ui_page_media_now_playing_t;

    typedef struct
    {
        ui_page_media_signal_t signal;
        const char*            scene_id;
        uint8_t                volume;
    } ui_page_media_event_t;

#define UI_PAGE_MEDIA_EVENT_COMMAND ((lv_event_code_t)(LV_EVENT_LAST + 30))

    lv_obj_t* ui_page_media_create(lv_obj_t* parent);
    lv_obj_t* ui_page_media_get_obj(void);
    lv_obj_t* ui_page_media_get_previous_button(void);
    lv_obj_t* ui_page_media_get_play_pause_button(void);
    lv_obj_t* ui_page_media_get_next_button(void);
    lv_obj_t* ui_page_media_get_volume_slider(void);
    lv_obj_t* ui_page_media_get_scene_button(size_t index);
    size_t    ui_page_media_get_scene_count(void);

    void ui_page_media_set_now_playing(const ui_page_media_now_playing_t* now_playing);
    void ui_page_media_set_quick_scenes(const ui_page_media_scene_t* scenes, size_t scene_count);

#ifdef __cplusplus
}
#endif
