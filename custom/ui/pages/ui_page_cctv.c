/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ui_page_cctv.h"

#include "../ui_wallpaper.h"

static void ui_page_cctv_delete_cb(lv_event_t *event)
{
    ui_wallpaper_t *wallpaper = (ui_wallpaper_t *)lv_event_get_user_data(event);
    ui_wallpaper_detach(wallpaper);
}

static lv_obj_t *ui_page_create_content(lv_obj_t *page, const char *title_text)
{
    lv_obj_t *content = lv_obj_create(page);
    lv_obj_remove_style_all(content);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_left(content, 192, LV_PART_MAIN);
    lv_obj_set_style_pad_right(content, 48, LV_PART_MAIN);
    lv_obj_set_style_pad_top(content, 40, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(content, 40, LV_PART_MAIN);
    lv_obj_set_style_pad_row(content, 32, LV_PART_MAIN);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t *title = lv_obj_create(content);
    lv_obj_remove_style_all(title);
    lv_obj_set_width(title, LV_PCT(100));
    lv_obj_set_style_bg_color(title, lv_color_hex(0x141e28), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(title, LV_OPA_78, LV_PART_MAIN);
    lv_obj_set_style_radius(title, 16, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(title, 24, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(title, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_shadow_ofs_y(title, 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(title, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(title, 28, LV_PART_MAIN);

    lv_obj_t *label = lv_label_create(title);
    lv_label_set_text(label, title_text);
    lv_obj_set_width(label, LV_PCT(100));
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xf1f5f9), LV_PART_MAIN);

    return content;
}

lv_obj_t *ui_page_cctv_create(lv_obj_t *parent)
{
    if (parent == NULL) {
        return NULL;
    }

    lv_obj_t *page = lv_obj_create(parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_scroll_dir(page, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(page, LV_OBJ_FLAG_CLICKABLE);

    ui_wallpaper_t *wallpaper = ui_wallpaper_attach(page);
    if (wallpaper != NULL) {
        lv_obj_add_event_cb(page, ui_page_cctv_delete_cb, LV_EVENT_DELETE, wallpaper);
    }

    ui_page_create_content(page, "Frigate Security");

    return page;
}
