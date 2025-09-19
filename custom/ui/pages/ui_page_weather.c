/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ui_page_weather.h"

#include "../ui_theme.h"
#include "../ui_wallpaper.h"
#include "../widgets/ui_room_card.h"

static lv_obj_t* create_metric_block(lv_obj_t* parent, const char* title, const char* value)
{
    lv_obj_t* block = lv_obj_create(parent);
    lv_obj_remove_style_all(block);
    lv_obj_set_width(block, LV_PCT(100));
    lv_obj_set_style_bg_opa(block, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_flex_flow(block, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_gap(block, 4, LV_PART_MAIN);

    lv_obj_t* title_label = lv_label_create(block);
    lv_label_set_text(title_label, title != NULL ? title : "");
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(title_label, ui_theme_color_muted(), LV_PART_MAIN);

    lv_obj_t* value_label = lv_label_create(block);
    lv_label_set_text(value_label, value != NULL ? value : "");
    lv_obj_set_style_text_font(value_label, &lv_font_montserrat_22, LV_PART_MAIN);
    lv_obj_set_style_text_color(value_label, ui_theme_color_on_surface(), LV_PART_MAIN);

    return block;
}

static void
create_forecast_item(lv_obj_t* parent, const char* day, const char* icon, const char* range)
{
    lv_obj_t* item = lv_obj_create(parent);
    lv_obj_remove_style_all(item);
    lv_obj_set_size(item, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_bg_color(item, ui_theme_color_surface(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(item, LV_OPA_60, LV_PART_MAIN);
    lv_obj_set_style_radius(item, 14, LV_PART_MAIN);
    lv_obj_set_style_pad_all(item, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(item, 8, LV_PART_MAIN);
    lv_obj_set_style_border_width(item, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(item, ui_theme_color_outline(), LV_PART_MAIN);
    lv_obj_set_style_border_opa(item, LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_flex_flow(item, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(item, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_flex_grow(item, 1);

    lv_obj_t* day_label = lv_label_create(item);
    lv_label_set_text(day_label, day != NULL ? day : "");
    lv_obj_set_style_text_font(day_label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(day_label, ui_theme_color_muted(), LV_PART_MAIN);

    lv_obj_t* icon_label = lv_label_create(item);
    lv_label_set_text(icon_label, icon != NULL ? icon : LV_SYMBOL_MINUS);
    lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_set_style_text_color(icon_label, ui_theme_color_accent(), LV_PART_MAIN);

    lv_obj_t* range_label = lv_label_create(item);
    lv_label_set_text(range_label, range != NULL ? range : "");
    lv_obj_set_style_text_font(range_label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_style_text_color(range_label, ui_theme_color_on_surface(), LV_PART_MAIN);
}

static void ui_page_weather_delete_cb(lv_event_t* event)
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
    lv_obj_set_style_pad_row(content, 32, LV_PART_MAIN);
    lv_obj_set_flex_flow(content, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(content, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    lv_obj_t* title = lv_label_create(content);
    lv_label_set_text(title, title_text != NULL ? title_text : "");
    lv_obj_set_width(title, LV_PCT(100));
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, ui_theme_color_on_surface(), LV_PART_MAIN);

    ui_room_card_config_t living_config = {
        .room_id   = "living_room",
        .title     = "Living Room Climate",
        .icon_text = LV_SYMBOL_HOME,
    };
    ui_room_card_t* living_card = ui_room_card_create(content, &living_config);
    if (living_card != NULL)
    {
        lv_obj_t* living_obj = ui_room_card_get_obj(living_card);
        lv_obj_t* toggle     = ui_room_card_get_toggle(living_card);
        if (toggle != NULL)
        {
            lv_obj_add_flag(toggle, LV_OBJ_FLAG_HIDDEN);
        }
        if (living_obj != NULL)
        {
            lv_obj_t* specs = lv_obj_get_child(living_obj, -1);
            if (specs != NULL)
            {
                lv_obj_add_flag(specs, LV_OBJ_FLAG_HIDDEN);
            }

            lv_obj_t* metrics = lv_obj_create(living_obj);
            lv_obj_remove_style_all(metrics);
            lv_obj_set_width(metrics, LV_PCT(100));
            lv_obj_set_style_bg_opa(metrics, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_pad_gap(metrics, 16, LV_PART_MAIN);
            lv_obj_set_flex_flow(metrics, LV_FLEX_FLOW_COLUMN);

            create_metric_block(metrics, "Indoor Temperature", "72 \u00B0F");
            create_metric_block(metrics, "Humidity", "45% RH");
            create_metric_block(metrics, "HVAC Mode", "Auto (Cooling)");
        }
    }

    ui_room_card_config_t outdoor_config = {
        .room_id   = "outdoor",
        .title     = "Outdoor Sensors",
        .icon_text = LV_SYMBOL_GPS,
    };
    ui_room_card_t* outdoor_card = ui_room_card_create(content, &outdoor_config);
    if (outdoor_card != NULL)
    {
        lv_obj_t* outdoor_obj = ui_room_card_get_obj(outdoor_card);
        lv_obj_t* toggle      = ui_room_card_get_toggle(outdoor_card);
        if (toggle != NULL)
        {
            lv_obj_add_flag(toggle, LV_OBJ_FLAG_HIDDEN);
        }
        if (outdoor_obj != NULL)
        {
            lv_obj_t* specs = lv_obj_get_child(outdoor_obj, -1);
            if (specs != NULL)
            {
                lv_obj_add_flag(specs, LV_OBJ_FLAG_HIDDEN);
            }

            lv_obj_t* metrics = lv_obj_create(outdoor_obj);
            lv_obj_remove_style_all(metrics);
            lv_obj_set_width(metrics, LV_PCT(100));
            lv_obj_set_style_bg_opa(metrics, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_pad_gap(metrics, 16, LV_PART_MAIN);
            lv_obj_set_flex_flow(metrics, LV_FLEX_FLOW_COLUMN);

            create_metric_block(metrics, "sensor.tab5_temperature", "18 \u00B0C");
            create_metric_block(metrics, "sensor.tab5_humidity", "52 %");
        }
    }

    lv_obj_t* forecast_label = lv_label_create(content);
    lv_label_set_text(forecast_label, "Forecast");
    lv_obj_set_style_text_font(forecast_label, &lv_font_montserrat_24, LV_PART_MAIN);
    lv_obj_set_style_text_color(forecast_label, ui_theme_color_on_surface(), LV_PART_MAIN);

    lv_obj_t* forecast = lv_obj_create(content);
    lv_obj_remove_style_all(forecast);
    lv_obj_set_width(forecast, LV_PCT(100));
    lv_obj_set_style_bg_color(forecast, ui_theme_color_surface(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(forecast, LV_OPA_50, LV_PART_MAIN);
    lv_obj_set_style_radius(forecast, 18, LV_PART_MAIN);
    lv_obj_set_style_border_width(forecast, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(forecast, ui_theme_color_outline(), LV_PART_MAIN);
    lv_obj_set_style_border_opa(forecast, LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_pad_all(forecast, 20, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(forecast, 16, LV_PART_MAIN);
    lv_obj_set_flex_flow(forecast, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(
        forecast, LV_FLEX_ALIGN_SPACE_AROUND, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    create_forecast_item(forecast, "Today", LV_SYMBOL_REFRESH, "75° / 62°");
    create_forecast_item(forecast, "Tomorrow", LV_SYMBOL_DOWNLOAD, "78° / 64°");
    create_forecast_item(forecast, "Sat", LV_SYMBOL_UPLOAD, "80° / 66°");

    return content;
}

lv_obj_t* ui_page_weather_create(lv_obj_t* parent)
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
        lv_obj_add_event_cb(page, ui_page_weather_delete_cb, LV_EVENT_DELETE, wallpaper);
    }

    ui_page_create_content(page, "Local Climate Station");

    return page;
}
