/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ui_wallpaper.h"

#include "app/assets/assets.h"

ui_wallpaper_t* ui_wallpaper_attach(lv_obj_t* parent)
{
    if (parent == NULL)
    {
        return NULL;
    }

    ui_wallpaper_t* wallpaper = (ui_wallpaper_t*)lv_malloc(sizeof(ui_wallpaper_t));
    if (wallpaper == NULL)
    {
        return NULL;
    }
    lv_memset(wallpaper, 0, sizeof(ui_wallpaper_t));

    wallpaper->layer = lv_obj_create(parent);
    if (wallpaper->layer == NULL)
    {
        lv_free(wallpaper);
        return NULL;
    }

    lv_obj_remove_style_all(wallpaper->layer);
    lv_obj_set_size(wallpaper->layer, LV_PCT(100), LV_PCT(100));
    lv_obj_add_flag(wallpaper->layer, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_move_background(wallpaper->layer);
    lv_obj_set_style_bg_opa(wallpaper->layer, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(wallpaper->layer, 0, LV_PART_MAIN);

    wallpaper->image = lv_image_create(wallpaper->layer);
    if (wallpaper->image == NULL)
    {
        lv_obj_del(wallpaper->layer);
        lv_free(wallpaper);
        return NULL;
    }

    lv_obj_remove_style_all(wallpaper->image);
    lv_image_set_src(wallpaper->image, &launcher_bg);
    lv_obj_set_align(wallpaper->image, LV_ALIGN_CENTER);
    lv_obj_add_flag(wallpaper->image, LV_OBJ_FLAG_IGNORE_LAYOUT);
    lv_obj_move_background(wallpaper->image);

    return wallpaper;
}

void ui_wallpaper_detach(ui_wallpaper_t* wallpaper)
{
    if (wallpaper == NULL)
    {
        return;
    }

    if (wallpaper->image != NULL)
    {
        lv_obj_del(wallpaper->image);
        wallpaper->image = NULL;
    }

    if (wallpaper->layer != NULL)
    {
        lv_obj_del(wallpaper->layer);
        wallpaper->layer = NULL;
    }

    lv_free(wallpaper);
}
