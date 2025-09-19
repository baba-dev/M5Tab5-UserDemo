/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ui_nav_rail.h"

#include <stddef.h>

struct ui_nav_rail_t {
    lv_obj_t *container;
    lv_obj_t *buttons[UI_NAV_PAGE_COUNT];
    ui_nav_page_t active;
    ui_nav_rail_callback_t callback;
    void *user_data;
    bool visible;
    lv_coord_t base_offset;
    lv_coord_t hidden_offset;
};

static const char* k_nav_icons[UI_NAV_PAGE_COUNT] = {
    LV_SYMBOL_DRIVE,
    LV_SYMBOL_HOME,
    LV_SYMBOL_VIDEO,
    LV_SYMBOL_GPS,
    LV_SYMBOL_AUDIO,
    LV_SYMBOL_SETTINGS,
};

static const char* k_nav_labels[UI_NAV_PAGE_COUNT] = {
    "ESP32P4",
    "Rooms",
    "Frigate Security",
    "Local Climate",
    "Media",
    "Settings",
};

static void ui_nav_button_event_cb(lv_event_t *event)
{
    if (event == NULL) {
        return;
    }

    ui_nav_rail_t *rail = (ui_nav_rail_t *)lv_event_get_user_data(event);
    if (rail == NULL || lv_event_get_code(event) != LV_EVENT_CLICKED) {
        return;
    }

    lv_obj_t *target = lv_event_get_target(event);
    for (uint32_t i = 0; i < UI_NAV_PAGE_COUNT; i++) {
        if (rail->buttons[i] == target) {
            if (rail->callback != NULL) {
                rail->callback(rail, (ui_nav_page_t)i, rail->user_data);
            }
            break;
        }
    }
}

static void ui_nav_apply_button_style(lv_obj_t *button)
{
    lv_obj_set_width(button, LV_PCT(100));
    lv_obj_set_style_radius(button, 16, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x1b2430), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(button, LV_OPA_90, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, lv_color_hex(0x2563eb), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(button, LV_OPA_100, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_width(button, 18, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_opa(button, LV_OPA_60, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_shadow_ofs_y(button, 6, LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_border_width(button, 0, LV_PART_MAIN);
    lv_obj_set_style_outline_width(button, 2, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_color(button, lv_color_hex(0x38bdf8), LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_outline_pad(button, 4, LV_PART_MAIN | LV_STATE_FOCUS_KEY);
    lv_obj_set_style_text_color(button, lv_color_hex(0xe6edf3), LV_PART_MAIN);
    lv_obj_set_style_text_color(button, lv_color_hex(0xf8fafc), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_pad_all(button, 12, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(button, 6, LV_PART_MAIN);
    lv_obj_set_flex_flow(button, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(button, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
}

static void ui_nav_add_button_content(lv_obj_t *button, uint32_t index)
{
    lv_obj_t *icon = lv_label_create(button);
    lv_label_set_text(icon, k_nav_icons[index]);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_style_text_color(icon, lv_color_hex(0xe6edf3), LV_PART_MAIN);

    lv_obj_t *label = lv_label_create(button);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, LV_PCT(100));
    lv_label_set_text(label, k_nav_labels[index]);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_16, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, lv_color_hex(0xcad3df), LV_PART_MAIN);
}

static void ui_nav_rail_update_offsets(ui_nav_rail_t *rail)
{
    if (rail == NULL || rail->container == NULL) {
        return;
    }

    lv_coord_t width    = lv_obj_get_width(rail->container);
    rail->hidden_offset = -(width + rail->base_offset);

    if (!rail->visible) {
        lv_obj_set_style_translate_x(rail->container, rail->hidden_offset, LV_PART_MAIN);
    }
}

static void ui_nav_container_event_cb(lv_event_t *event)
{
    if (event == NULL) {
        return;
    }

    if (lv_event_get_code(event) != LV_EVENT_SIZE_CHANGED) {
        return;
    }

    ui_nav_rail_t *rail = (ui_nav_rail_t *)lv_event_get_user_data(event);
    ui_nav_rail_update_offsets(rail);
}

ui_nav_rail_t *ui_nav_rail_create(lv_obj_t *parent, ui_nav_rail_callback_t callback, void *user_data)
{
    if (parent == NULL) {
        return NULL;
    }

    ui_nav_rail_t *rail = (ui_nav_rail_t *)lv_malloc(sizeof(ui_nav_rail_t));
    if (rail == NULL) {
        return NULL;
    }
    lv_memset(rail, 0, sizeof(ui_nav_rail_t));

    rail->container = lv_obj_create(parent);
    lv_obj_set_size(rail->container, 156, LV_PCT(100));
    lv_obj_align(rail->container, LV_ALIGN_LEFT_MID, 24, 0);
    lv_obj_remove_flag(rail->container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(rail->container, 20, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rail->container, lv_color_hex(0x11161d), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(rail->container, LV_OPA_90, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(rail->container, 28, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(rail->container, LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_shadow_ofs_x(rail->container, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_ofs_y(rail->container, 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(rail->container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(rail->container, 18, LV_PART_MAIN);
    lv_obj_set_style_pad_row(rail->container, 18, LV_PART_MAIN);
    lv_obj_set_style_pad_top(rail->container, 32, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(rail->container, 32, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(rail->container, 16, LV_PART_MAIN);
    lv_obj_set_flex_flow(rail->container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(rail->container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    rail->callback  = callback;
    rail->user_data = user_data;

    for (uint32_t i = 0; i < UI_NAV_PAGE_COUNT; i++) {
        lv_obj_t *button = lv_button_create(rail->container);
        lv_obj_add_flag(button, LV_OBJ_FLAG_CHECKABLE);
        lv_obj_add_event_cb(button, ui_nav_button_event_cb, LV_EVENT_CLICKED, rail);
        ui_nav_apply_button_style(button);
        ui_nav_add_button_content(button, i);
        rail->buttons[i] = button;
    }

    ui_nav_rail_set_active(rail, UI_NAV_PAGE_DEFAULT);

    rail->base_offset = lv_obj_get_x(rail->container);
    rail->visible     = false;
    ui_nav_rail_update_offsets(rail);
    lv_obj_set_style_translate_x(rail->container, rail->hidden_offset, LV_PART_MAIN);
    lv_obj_add_flag(rail->container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(rail->container, ui_nav_container_event_cb, LV_EVENT_SIZE_CHANGED, rail);

    return rail;
}

void ui_nav_rail_destroy(ui_nav_rail_t *rail)
{
    if (rail == NULL) {
        return;
    }

    if (rail->container != NULL) {
        lv_obj_del(rail->container);
        rail->container = NULL;
    }

    lv_free(rail);
}

void ui_nav_rail_set_active(ui_nav_rail_t *rail, ui_nav_page_t page)
{
    if (rail == NULL || page >= UI_NAV_PAGE_COUNT) {
        return;
    }

    rail->active = page;
    for (uint32_t i = 0; i < UI_NAV_PAGE_COUNT; i++) {
        lv_obj_t *button = rail->buttons[i];
        if (button == NULL) {
            continue;
        }

        if (i == (uint32_t)page) {
            lv_obj_add_state(button, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(button, LV_STATE_CHECKED);
        }
    }
}

lv_obj_t *ui_nav_rail_get_container(ui_nav_rail_t *rail)
{
    if (rail == NULL) {
        return NULL;
    }
    return rail->container;
}

void ui_nav_rail_show(ui_nav_rail_t *rail, bool animate)
{
    if (rail == NULL || rail->container == NULL) {
        return;
    }

    if (rail->visible) {
        return;
    }

    LV_UNUSED(animate);

    lv_obj_clear_flag(rail->container, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_translate_x(rail->container, 0, LV_PART_MAIN);
    lv_obj_move_foreground(rail->container);
    rail->visible     = true;
    rail->base_offset = lv_obj_get_x(rail->container);
    ui_nav_rail_update_offsets(rail);
}

void ui_nav_rail_hide(ui_nav_rail_t *rail, bool animate)
{
    if (rail == NULL || rail->container == NULL) {
        return;
    }

    if (!rail->visible) {
        lv_obj_set_style_translate_x(rail->container, rail->hidden_offset, LV_PART_MAIN);
        return;
    }

    LV_UNUSED(animate);

    lv_obj_set_style_translate_x(rail->container, rail->hidden_offset, LV_PART_MAIN);
    lv_obj_add_flag(rail->container, LV_OBJ_FLAG_HIDDEN);
    rail->visible = false;
}

bool ui_nav_rail_is_visible(const ui_nav_rail_t *rail)
{
    if (rail == NULL) {
        return false;
    }
    return rail->visible;
}

lv_coord_t ui_nav_rail_get_hidden_offset(const ui_nav_rail_t *rail)
{
    if (rail == NULL) {
        return 0;
    }
    return rail->hidden_offset;
}
