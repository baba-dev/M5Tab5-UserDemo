/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdbool.h>

#ifdef __has_include
#if __has_include("lvgl.h")
#ifndef LV_LVGL_H_INCLUDE_SIMPLE
#define LV_LVGL_H_INCLUDE_SIMPLE
#endif
#endif
#endif

#if defined(LV_LVGL_H_INCLUDE_SIMPLE)
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    UI_NAV_PAGE_DEFAULT = 0,
    UI_NAV_PAGE_ROOMS,
    UI_NAV_PAGE_CCTV,
    UI_NAV_PAGE_WEATHER,
    UI_NAV_PAGE_MEDIA,
    UI_NAV_PAGE_COUNT
} ui_nav_page_t;

typedef struct ui_nav_rail_t ui_nav_rail_t;

typedef void (*ui_nav_rail_callback_t)(ui_nav_rail_t *rail, ui_nav_page_t page, void *user_data);

ui_nav_rail_t *ui_nav_rail_create(lv_obj_t *parent, ui_nav_rail_callback_t callback, void *user_data);
void ui_nav_rail_destroy(ui_nav_rail_t *rail);
void ui_nav_rail_set_active(ui_nav_rail_t *rail, ui_nav_page_t page);
lv_obj_t *ui_nav_rail_get_container(ui_nav_rail_t *rail);
void ui_nav_rail_show(ui_nav_rail_t *rail, bool animate);
void ui_nav_rail_hide(ui_nav_rail_t *rail, bool animate);
bool ui_nav_rail_is_visible(const ui_nav_rail_t *rail);

#ifdef __cplusplus
}
#endif
