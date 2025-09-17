/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include <stdio.h>

#include "ui_wallpaper.h"

#include "assets/asset_mount.h"

#define WALLPAPER_SWITCH_PERIOD_MS 30000

static void ui_wallpaper_set_src(const ui_wallpaper_t *wallpaper, bool second)
{
    if (wallpaper == NULL || wallpaper->img == NULL) {
        return;
    }

    const char *path = second ? assets_path_2() : assets_path_1();
    static char lv_path[256];
    int written = snprintf(lv_path, sizeof(lv_path), "A:%s", path);
    if (written < 0 || written >= (int)sizeof(lv_path)) {
        LV_LOG_WARN("Wallpaper path truncated: %s", path);
        return;
    }

    lv_image_set_src(wallpaper->img, lv_path);
}

static void ui_wallpaper_timer_cb(lv_timer_t *timer)
{
    ui_wallpaper_t *wallpaper = (ui_wallpaper_t *)timer->user_data;
    if (wallpaper == NULL || wallpaper->img == NULL) {
        return;
    }

    wallpaper->idx ^= 1U;
    ui_wallpaper_set_src(wallpaper, wallpaper->idx != 0U);
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

    assets_fs_init();

    wallpaper->img = lv_image_create(parent);
    lv_obj_add_flag(wallpaper->img, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_set_size(wallpaper->img, LV_PCT(100), LV_PCT(100));
    lv_obj_center(wallpaper->img);
    ui_wallpaper_set_src(wallpaper, false);
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
