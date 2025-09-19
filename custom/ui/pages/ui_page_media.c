/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ui_page_media.h"

#include "../ui_theme.h"
#include "../ui_wallpaper.h"
#include "../widgets/ui_room_card.h"

static void ui_page_media_delete_cb(lv_event_t* event)
{
    ui_wallpaper_t* wallpaper = (ui_wallpaper_t*)lv_event_get_user_data(event);
    ui_wallpaper_detach(wallpaper);
}

static lv_obj_t* ui_page_create_content(lv_obj_t* page, const char* title_text)
{
    LV_UNUSED(title_text);

    lv_obj_t* content = lv_obj_create(page);
    lv_obj_remove_style_all(content);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(content, 48, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(content, 32, LV_PART_MAIN);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_SCROLLABLE);

    return content;
}

lv_obj_t* ui_page_media_create(lv_obj_t* parent)
{
    if (parent == NULL)
    {
        return NULL;
    }

    lv_obj_t* page = lv_obj_create(parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_scroll_dir(page, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(page, LV_OBJ_FLAG_CLICKABLE);

    ui_wallpaper_t* wallpaper = ui_wallpaper_attach(page);
    if (wallpaper != NULL)
    {
        lv_obj_add_event_cb(page, ui_page_media_delete_cb, LV_EVENT_DELETE, wallpaper);
    }

    lv_obj_t* content = ui_page_create_content(page, "Media");

    ui_room_card_config_t now_playing_config = {
        .room_id   = "media.now_playing",
        .title     = "Now Playing",
        .icon_text = LV_SYMBOL_AUDIO,
    };

    ui_room_card_t* now_playing_card = ui_room_card_create(content, &now_playing_config);
    if (now_playing_card != NULL)
    {
        lv_obj_t* card_obj = ui_room_card_get_obj(now_playing_card);
        if (card_obj != NULL)
        {
            lv_obj_t* toggle = ui_room_card_get_toggle(now_playing_card);
            if (toggle != NULL)
            {
                lv_obj_add_flag(toggle, LV_OBJ_FLAG_HIDDEN);
            }

            lv_obj_t* specs = lv_obj_get_child(card_obj, 2);
            if (specs != NULL)
            {
                lv_obj_add_flag(specs, LV_OBJ_FLAG_HIDDEN);
            }

            lv_obj_t* info_row = lv_obj_create(card_obj);
            lv_obj_remove_style_all(info_row);
            lv_obj_set_width(info_row, LV_PCT(100));
            lv_obj_set_style_bg_opa(info_row, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_pad_gap(info_row, 24, LV_PART_MAIN);
            lv_obj_set_flex_flow(info_row, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(
                info_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
            lv_obj_clear_flag(info_row, LV_OBJ_FLAG_SCROLLABLE);

            lv_obj_t* album_art = lv_obj_create(info_row);
            lv_obj_remove_style_all(album_art);
            lv_obj_set_size(album_art, 240, 240);
            lv_obj_set_style_bg_color(album_art, ui_theme_color_surface(), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(album_art, LV_OPA_70, LV_PART_MAIN);
            lv_obj_set_style_radius(album_art, 16, LV_PART_MAIN);
            lv_obj_set_style_border_width(album_art, 0, LV_PART_MAIN);
            lv_obj_clear_flag(album_art, LV_OBJ_FLAG_SCROLLABLE);

            lv_obj_t* track_info = lv_obj_create(info_row);
            lv_obj_remove_style_all(track_info);
            lv_obj_set_width(track_info, LV_PCT(100));
            lv_obj_set_style_bg_opa(track_info, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_pad_gap(track_info, 12, LV_PART_MAIN);
            lv_obj_set_flex_flow(track_info, LV_FLEX_FLOW_COLUMN);
            lv_obj_set_flex_align(
                track_info, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
            lv_obj_clear_flag(track_info, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_flex_grow(track_info, 1);

            lv_obj_t* track_title = lv_label_create(track_info);
            lv_label_set_text(track_title, "Placeholder Track");
            lv_obj_set_style_text_font(track_title, &lv_font_montserrat_26, LV_PART_MAIN);
            lv_obj_set_style_text_color(track_title, ui_theme_color_on_surface(), LV_PART_MAIN);
            lv_label_set_long_mode(track_title, LV_LABEL_LONG_WRAP);
            lv_obj_set_width(track_title, LV_PCT(100));

            lv_obj_t* track_artist = lv_label_create(track_info);
            lv_label_set_text(track_artist, "Artist Name");
            lv_obj_set_style_text_font(track_artist, &lv_font_montserrat_20, LV_PART_MAIN);
            lv_obj_set_style_text_color(track_artist, ui_theme_color_muted(), LV_PART_MAIN);
            lv_label_set_long_mode(track_artist, LV_LABEL_LONG_WRAP);
            lv_obj_set_width(track_artist, LV_PCT(100));

            lv_obj_t* track_source = lv_label_create(track_info);
            lv_label_set_text(track_source, "Source · Placeholder");
            lv_obj_set_style_text_font(track_source, &lv_font_montserrat_18, LV_PART_MAIN);
            lv_obj_set_style_text_color(track_source, ui_theme_color_muted(), LV_PART_MAIN);
            lv_label_set_long_mode(track_source, LV_LABEL_LONG_WRAP);
            lv_obj_set_width(track_source, LV_PCT(100));

            lv_obj_t* transport_row = lv_obj_create(card_obj);
            lv_obj_remove_style_all(transport_row);
            lv_obj_set_width(transport_row, LV_PCT(100));
            lv_obj_set_style_bg_opa(transport_row, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_pad_gap(transport_row, 16, LV_PART_MAIN);
            lv_obj_set_flex_flow(transport_row, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(
                transport_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_clear_flag(transport_row, LV_OBJ_FLAG_SCROLLABLE);

            const char* button_labels[] = {"Prev", "Play/Pause", "Next"};
            for (size_t i = 0; i < (sizeof(button_labels) / sizeof(button_labels[0])); i++)
            {
                lv_obj_t* control_btn = lv_btn_create(transport_row);
                lv_obj_remove_style_all(control_btn);
                lv_obj_set_flex_grow(control_btn, 1);
                lv_obj_set_height(control_btn, 56);
                lv_obj_set_style_radius(control_btn, 18, LV_PART_MAIN);
                lv_obj_set_style_bg_color(control_btn, ui_theme_color_accent(), LV_PART_MAIN);
                lv_obj_set_style_bg_opa(control_btn, LV_OPA_80, LV_PART_MAIN);
                lv_obj_set_style_border_width(control_btn, 0, LV_PART_MAIN);
                lv_obj_set_style_shadow_width(control_btn, 0, LV_PART_MAIN);
                lv_obj_set_style_pad_hor(control_btn, 24, LV_PART_MAIN);
                lv_obj_set_style_pad_ver(control_btn, 8, LV_PART_MAIN);
                lv_obj_clear_flag(control_btn, LV_OBJ_FLAG_SCROLLABLE);

                lv_obj_t* btn_label = lv_label_create(control_btn);
                lv_label_set_text(btn_label, button_labels[i]);
                lv_obj_center(btn_label);
                lv_obj_set_style_text_font(btn_label, &lv_font_montserrat_18, LV_PART_MAIN);
                lv_obj_set_style_text_color(btn_label, lv_color_white(), LV_PART_MAIN);
            }

            lv_obj_t* volume_slider = lv_slider_create(transport_row);
            lv_obj_set_flex_grow(volume_slider, 2);
            lv_obj_set_height(volume_slider, 36);
            lv_slider_set_range(volume_slider, 0, 100);
            lv_slider_set_value(volume_slider, 40, LV_ANIM_OFF);
            lv_obj_clear_flag(volume_slider, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_color(volume_slider, ui_theme_color_muted(), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(volume_slider, LV_OPA_30, LV_PART_MAIN);
            lv_obj_set_style_radius(volume_slider, 18, LV_PART_MAIN);
            lv_obj_set_style_border_width(volume_slider, 0, LV_PART_MAIN);
            lv_obj_set_style_bg_color(volume_slider, ui_theme_color_accent(), LV_PART_INDICATOR);
            lv_obj_set_style_bg_opa(volume_slider, LV_OPA_COVER, LV_PART_INDICATOR);
            lv_obj_set_style_radius(volume_slider, 18, LV_PART_INDICATOR);
        }
    }

    ui_room_card_config_t scenes_config = {
        .room_id   = "media.quick_scenes",
        .title     = "Quick Scenes",
        .icon_text = LV_SYMBOL_LIST,
    };

    ui_room_card_t* scenes_card = ui_room_card_create(content, &scenes_config);
    if (scenes_card != NULL)
    {
        lv_obj_t* card_obj = ui_room_card_get_obj(scenes_card);
        if (card_obj != NULL)
        {
            lv_obj_t* toggle = ui_room_card_get_toggle(scenes_card);
            if (toggle != NULL)
            {
                lv_obj_add_flag(toggle, LV_OBJ_FLAG_HIDDEN);
            }

            lv_obj_t* specs = lv_obj_get_child(card_obj, 2);
            if (specs != NULL)
            {
                lv_obj_add_flag(specs, LV_OBJ_FLAG_HIDDEN);
            }

            lv_obj_t* scene_grid = lv_obj_create(card_obj);
            lv_obj_remove_style_all(scene_grid);
            lv_obj_set_width(scene_grid, LV_PCT(100));
            lv_obj_set_style_bg_opa(scene_grid, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_pad_gap(scene_grid, 16, LV_PART_MAIN);
            lv_obj_set_flex_flow(scene_grid, LV_FLEX_FLOW_ROW_WRAP);
            lv_obj_set_flex_align(
                scene_grid, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER);
            lv_obj_clear_flag(scene_grid, LV_OBJ_FLAG_SCROLLABLE);

            const char* scene_labels[] = {"Morning", "Movie", "Night", "Party"};
            for (size_t i = 0; i < (sizeof(scene_labels) / sizeof(scene_labels[0])); i++)
            {
                lv_obj_t* scene_btn = lv_btn_create(scene_grid);
                lv_obj_remove_style_all(scene_btn);
                lv_obj_set_style_radius(scene_btn, 14, LV_PART_MAIN);
                lv_obj_set_style_bg_color(scene_btn, ui_theme_color_surface(), LV_PART_MAIN);
                lv_obj_set_style_bg_opa(scene_btn, LV_OPA_60, LV_PART_MAIN);
                lv_obj_set_style_border_width(scene_btn, 0, LV_PART_MAIN);
                lv_obj_set_style_shadow_width(scene_btn, 0, LV_PART_MAIN);
                lv_obj_set_style_pad_hor(scene_btn, 24, LV_PART_MAIN);
                lv_obj_set_style_pad_ver(scene_btn, 16, LV_PART_MAIN);
                lv_obj_set_flex_grow(scene_btn, 1);
                lv_obj_clear_flag(scene_btn, LV_OBJ_FLAG_SCROLLABLE);

                lv_obj_t* label = lv_label_create(scene_btn);
                lv_label_set_text(label, scene_labels[i]);
                lv_obj_center(label);
                lv_obj_set_style_text_font(label, &lv_font_montserrat_18, LV_PART_MAIN);
                lv_obj_set_style_text_color(label, ui_theme_color_on_surface(), LV_PART_MAIN);
            }
        }
    }

    return page;
}
