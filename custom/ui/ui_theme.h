/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

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

    /**
     * @brief Return the primary accent color used for active controls.
     */
    lv_color_t ui_theme_color_accent(void);

    /**
     * @brief Return the translucent surface color used for glassmorphic cards.
     */
    lv_color_t ui_theme_color_surface(void);

    /**
     * @brief Return the text color for content resting on the surface color.
     */
    lv_color_t ui_theme_color_on_surface(void);

    /**
     * @brief Return a muted text color for secondary metadata labels.
     */
    lv_color_t ui_theme_color_muted(void);

    /**
     * @brief Return the outline tint used for elevated surfaces.
     */
    lv_color_t ui_theme_color_outline(void);

#ifdef __cplusplus
}
#endif
