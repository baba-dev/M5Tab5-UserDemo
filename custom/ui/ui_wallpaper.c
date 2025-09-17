/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ui_wallpaper.h"

#include "custom/assets/bg/bg_images.h"

#define WALLPAPER_SWITCH_PERIOD_MS 30000

static const lv_image_dsc_t *k_wallpapers[] = {
    &bg_wallpaper_1,
    &bg_wallpaper_2,
};

static void ui_wallpaper_timer_cb(lv_timer_t *timer)
{
    ui_wallpaper_t *wallpaper = (ui_wallpaper_t *)timer->user_data;
    if (wallpaper == NULL || wallpaper->img == NULL) {
        return;
    }

    wallpaper->idx ^= 1U;
    lv_image_set_src(wallpaper->img, k_wallpapers[wallpaper->idx]);
}

ui_wallpaper_t *ui_wallpaper_attach(lv_obj_t *parent)
{
    if (parent == NULL) {
        return NULL;
    }

    ui_wallpaper_t *wallpaper = (ui_wallpaper_t *)lv_malloc(sizeof(ui_wallpaper_t));
    if (wallpaper == NULL) {
        return NULL;
    }
    lv_memset_00(wallpaper, sizeof(ui_wallpaper_t));

    wallpaper->img = lv_image_create(parent);
    lv_obj_add_flag(wallpaper->img, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_set_size(wallpaper->img, LV_PCT(100), LV_PCT(100));
    lv_obj_center(wallpaper->img);
    lv_image_set_src(wallpaper->img, k_wallpapers[0]);
    lv_obj_set_style_bg_opa(wallpaper->img, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(wallpaper->img, 0, LV_PART_MAIN);
    lv_obj_move_background(wallpaper->img);

    wallpaper->timer = lv_timer_create(ui_wallpaper_timer_cb, WALLPAPER_SWITCH_PERIOD_MS, wallpaper);
    if (wallpaper->timer == NULL) {
        ui_wallpaper_detach(wallpaper);
        return NULL;
    }

    return wallpaper;
}

void ui_wallpaper_detach(ui_wallpaper_t *wallpaper)
{
    if (wallpaper == NULL) {
        return;
    }

    if (wallpaper->timer != NULL) {
        lv_timer_del(wallpaper->timer);
        wallpaper->timer = NULL;
    }

    if (wallpaper->img != NULL) {
        lv_obj_del(wallpaper->img);
        wallpaper->img = NULL;
    }

    lv_free(wallpaper);
}
