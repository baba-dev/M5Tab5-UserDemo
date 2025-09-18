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

    typedef struct ui_room_card_t ui_room_card_t;

    typedef struct
    {
        const char* room_id;
        const char* title;
        const char* icon_text;
    } ui_room_card_config_t;

    typedef struct
    {
        bool        on;
        bool        available;
        int8_t      temp_c;
        uint8_t     humidity;
        const char* primary_entity_id;
    } ui_room_card_state_t;

    ui_room_card_t* ui_room_card_create(lv_obj_t* parent, const ui_room_card_config_t* config);
    void            ui_room_card_destroy(ui_room_card_t* card);
    lv_obj_t*       ui_room_card_get_obj(ui_room_card_t* card);
    lv_obj_t*       ui_room_card_get_toggle(ui_room_card_t* card);
    void            ui_room_card_set_state(ui_room_card_t* card, const ui_room_card_state_t* state);
    const char*     ui_room_card_get_room_id(const ui_room_card_t* card);
    const char*     ui_room_card_get_entity_id(const ui_room_card_t* card);
    void            ui_room_card_play_toggle_feedback(ui_room_card_t* card);
    void            ui_room_card_play_enter_anim(ui_room_card_t* card, uint32_t delay_ms);

#ifdef __cplusplus
}
#endif
