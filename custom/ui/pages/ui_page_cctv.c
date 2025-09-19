/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ui_page_cctv.h"

#include "../ui_theme.h"
#include "../ui_wallpaper.h"
#include "../widgets/ui_room_card.h"

static void ui_page_cctv_delete_cb(lv_event_t* event)
{
    ui_wallpaper_t* wallpaper = (ui_wallpaper_t*)lv_event_get_user_data(event);
    ui_wallpaper_detach(wallpaper);
}

static lv_obj_t* ui_page_create_content(lv_obj_t* page, const char* title_text)
{
    lv_obj_t* content = lv_obj_create(page);
    lv_obj_remove_style_all(content);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(content, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_left(content, 192, LV_PART_MAIN);
    lv_obj_set_style_pad_right(content, 48, LV_PART_MAIN);
    lv_obj_set_style_pad_top(content, 48, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(content, 48, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(content, 32, LV_PART_MAIN);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t* title = lv_label_create(content);
    lv_label_set_text(title, title_text);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, ui_theme_color_on_surface(), LV_PART_MAIN);

    lv_obj_t* toolbar = lv_obj_create(content);
    lv_obj_remove_style_all(toolbar);
    lv_obj_set_width(toolbar, LV_PCT(100));
    lv_obj_set_style_bg_color(toolbar, ui_theme_color_surface(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(toolbar, LV_OPA_70, LV_PART_MAIN);
    lv_obj_set_style_border_width(toolbar, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(toolbar, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_all(toolbar, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(toolbar, 20, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(toolbar, 12, LV_PART_MAIN);
    lv_obj_set_style_shadow_ofs_y(toolbar, 10, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(toolbar, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(toolbar, ui_theme_color_outline(), LV_PART_MAIN);
    lv_obj_set_flex_flow(toolbar, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(toolbar, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t* prev_btn = lv_btn_create(toolbar);
    lv_obj_remove_style_all(prev_btn);
    lv_obj_set_style_bg_color(prev_btn, ui_theme_color_outline(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(prev_btn, LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_radius(prev_btn, 20, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(prev_btn, 20, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(prev_btn, 12, LV_PART_MAIN);
    lv_obj_set_style_text_color(prev_btn, ui_theme_color_on_surface(), LV_PART_MAIN);

    lv_obj_t* prev_label = lv_label_create(prev_btn);
    lv_label_set_text(prev_label, LV_SYMBOL_LEFT " Prev");
    lv_obj_set_style_text_font(prev_label, &lv_font_montserrat_18, LV_PART_MAIN);

    lv_obj_t* camera_label = lv_label_create(toolbar);
    lv_label_set_text(camera_label, "Camera 2 of 6");
    lv_obj_set_style_text_font(camera_label, &lv_font_montserrat_20, LV_PART_MAIN);
    lv_obj_set_style_text_color(camera_label, ui_theme_color_on_surface(), LV_PART_MAIN);

    lv_obj_t* next_btn = lv_btn_create(toolbar);
    lv_obj_remove_style_all(next_btn);
    lv_obj_set_style_bg_color(next_btn, ui_theme_color_outline(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(next_btn, LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_radius(next_btn, 20, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(next_btn, 20, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(next_btn, 12, LV_PART_MAIN);
    lv_obj_set_style_text_color(next_btn, ui_theme_color_on_surface(), LV_PART_MAIN);

    lv_obj_t* next_label = lv_label_create(next_btn);
    lv_label_set_text(next_label, "Next " LV_SYMBOL_RIGHT);
    lv_obj_set_style_text_font(next_label, &lv_font_montserrat_18, LV_PART_MAIN);

    lv_obj_t* quality_btn = lv_btn_create(toolbar);
    lv_obj_remove_style_all(quality_btn);
    lv_obj_set_style_bg_color(quality_btn, ui_theme_color_surface(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(quality_btn, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_radius(quality_btn, 20, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(quality_btn, 22, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(quality_btn, 12, LV_PART_MAIN);
    lv_obj_set_style_border_width(quality_btn, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(quality_btn, ui_theme_color_outline(), LV_PART_MAIN);
    lv_obj_set_style_text_color(quality_btn, ui_theme_color_on_surface(), LV_PART_MAIN);

    lv_obj_t* quality_label = lv_label_create(quality_btn);
    lv_label_set_text(quality_label, "Quality ▾");
    lv_obj_set_style_text_font(quality_label, &lv_font_montserrat_18, LV_PART_MAIN);

    lv_obj_t* mute_btn = lv_btn_create(toolbar);
    lv_obj_remove_style_all(mute_btn);
    lv_obj_set_style_bg_color(mute_btn, ui_theme_color_surface(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(mute_btn, LV_OPA_60, LV_PART_MAIN);
    lv_obj_set_style_radius(mute_btn, 20, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(mute_btn, 22, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(mute_btn, 12, LV_PART_MAIN);
    lv_obj_set_style_border_width(mute_btn, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(mute_btn, ui_theme_color_outline(), LV_PART_MAIN);
    lv_obj_set_style_text_color(mute_btn, ui_theme_color_on_surface(), LV_PART_MAIN);

    lv_obj_t* mute_label = lv_label_create(mute_btn);
    lv_label_set_text(mute_label, LV_SYMBOL_AUDIO " Mute");
    lv_obj_set_style_text_font(mute_label, &lv_font_montserrat_18, LV_PART_MAIN);

    ui_room_card_config_t camera_config = {
        .room_id   = "cctv_main",
        .title     = "Backyard Camera",
        .icon_text = LV_SYMBOL_EYE_OPEN,
    };

    ui_room_card_t* camera_card     = ui_room_card_create(content, &camera_config);
    lv_obj_t*       camera_card_obj = ui_room_card_get_obj(camera_card);
    lv_obj_set_style_pad_gap(camera_card_obj, 24, LV_PART_MAIN);

    lv_obj_t* camera_toggle = ui_room_card_get_toggle(camera_card);
    if (camera_toggle != NULL)
    {
        lv_obj_add_flag(camera_toggle, LV_OBJ_FLAG_HIDDEN);
    }

    lv_obj_t* camera_specs = lv_obj_get_child(camera_card_obj, -1);
    if (camera_specs != NULL)
    {
        lv_label_set_text(camera_specs, "Streaming · 1080p60");
    }

    lv_obj_t* video = lv_obj_create(camera_card_obj);
    lv_obj_remove_style_all(video);
    lv_obj_set_width(video, LV_PCT(100));
    lv_obj_set_height(video, 280);
    lv_obj_set_style_bg_color(video, ui_theme_color_outline(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(video, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_radius(video, 18, LV_PART_MAIN);
    lv_obj_set_style_border_width(video, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(video, 24, LV_PART_MAIN);

    lv_obj_t* video_label = lv_label_create(video);
    lv_label_set_text(video_label, "Live feed (stub)");
    lv_obj_set_style_text_font(video_label, &lv_font_montserrat_22, LV_PART_MAIN);
    lv_obj_set_style_text_color(video_label, ui_theme_color_on_surface(), LV_PART_MAIN);
    lv_obj_center(video_label);

    lv_obj_t* events_row = lv_obj_create(content);
    lv_obj_remove_style_all(events_row);
    lv_obj_set_width(events_row, LV_PCT(100));
    lv_obj_set_style_bg_opa(events_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(events_row, 24, LV_PART_MAIN);
    lv_obj_set_flex_flow(events_row, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(
        events_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    static const struct
    {
        const char* room_id;
        const char* title;
        const char* description;
        const char* timestamp;
    } k_event_cards[] = {
        {"door", "Front Door", "Person @ Door – 2m", "Today · 12:42"},
        {"garage", "Garage", "Motion – 5m", "Today · 12:37"},
        {"yard", "Backyard", "Person @ Gate – 8m", "Today · 12:20"},
        {"drive", "Driveway", "Vehicle Detected", "Today · 11:58"},
    };

    for (size_t i = 0; i < (sizeof(k_event_cards) / sizeof(k_event_cards[0])); i++)
    {
        ui_room_card_config_t event_config = {
            .room_id   = k_event_cards[i].room_id,
            .title     = k_event_cards[i].title,
            .icon_text = LV_SYMBOL_VIDEO,
        };

        ui_room_card_t* event_card = ui_room_card_create(events_row, &event_config);
        lv_obj_t*       event_obj  = ui_room_card_get_obj(event_card);
        lv_obj_set_width(event_obj, 280);
        lv_obj_set_style_pad_gap(event_obj, 16, LV_PART_MAIN);

        lv_obj_t* event_toggle = ui_room_card_get_toggle(event_card);
        if (event_toggle != NULL)
        {
            lv_obj_add_flag(event_toggle, LV_OBJ_FLAG_HIDDEN);
        }

        lv_obj_t* event_description = lv_label_create(event_obj);
        lv_label_set_text(event_description, k_event_cards[i].description);
        lv_obj_set_style_text_font(event_description, &lv_font_montserrat_18, LV_PART_MAIN);
        lv_obj_set_style_text_color(event_description, ui_theme_color_on_surface(), LV_PART_MAIN);
        lv_label_set_long_mode(event_description, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(event_description, LV_PCT(100));

        lv_obj_t* event_time = lv_obj_get_child(event_obj, -1);
        if (event_time != NULL)
        {
            lv_label_set_text(event_time, k_event_cards[i].timestamp);
            lv_obj_set_style_text_color(event_time, ui_theme_color_muted(), LV_PART_MAIN);
        }
    }

    lv_obj_t* actions_row = lv_obj_create(content);
    lv_obj_remove_style_all(actions_row);
    lv_obj_set_width(actions_row, LV_PCT(100));
    lv_obj_set_style_bg_opa(actions_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(actions_row, 24, LV_PART_MAIN);
    lv_obj_set_flex_flow(actions_row, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(
        actions_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    static const char* k_actions[] = {
        "Open Gate",
        "Talk",
        "Snapshot",
        "Timeline ▾",
    };

    for (size_t i = 0; i < (sizeof(k_actions) / sizeof(k_actions[0])); i++)
    {
        lv_obj_t* action_btn = lv_btn_create(actions_row);
        lv_obj_remove_style_all(action_btn);
        lv_obj_set_style_bg_color(action_btn, ui_theme_color_surface(), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(action_btn, LV_OPA_80, LV_PART_MAIN);
        lv_obj_set_style_radius(action_btn, 26, LV_PART_MAIN);
        lv_obj_set_style_border_width(action_btn, 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(action_btn, ui_theme_color_outline(), LV_PART_MAIN);
        lv_obj_set_style_pad_hor(action_btn, 28, LV_PART_MAIN);
        lv_obj_set_style_pad_ver(action_btn, 16, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(action_btn, 0, LV_PART_MAIN);
        lv_obj_set_style_text_color(action_btn, ui_theme_color_on_surface(), LV_PART_MAIN);

        lv_obj_t* action_label = lv_label_create(action_btn);
        lv_label_set_text(action_label, k_actions[i]);
        lv_obj_set_style_text_font(action_label, &lv_font_montserrat_18, LV_PART_MAIN);
    }

    return content;
}

lv_obj_t* ui_page_cctv_create(lv_obj_t* parent)
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
        lv_obj_add_event_cb(page, ui_page_cctv_delete_cb, LV_EVENT_DELETE, wallpaper);
    }

    ui_page_create_content(page, "Frigate Security");

    return page;
}
