/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

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

typedef struct ui_wallpaper_t {
    lv_obj_t *layer;
} ui_wallpaper_t;

ui_wallpaper_t *ui_wallpaper_attach(lv_obj_t *parent);
void ui_wallpaper_detach(ui_wallpaper_t *wallpaper);

#ifdef __cplusplus
}
#endif
