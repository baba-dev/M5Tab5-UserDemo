/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ui_home.h"

// ---- small helpers ----
static void set_bg(lv_obj_t* obj, lv_color_t color)
{
    lv_obj_set_style_bg_color(obj, color, 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
}

static lv_obj_t*
make_label(lv_obj_t* parent, const char* txt, const lv_font_t* font, lv_color_t color)
{
    lv_obj_t* label = lv_label_create(parent);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, color, 0);
    lv_label_set_text(label, txt);
    return label;
}

static lv_obj_t* make_divider(lv_obj_t* parent)
{
    lv_obj_t* line = lv_obj_create(parent);
    lv_obj_remove_style_all(line);
    lv_obj_set_height(line, 2);
    set_bg(line, lv_color_hex(0x223049));
    lv_obj_set_width(line, LV_PCT(100));
    return line;
}

static lv_obj_t* make_card(lv_obj_t* grid, const char* title, const char* body)
{
    lv_obj_t* card = lv_obj_create(grid);
    set_bg(card, lv_color_hex(0x0F172A));
    lv_obj_set_style_radius(card, 20, 0);
    lv_obj_set_style_pad_all(card, 16, 0);
    lv_obj_set_style_pad_bottom(card, 18, 0);
    lv_obj_set_style_border_width(card, 3, 0);
    lv_obj_set_style_border_color(card, lv_color_hex(0x10B981), 0);

    lv_obj_t* header = lv_obj_create(card);
    lv_obj_remove_style_all(header);
    lv_obj_set_style_radius(header, 14, 0);
    lv_obj_set_style_pad_all(header, 10, 0);
    set_bg(header, lv_color_hex(0x10B981));
    lv_obj_set_width(header, LV_PCT(100));
    lv_obj_set_height(header, 44);

    make_label(header, title, &lv_font_montserrat_20, lv_color_white());

    lv_obj_t* text = make_label(card, body, &lv_font_montserrat_18, lv_color_hex(0x94A3B8));
    lv_obj_align_to(text, header, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 12);

    return card;
}

lv_obj_t* ui_home_create(lv_obj_t* parent)
{
    lv_obj_t* root = parent ? parent : lv_scr_act();
    set_bg(root, lv_color_hex(0x0B1220));

    lv_obj_set_layout(root, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(root, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(root, 0, 0);

    lv_obj_t* rail = lv_obj_create(root);
    lv_obj_remove_style_all(rail);
    lv_obj_set_width(rail, 96);
    lv_obj_set_height(rail, LV_PCT(100));

    lv_obj_t* content = lv_obj_create(root);
    lv_obj_remove_style_all(content);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100));
    lv_obj_set_flex_grow(content, 1);

    lv_obj_t* header = lv_obj_create(content);
    lv_obj_remove_style_all(header);
    lv_obj_set_layout(header, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_pad_all(header, 16, 0);
    lv_obj_set_style_pad_column(header, 16, 0);
    lv_obj_set_width(header, LV_PCT(100));
    lv_obj_set_height(header, 120);

    lv_obj_t* left = lv_obj_create(header);
    lv_obj_remove_style_all(left);
    lv_obj_set_layout(left, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(left, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(left, 4, 0);
    lv_obj_set_width(left, LV_PCT(100));
    lv_obj_set_flex_grow(left, 1);

    make_label(left, "HH:MM:SS", &lv_font_montserrat_28, lv_color_hex(0xE5E7EB));
    make_label(left, "DD:MM:YYYY", &lv_font_montserrat_18, lv_color_hex(0x94A3B8));
    make_label(left,
               "Weather Forecast: Feeling Cloudy, Pack a raincoat",
               &lv_font_montserrat_18,
               lv_color_hex(0xE5E7EB));

    lv_obj_t* right = lv_obj_create(header);
    lv_obj_remove_style_all(right);
    lv_obj_set_layout(right, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(right, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(right, 0, 0);
    lv_obj_set_width(right, 320);
    lv_obj_set_height(right, LV_PCT(100));

    lv_obj_t* red = lv_obj_create(right);
    lv_obj_remove_style_all(red);
    set_bg(red, lv_color_hex(0xEF4444));
    lv_obj_set_size(red, 320, 78);
    lv_obj_align(red, LV_ALIGN_TOP_RIGHT, 0, 0);

    lv_obj_t* aqi = make_label(right, "AQI: XXX", &lv_font_montserrat_18, lv_color_hex(0xEF4444));
    lv_obj_align(aqi, LV_ALIGN_BOTTOM_RIGHT, 0, 0);

    make_divider(content);

    lv_obj_t* title =
        make_label(content, "Page Title: Homepage", &lv_font_montserrat_20, lv_color_hex(0xE5E7EB));
    lv_obj_set_style_pad_top(title, 8, 0);
    lv_obj_set_style_pad_left(title, 16, 0);

    lv_obj_t* grid = lv_obj_create(content);
    lv_obj_remove_style_all(grid);
    lv_obj_set_layout(grid, LV_LAYOUT_GRID);

    static int32_t cols[] = {LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static int32_t rows[] = {LV_GRID_CONTENT, LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};
    lv_obj_set_grid_dsc_array(grid, cols, rows);

    lv_obj_set_style_pad_row(grid, 12, 0);
    lv_obj_set_style_pad_column(grid, 12, 0);
    lv_obj_set_style_pad_left(grid, 16, 0);
    lv_obj_set_style_pad_right(grid, 16, 0);
    lv_obj_set_style_pad_top(grid, 12, 0);
    lv_obj_set_width(grid, LV_PCT(100));
    lv_obj_set_height(grid, LV_PCT(100));

    lv_obj_t* card_bakery = make_card(grid,
                                      "(ICON): Bakery",
                                      "Temp: X Celsius\nX Lights are currently on.\n\nTurn off    "
                                      "Controls\n\nX Ventilation are currently on.\n\nTurn Off");
    lv_obj_set_grid_cell(card_bakery, LV_GRID_ALIGN_STRETCH, 0, 1, LV_GRID_ALIGN_START, 0, 1);

    lv_obj_t* card_bedroom = make_card(grid, "(ICON): Bedroom", "Same as Bakery Card");
    lv_obj_set_grid_cell(card_bedroom, LV_GRID_ALIGN_STRETCH, 1, 1, LV_GRID_ALIGN_START, 0, 1);

    lv_obj_t* card_outside = make_card(grid, "(ICON): Outside", "Same as bakery card");
    lv_obj_set_grid_cell(card_outside, LV_GRID_ALIGN_STRETCH, 2, 1, LV_GRID_ALIGN_START, 0, 1);

    return root;
}
