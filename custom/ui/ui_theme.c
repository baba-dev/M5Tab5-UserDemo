/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ui_theme.h"

static lv_color_t make_color(uint8_t r, uint8_t g, uint8_t b)
{
    return lv_color_make(r, g, b);
}

lv_color_t ui_theme_color_accent(void)
{
    return make_color(124, 131, 255);
}

lv_color_t ui_theme_color_surface(void)
{
    return make_color(24, 31, 45);
}

lv_color_t ui_theme_color_on_surface(void)
{
    return make_color(248, 250, 252);
}

lv_color_t ui_theme_color_muted(void)
{
    return make_color(162, 173, 195);
}

lv_color_t ui_theme_color_outline(void)
{
    return make_color(88, 103, 136);
}
