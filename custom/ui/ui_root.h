/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "ui_nav_rail.h"

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

typedef struct ui_root_t ui_root_t;

ui_root_t *ui_root_create(void);
void ui_root_destroy(ui_root_t *root);
void ui_root_show_page(ui_root_t *root, ui_nav_page_t page);
ui_nav_page_t ui_root_get_active(const ui_root_t *root);

#ifdef __cplusplus
}
#endif
