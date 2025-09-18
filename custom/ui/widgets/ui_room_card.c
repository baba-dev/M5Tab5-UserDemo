/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ui_room_card.h"

#include <stdio.h>

#include "../ui_theme.h"

struct ui_room_card_t
{
    lv_obj_t*   container;
    lv_obj_t*   icon_label;
    lv_obj_t*   title_label;
    lv_obj_t*   toggle_btn;
    lv_obj_t*   toggle_label;
    lv_obj_t*   specs_label;
    bool        on;
    bool        available;
    int8_t      temp_c;
    uint8_t     humidity;
    const char* room_id;
    const char* entity_id;
};

static void card_delete_cb(lv_event_t* event)
{
    if (event == NULL)
    {
        return;
    }
    ui_room_card_t* card = (ui_room_card_t*)lv_event_get_user_data(event);
    if (card == NULL)
    {
        return;
    }
    lv_obj_remove_event_cb(card->container, card_delete_cb);
    card->container    = NULL;
    card->icon_label   = NULL;
    card->title_label  = NULL;
    card->toggle_btn   = NULL;
    card->toggle_label = NULL;
    card->specs_label  = NULL;
    lv_free(card);
}

static void set_card_base_style(lv_obj_t* container)
{
    lv_obj_remove_style_all(container);
    lv_obj_set_width(container, LV_PCT(100));
    lv_obj_set_style_bg_color(container, ui_theme_color_surface(), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(
        container, lv_color_mix(ui_theme_color_surface(), lv_color_white(), 32), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_dir(container, LV_GRAD_DIR_VER, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(container, LV_OPA_70, LV_PART_MAIN);
    lv_obj_set_style_border_width(container, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(container, 18, LV_PART_MAIN);
    lv_obj_set_style_pad_all(container, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(container, 18, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(container, 6, LV_PART_MAIN);
    lv_obj_set_style_shadow_ofs_y(container, 8, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(container, LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(container, ui_theme_color_outline(), LV_PART_MAIN);
    lv_obj_set_style_text_color(container, ui_theme_color_on_surface(), LV_PART_MAIN);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_add_flag(container, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_flag(container, LV_OBJ_FLAG_GESTURE_BUBBLE);
}

static lv_obj_t* create_header(lv_obj_t* parent, const ui_room_card_config_t* config)
{
    lv_obj_t* header = lv_obj_create(parent);
    lv_obj_remove_style_all(header);
    lv_obj_set_width(header, LV_PCT(100));
    lv_obj_set_style_bg_opa(header, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_flex_flow(header, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(header, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(header, 12, LV_PART_MAIN);

    lv_obj_t* icon = lv_label_create(header);
    lv_label_set_text(icon, config->icon_text != NULL ? config->icon_text : LV_SYMBOL_HOME);
    lv_obj_set_style_text_color(icon, ui_theme_color_muted(), LV_PART_MAIN);
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_28, LV_PART_MAIN);

    lv_obj_t* title = lv_label_create(header);
    lv_label_set_text(title, config->title != NULL ? config->title : "Room");
    lv_obj_set_style_text_color(title, ui_theme_color_on_surface(), LV_PART_MAIN);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_22, LV_PART_MAIN);

    LV_UNUSED(icon);
    LV_UNUSED(title);

    return header;
}

static lv_obj_t* create_toggle(lv_obj_t* parent)
{
    lv_obj_t* btn = lv_btn_create(parent);
    lv_obj_remove_style_all(btn);
    lv_obj_set_width(btn, LV_PCT(100));
    lv_obj_set_height(btn, 44);
    lv_obj_set_style_radius(btn, 22, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(btn, LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_bg_color(btn, ui_theme_color_surface(), LV_PART_MAIN);
    lv_obj_set_style_border_color(btn, ui_theme_color_outline(), LV_PART_MAIN);
    lv_obj_set_style_border_opa(btn, LV_OPA_60, LV_PART_MAIN);
    lv_obj_set_style_border_width(btn, 1, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(btn, 0, LV_PART_MAIN);
    lv_obj_set_style_text_color(btn, ui_theme_color_on_surface(), LV_PART_MAIN);

    lv_obj_t* label = lv_label_create(btn);
    lv_obj_center(label);
    lv_label_set_text(label, "Off");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, ui_theme_color_on_surface(), LV_PART_MAIN);

    return btn;
}

static lv_obj_t* create_specs_label(lv_obj_t* parent)
{
    lv_obj_t* label = lv_label_create(parent);
    lv_label_set_text(label, "24 °C · 48% RH");
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_set_style_text_color(label, ui_theme_color_muted(), LV_PART_MAIN);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, LV_PCT(100));
    return label;
}

static void update_specs(ui_room_card_t* card)
{
    if (card == NULL || card->specs_label == NULL)
    {
        return;
    }
    char buffer[48];
    lv_snprintf(
        buffer, sizeof(buffer), "%d °C · %u%% RH", (int)card->temp_c, (unsigned int)card->humidity);
    lv_label_set_text(card->specs_label, buffer);
}

static void apply_state_styles(ui_room_card_t* card)
{
    if (card == NULL || card->container == NULL)
    {
        return;
    }

    lv_color_t accent     = ui_theme_color_accent();
    lv_color_t surface    = ui_theme_color_surface();
    lv_color_t muted      = ui_theme_color_muted();
    lv_color_t on_surface = ui_theme_color_on_surface();

    lv_obj_set_style_bg_color(card->toggle_btn, card->on ? accent : surface, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(card->toggle_btn, card->on ? LV_OPA_COVER : LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_border_width(card->toggle_btn, card->on ? 0 : 1, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(card->toggle_btn, card->on ? 10 : 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(
        card->toggle_btn, card->on ? LV_OPA_40 : LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(card->toggle_btn, accent, LV_PART_MAIN);

    if (card->toggle_label != NULL)
    {
        lv_obj_set_style_text_color(
            card->toggle_label, card->on ? on_surface : on_surface, LV_PART_MAIN);
        lv_label_set_text(card->toggle_label, card->on ? "On" : "Off");
    }

    if (card->icon_label != NULL)
    {
        lv_obj_set_style_text_color(card->icon_label, card->on ? accent : muted, LV_PART_MAIN);
    }

    lv_obj_set_style_shadow_width(card->container, card->on ? 10 : 6, LV_PART_MAIN);
    lv_obj_set_style_shadow_opa(card->container, card->on ? LV_OPA_50 : LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_shadow_color(
        card->container, card->on ? accent : ui_theme_color_outline(), LV_PART_MAIN);
    lv_obj_set_style_bg_grad_color(card->container,
                                   card->on ? lv_color_mix(accent, surface, 200)
                                            : lv_color_mix(surface, lv_color_white(), 32),
                                   LV_PART_MAIN);
    lv_obj_set_style_bg_opa(card->container, card->available ? LV_OPA_70 : LV_OPA_40, LV_PART_MAIN);
    lv_obj_set_style_opa(card->container, card->available ? LV_OPA_COVER : LV_OPA_70, LV_PART_MAIN);
}

ui_room_card_t* ui_room_card_create(lv_obj_t* parent, const ui_room_card_config_t* config)
{
    if (parent == NULL)
    {
        return NULL;
    }

    ui_room_card_t* card = (ui_room_card_t*)lv_malloc(sizeof(ui_room_card_t));
    if (card == NULL)
    {
        return NULL;
    }

    card->container    = lv_obj_create(parent);
    card->icon_label   = NULL;
    card->title_label  = NULL;
    card->toggle_btn   = NULL;
    card->toggle_label = NULL;
    card->specs_label  = NULL;
    card->on           = false;
    card->available    = true;
    card->temp_c       = 24;
    card->humidity     = 48;
    card->room_id      = config != NULL ? config->room_id : NULL;
    card->entity_id    = NULL;

    set_card_base_style(card->container);
    lv_obj_add_event_cb(card->container, card_delete_cb, LV_EVENT_DELETE, card);

    lv_obj_t* header = create_header(card->container, config);
    lv_obj_t* icon   = lv_obj_get_child(header, 0);
    lv_obj_t* title  = lv_obj_get_child(header, 1);

    card->icon_label  = icon;
    card->title_label = title;

    card->toggle_btn   = create_toggle(card->container);
    card->toggle_label = lv_obj_get_child(card->toggle_btn, 0);

    card->specs_label = create_specs_label(card->container);

    update_specs(card);
    apply_state_styles(card);

    return card;
}

void ui_room_card_destroy(ui_room_card_t* card)
{
    if (card == NULL)
    {
        return;
    }
    if (card->container != NULL)
    {
        lv_obj_del(card->container);
    }
    else
    {
        lv_free(card);
    }
}

lv_obj_t* ui_room_card_get_obj(ui_room_card_t* card)
{
    return card != NULL ? card->container : NULL;
}

lv_obj_t* ui_room_card_get_toggle(ui_room_card_t* card)
{
    return card != NULL ? card->toggle_btn : NULL;
}

void ui_room_card_set_state(ui_room_card_t* card, const ui_room_card_state_t* state)
{
    if (card == NULL || state == NULL)
    {
        return;
    }
    card->on        = state->on;
    card->available = state->available;
    card->temp_c    = state->temp_c;
    card->humidity  = state->humidity;
    card->entity_id = state->primary_entity_id;
    update_specs(card);
    apply_state_styles(card);
}

const char* ui_room_card_get_room_id(const ui_room_card_t* card)
{
    return card != NULL ? card->room_id : NULL;
}

const char* ui_room_card_get_entity_id(const ui_room_card_t* card)
{
    return card != NULL ? card->entity_id : NULL;
}

static void icon_scale_exec_cb(void* var, int32_t value)
{
    lv_obj_t* obj = (lv_obj_t*)var;
    if (obj == NULL)
    {
        return;
    }
    lv_obj_set_style_transform_scale(obj, value, LV_PART_MAIN);
}

static void button_shadow_exec_cb(void* var, int32_t value)
{
    lv_obj_t* obj = (lv_obj_t*)var;
    if (obj == NULL)
    {
        return;
    }
    lv_obj_set_style_shadow_width(obj, value, LV_PART_MAIN);
}

void ui_room_card_play_toggle_feedback(ui_room_card_t* card)
{
    if (card == NULL)
    {
        return;
    }

    if (card->icon_label != NULL)
    {
        lv_anim_t a;
        lv_anim_init(&a);
        lv_anim_set_var(&a, card->icon_label);
        lv_anim_set_values(&a, 256, 276);
        lv_anim_set_time(&a, 80);
        lv_anim_set_path_cb(&a, lv_anim_path_overshoot);
        lv_anim_set_exec_cb(&a, icon_scale_exec_cb);
        lv_anim_set_playback_time(&a, 80);
        lv_anim_start(&a);
    }

    if (card->toggle_btn != NULL)
    {
        lv_anim_t b;
        lv_anim_init(&b);
        lv_anim_set_var(&b, card->toggle_btn);
        lv_anim_set_values(&b, card->on ? 10 : 0, card->on ? 14 : 6);
        lv_anim_set_time(&b, 90);
        lv_anim_set_path_cb(&b, lv_anim_path_ease_in_out);
        lv_anim_set_exec_cb(&b, button_shadow_exec_cb);
        lv_anim_set_playback_time(&b, 90);
        lv_anim_start(&b);
    }
}

static void card_opa_exec_cb(void* var, int32_t value)
{
    lv_obj_t* obj = (lv_obj_t*)var;
    if (obj == NULL)
    {
        return;
    }
    lv_obj_set_style_opa(obj, (lv_opa_t)value, LV_PART_MAIN);
}

void ui_room_card_play_enter_anim(ui_room_card_t* card, uint32_t delay_ms)
{
    if (card == NULL || card->container == NULL)
    {
        return;
    }

    lv_obj_set_style_transform_scale(card->container, 245, LV_PART_MAIN);
    lv_obj_set_style_opa(card->container, LV_OPA_TRANSP, LV_PART_MAIN);

    lv_anim_t scale_anim;
    lv_anim_init(&scale_anim);
    lv_anim_set_var(&scale_anim, card->container);
    lv_anim_set_values(&scale_anim, 245, 256);
    lv_anim_set_time(&scale_anim, 160);
    lv_anim_set_delay(&scale_anim, delay_ms);
    lv_anim_set_path_cb(&scale_anim, lv_anim_path_ease_out);
    lv_anim_set_exec_cb(&scale_anim, icon_scale_exec_cb);
    lv_anim_start(&scale_anim);

    lv_anim_t opa_anim;
    lv_anim_init(&opa_anim);
    lv_anim_set_var(&opa_anim, card->container);
    lv_anim_set_values(&opa_anim, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_time(&opa_anim, 160);
    lv_anim_set_delay(&opa_anim, delay_ms + 20);
    lv_anim_set_path_cb(&opa_anim, lv_anim_path_ease_in);
    lv_anim_set_exec_cb(&opa_anim, card_opa_exec_cb);
    lv_anim_start(&opa_anim);
}
