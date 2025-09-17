/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ui_wallpaper.h"

#include "assets/asset_mount.h"

ui_wallpaper_t *ui_wallpaper_attach(lv_obj_t *parent)
{
    if (parent == NULL) {
        return NULL;
    }

    ui_wallpaper_t *wallpaper = (ui_wallpaper_t *)lv_malloc(sizeof(ui_wallpaper_t));
    if (wallpaper == NULL) {
        return NULL;
    }
    lv_memset(wallpaper, 0, sizeof(ui_wallpaper_t));

    wallpaper->layer = lv_obj_create(parent);
    if (wallpaper->layer == NULL) {
        lv_free(wallpaper);
        return NULL;
    }

    lv_obj_remove_style_all(wallpaper->layer);
    lv_obj_set_size(wallpaper->layer, LV_PCT(100), LV_PCT(100));
    lv_obj_add_flag(wallpaper->layer, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_move_background(wallpaper->layer);
    lv_obj_set_style_bg_opa(wallpaper->layer, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_bg_color(wallpaper->layer, lv_color_hex(0x101827), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(wallpaper->layer, lv_color_hex(0x1f2a3b), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(wallpaper->layer, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_border_width(wallpaper->layer, 0, LV_PART_MAIN);

    assets_fs_init();

    const char *image_path = assets_path_1();
    if (image_path != NULL) {
        wallpaper->image = lv_img_create(wallpaper->layer);
        if (wallpaper->image != NULL) {
            lv_obj_add_flag(wallpaper->image, LV_OBJ_FLAG_IGNORE_LAYOUT);
            lv_obj_set_size(wallpaper->image, LV_PCT(100), LV_PCT(100));
            lv_obj_center(wallpaper->image);
#if LVGL_VERSION_MAJOR >= 9
            lv_img_set_src(wallpaper->image, image_path);
#else
            if (lv_img_set_src(wallpaper->image, image_path) != LV_RES_OK) {
                lv_obj_del(wallpaper->image);
                wallpaper->image = NULL;
            }
#endif
        }
    }

    return wallpaper;
}

void ui_wallpaper_detach(ui_wallpaper_t *wallpaper)
{
    if (wallpaper == NULL) {
        return;
    }

    if (wallpaper->image != NULL) {
        lv_obj_del(wallpaper->image);
        wallpaper->image = NULL;
    }

    if (wallpaper->layer != NULL) {
        lv_obj_del(wallpaper->layer);
        wallpaper->layer = NULL;
    }

    lv_free(wallpaper);
}
