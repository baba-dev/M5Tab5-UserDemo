/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once
#include "lvgl.h"
#ifdef __cplusplus
extern "C"
{
#endif

    /** Create the Home screen under parent (or lv_scr_act() if NULL). */
    lv_obj_t* ui_home_create(lv_obj_t* parent);

#ifdef __cplusplus
}
#endif
