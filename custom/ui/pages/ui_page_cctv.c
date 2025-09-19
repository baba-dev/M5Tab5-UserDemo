/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ui_page_cctv.h"

#include <string.h>

#include "../ui_theme.h"
#include "../ui_wallpaper.h"
#include "../widgets/ui_room_card.h"

typedef struct
{
    lv_obj_t*            card_obj;
    ui_page_cctv_event_t event;
} ui_page_cctv_event_slot_t;

typedef struct
{
    lv_obj_t*       page;
    lv_obj_t*       content;
    lv_obj_t*       toolbar;
    lv_obj_t*       prev_btn;
    lv_obj_t*       next_btn;
    lv_obj_t*       quality_btn;
    lv_obj_t*       mute_btn;
    lv_obj_t*       camera_index_label;
    lv_obj_t*       quality_label;
    lv_obj_t*       mute_label;
    ui_room_card_t* camera_card;
    lv_obj_t*       camera_title_label;
    lv_obj_t*       camera_specs_label;
    lv_obj_t*       video_container;
    lv_obj_t*       stream_label;
    lv_obj_t*       events_row;
    lv_obj_t*       actions_row;
    lv_obj_t*       open_gate_button;
    lv_obj_t*       talk_button;
    lv_obj_t*       snapshot_button;
    lv_obj_t*       timeline_button;
    ui_wallpaper_t* wallpaper;

    size_t camera_count;
    size_t active_index;
    bool   muted;
    bool   audio_supported;

    char* active_camera_id;
    char* active_stream_url;
    char* quality_label_text;

    ui_page_cctv_event_slot_t* event_slots;
    size_t                     event_count;
} ui_page_cctv_ctx_t;

static ui_page_cctv_ctx_t* s_ctx = NULL;

static char* duplicate_string(const char* source)
{
    if (source == NULL)
    {
        return NULL;
    }

    size_t length = strlen(source);
    char*  copy   = (char*)lv_malloc(length + 1);
    if (copy == NULL)
    {
        return NULL;
    }

    memcpy(copy, source, length);
    copy[length] = '\0';
    return copy;
}

static void replace_string(char** destination, const char* value)
{
    if (destination == NULL)
    {
        return;
    }

    if (*destination != NULL)
    {
        lv_free(*destination);
        *destination = NULL;
    }

    if (value != NULL)
    {
        *destination = duplicate_string(value);
    }
}

static void free_event_slot(ui_page_cctv_event_slot_t* slot)
{
    if (slot == NULL)
    {
        return;
    }

    if (slot->event.event_id != NULL)
    {
        lv_free((void*)slot->event.event_id);
        slot->event.event_id = NULL;
    }
    if (slot->event.camera_id != NULL)
    {
        lv_free((void*)slot->event.camera_id);
        slot->event.camera_id = NULL;
    }
    if (slot->event.title != NULL)
    {
        lv_free((void*)slot->event.title);
        slot->event.title = NULL;
    }
    if (slot->event.description != NULL)
    {
        lv_free((void*)slot->event.description);
        slot->event.description = NULL;
    }
    if (slot->event.timestamp != NULL)
    {
        lv_free((void*)slot->event.timestamp);
        slot->event.timestamp = NULL;
    }
    if (slot->event.clip_url != NULL)
    {
        lv_free((void*)slot->event.clip_url);
        slot->event.clip_url = NULL;
    }
    if (slot->event.snapshot_url != NULL)
    {
        lv_free((void*)slot->event.snapshot_url);
        slot->event.snapshot_url = NULL;
    }
    slot->card_obj = NULL;
}

static void clear_event_slots(ui_page_cctv_ctx_t* ctx)
{
    if (ctx == NULL || ctx->event_slots == NULL)
    {
        return;
    }

    for (size_t i = 0; i < ctx->event_count; i++)
    {
        free_event_slot(&ctx->event_slots[i]);
    }

    lv_free(ctx->event_slots);
    ctx->event_slots = NULL;
    ctx->event_count = 0;
}

static void update_toolbar(ui_page_cctv_ctx_t* ctx)
{
    if (ctx == NULL)
    {
        return;
    }

    bool has_camera = (ctx->camera_count > 0);

    if (ctx->camera_index_label != NULL)
    {
        if (has_camera)
        {
            lv_label_set_text_fmt(ctx->camera_index_label,
                                  "Camera %u of %u",
                                  (unsigned int)(ctx->active_index + 1),
                                  (unsigned int)ctx->camera_count);
        }
        else
        {
            lv_label_set_text(ctx->camera_index_label, "No cameras");
        }
    }

    if (ctx->prev_btn != NULL)
    {
        if (ctx->camera_count > 1)
        {
            lv_obj_clear_state(ctx->prev_btn, LV_STATE_DISABLED);
        }
        else
        {
            lv_obj_add_state(ctx->prev_btn, LV_STATE_DISABLED);
        }
    }

    if (ctx->next_btn != NULL)
    {
        if (ctx->camera_count > 1)
        {
            lv_obj_clear_state(ctx->next_btn, LV_STATE_DISABLED);
        }
        else
        {
            lv_obj_add_state(ctx->next_btn, LV_STATE_DISABLED);
        }
    }

    const char* quality_label = NULL;
    if (ctx->quality_label_text != NULL && ctx->quality_label_text[0] != '\0')
    {
        quality_label = ctx->quality_label_text;
    }

    if (ctx->quality_label != NULL)
    {
        if (quality_label != NULL)
        {
            lv_label_set_text_fmt(ctx->quality_label, "%s ▾", quality_label);
        }
        else
        {
            lv_label_set_text(ctx->quality_label, "Quality ▾");
        }
    }

    if (ctx->quality_btn != NULL)
    {
        if (has_camera)
        {
            lv_obj_clear_state(ctx->quality_btn, LV_STATE_DISABLED);
        }
        else
        {
            lv_obj_add_state(ctx->quality_btn, LV_STATE_DISABLED);
        }
    }

    if (ctx->mute_btn != NULL)
    {
        if (has_camera && ctx->audio_supported)
        {
            lv_obj_clear_state(ctx->mute_btn, LV_STATE_DISABLED);
        }
        else
        {
            lv_obj_add_state(ctx->mute_btn, LV_STATE_DISABLED);
        }
    }

    if (ctx->mute_label != NULL)
    {
        if (has_camera && ctx->audio_supported)
        {
            const char* mute_text = ctx->muted ? LV_SYMBOL_MUTE " Unmute" : LV_SYMBOL_AUDIO " Mute";
            lv_label_set_text(ctx->mute_label, mute_text);
        }
        else
        {
            lv_label_set_text(ctx->mute_label, LV_SYMBOL_AUDIO " Mute");
        }
    }
}

static void update_camera_card(ui_page_cctv_ctx_t* ctx, const ui_page_cctv_camera_t* camera)
{
    if (ctx == NULL || ctx->camera_card == NULL)
    {
        return;
    }

    if (ctx->camera_title_label != NULL)
    {
        if (camera != NULL && camera->name != NULL)
        {
            lv_label_set_text(ctx->camera_title_label, camera->name);
        }
        else
        {
            lv_label_set_text(ctx->camera_title_label, "Camera");
        }
    }

    if (ctx->camera_specs_label != NULL)
    {
        if (camera != NULL && camera->status != NULL)
        {
            lv_label_set_text(ctx->camera_specs_label, camera->status);
        }
        else
        {
            lv_label_set_text(ctx->camera_specs_label, "Select a camera to view live feed.");
        }
    }

    if (ctx->stream_label != NULL)
    {
        if (camera != NULL)
        {
            if (camera->stream_url != NULL && camera->stream_url[0] != '\0')
            {
                lv_label_set_text_fmt(ctx->stream_label, "Live stream: %s", camera->stream_url);
            }
            else if (camera->snapshot_url != NULL && camera->snapshot_url[0] != '\0')
            {
                lv_label_set_text_fmt(
                    ctx->stream_label, "Snapshot fallback: %s", camera->snapshot_url);
            }
            else
            {
                lv_label_set_text(ctx->stream_label, "Live stream unavailable");
            }
        }
        else
        {
            lv_label_set_text(ctx->stream_label, "Add cameras in Home Assistant to view feeds.");
        }
    }
}

static void event_card_clicked_cb(lv_event_t* event)
{
    if (event == NULL)
    {
        return;
    }

    ui_page_cctv_ctx_t* ctx = (ui_page_cctv_ctx_t*)lv_event_get_user_data(event);
    if (ctx == NULL || ctx->page == NULL || ctx->event_slots == NULL)
    {
        return;
    }

    lv_obj_t* target = lv_event_get_target(event);

    for (size_t i = 0; i < ctx->event_count; i++)
    {
        if (ctx->event_slots[i].card_obj == target)
        {
            ui_page_cctv_clip_event_t clip = {
                .event = &ctx->event_slots[i].event,
                .index = i,
            };
            lv_event_send(ctx->page, UI_PAGE_CCTV_EVENT_OPEN_CLIP, &clip);
            break;
        }
    }
}

static void
update_events(ui_page_cctv_ctx_t* ctx, const ui_page_cctv_event_t* events, size_t event_count)
{
    if (ctx == NULL || ctx->events_row == NULL)
    {
        return;
    }

    lv_obj_clean(ctx->events_row);
    clear_event_slots(ctx);

    if (events == NULL || event_count == 0)
    {
        lv_obj_t* placeholder = lv_label_create(ctx->events_row);
        lv_label_set_text(placeholder, "No recent activity");
        lv_obj_set_style_text_font(placeholder, &lv_font_montserrat_18, LV_PART_MAIN);
        lv_obj_set_style_text_color(placeholder, ui_theme_color_muted(), LV_PART_MAIN);

        if (ctx->timeline_button != NULL)
        {
            lv_obj_add_state(ctx->timeline_button, LV_STATE_DISABLED);
        }
        return;
    }

    ctx->event_slots =
        (ui_page_cctv_event_slot_t*)lv_malloc(sizeof(ui_page_cctv_event_slot_t) * event_count);
    if (ctx->event_slots == NULL)
    {
        ctx->event_count = 0;
        return;
    }
    lv_memset_00(ctx->event_slots, sizeof(ui_page_cctv_event_slot_t) * event_count);

    size_t created = 0;

    for (size_t i = 0; i < event_count; i++)
    {
        const ui_page_cctv_event_t* source = &events[i];

        ui_room_card_config_t event_config = {
            .room_id   = source->event_id,
            .title     = source->title,
            .icon_text = LV_SYMBOL_VIDEO,
        };

        ui_room_card_t* event_card = ui_room_card_create(ctx->events_row, &event_config);
        if (event_card == NULL)
        {
            continue;
        }

        lv_obj_t* event_obj = ui_room_card_get_obj(event_card);
        lv_obj_set_width(event_obj, 280);
        lv_obj_set_style_pad_gap(event_obj, 16, LV_PART_MAIN);

        lv_obj_t* toggle = ui_room_card_get_toggle(event_card);
        if (toggle != NULL)
        {
            lv_obj_add_flag(toggle, LV_OBJ_FLAG_HIDDEN);
        }

        lv_obj_t* description = lv_label_create(event_obj);
        if (source->description != NULL)
        {
            lv_label_set_text(description, source->description);
        }
        else
        {
            lv_label_set_text(description, "No description");
        }
        lv_obj_set_style_text_font(description, &lv_font_montserrat_18, LV_PART_MAIN);
        lv_obj_set_style_text_color(description, ui_theme_color_on_surface(), LV_PART_MAIN);
        lv_label_set_long_mode(description, LV_LABEL_LONG_WRAP);
        lv_obj_set_width(description, LV_PCT(100));

        lv_obj_t* time_label = lv_obj_get_child(event_obj, -1);
        if (time_label != NULL)
        {
            if (source->timestamp != NULL)
            {
                lv_label_set_text(time_label, source->timestamp);
            }
            else
            {
                lv_label_set_text(time_label, "");
            }
            lv_obj_set_style_text_color(time_label, ui_theme_color_muted(), LV_PART_MAIN);
        }

        ui_page_cctv_event_slot_t* slot = &ctx->event_slots[created++];
        slot->card_obj                  = event_obj;
        slot->event.event_id            = duplicate_string(source->event_id);
        slot->event.camera_id           = duplicate_string(source->camera_id);
        slot->event.title               = duplicate_string(source->title);
        slot->event.description         = duplicate_string(source->description);
        slot->event.timestamp           = duplicate_string(source->timestamp);
        slot->event.clip_url            = duplicate_string(source->clip_url);
        slot->event.snapshot_url        = duplicate_string(source->snapshot_url);

        lv_obj_add_event_cb(event_obj, event_card_clicked_cb, LV_EVENT_CLICKED, ctx);
    }

    ctx->event_count = created;

    if (ctx->timeline_button != NULL)
    {
        if (ctx->event_count > 0)
        {
            lv_obj_clear_state(ctx->timeline_button, LV_STATE_DISABLED);
        }
        else
        {
            lv_obj_add_state(ctx->timeline_button, LV_STATE_DISABLED);
        }
    }

    if (created == 0)
    {
        lv_free(ctx->event_slots);
        ctx->event_slots = NULL;

        lv_obj_t* placeholder = lv_label_create(ctx->events_row);
        lv_label_set_text(placeholder, "No recent activity");
        lv_obj_set_style_text_font(placeholder, &lv_font_montserrat_18, LV_PART_MAIN);
        lv_obj_set_style_text_color(placeholder, ui_theme_color_muted(), LV_PART_MAIN);
    }
}

static void toolbar_button_cb(lv_event_t* event)
{
    if (event == NULL)
    {
        return;
    }

    ui_page_cctv_ctx_t* ctx = (ui_page_cctv_ctx_t*)lv_event_get_user_data(event);
    if (ctx == NULL || ctx->page == NULL)
    {
        return;
    }

    lv_obj_t* target = lv_event_get_target(event);
    if (lv_obj_has_state(target, LV_STATE_DISABLED))
    {
        return;
    }

    ui_page_cctv_action_t action;

    if (target == ctx->prev_btn)
    {
        action = UI_PAGE_CCTV_ACTION_PREVIOUS;
    }
    else if (target == ctx->next_btn)
    {
        action = UI_PAGE_CCTV_ACTION_NEXT;
    }
    else if (target == ctx->quality_btn)
    {
        action = UI_PAGE_CCTV_ACTION_QUALITY;
    }
    else if (target == ctx->mute_btn)
    {
        action = UI_PAGE_CCTV_ACTION_TOGGLE_MUTE;
    }
    else
    {
        return;
    }

    ui_page_cctv_action_event_t data = {
        .action        = action,
        .camera_index  = ctx->active_index,
        .camera_count  = ctx->camera_count,
        .camera_id     = ctx->active_camera_id,
        .stream_url    = ctx->active_stream_url,
        .quality_label = ctx->quality_label_text,
        .muted         = ctx->muted,
    };

    lv_event_send(ctx->page, UI_PAGE_CCTV_EVENT_ACTION, &data);
}

static void action_button_cb(lv_event_t* event)
{
    if (event == NULL)
    {
        return;
    }

    ui_page_cctv_ctx_t* ctx = (ui_page_cctv_ctx_t*)lv_event_get_user_data(event);
    if (ctx == NULL || ctx->page == NULL)
    {
        return;
    }

    lv_obj_t* target = lv_event_get_target(event);
    if (lv_obj_has_state(target, LV_STATE_DISABLED))
    {
        return;
    }

    ui_page_cctv_action_t action;

    if (target == ctx->open_gate_button)
    {
        action = UI_PAGE_CCTV_ACTION_OPEN_GATE;
    }
    else if (target == ctx->talk_button)
    {
        action = UI_PAGE_CCTV_ACTION_TALK;
    }
    else if (target == ctx->snapshot_button)
    {
        action = UI_PAGE_CCTV_ACTION_SNAPSHOT;
    }
    else if (target == ctx->timeline_button)
    {
        action = UI_PAGE_CCTV_ACTION_TIMELINE;
    }
    else
    {
        return;
    }

    ui_page_cctv_action_event_t data = {
        .action        = action,
        .camera_index  = ctx->active_index,
        .camera_count  = ctx->camera_count,
        .camera_id     = ctx->active_camera_id,
        .stream_url    = ctx->active_stream_url,
        .quality_label = ctx->quality_label_text,
        .muted         = ctx->muted,
    };

    lv_event_send(ctx->page, UI_PAGE_CCTV_EVENT_ACTION, &data);
}

static void hide_toggle(ui_room_card_t* card)
{
    if (card == NULL)
    {
        return;
    }

    lv_obj_t* toggle = ui_room_card_get_toggle(card);
    if (toggle != NULL)
    {
        lv_obj_add_flag(toggle, LV_OBJ_FLAG_HIDDEN);
    }
}

static lv_obj_t* create_toolbar(ui_page_cctv_ctx_t* ctx)
{
    lv_obj_t* toolbar = lv_obj_create(ctx->content);
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

    lv_obj_add_event_cb(prev_btn, toolbar_button_cb, LV_EVENT_CLICKED, ctx);

    lv_obj_t* camera_label = lv_label_create(toolbar);
    lv_label_set_text(camera_label, "No cameras");
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

    lv_obj_add_event_cb(next_btn, toolbar_button_cb, LV_EVENT_CLICKED, ctx);

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

    lv_obj_add_event_cb(quality_btn, toolbar_button_cb, LV_EVENT_CLICKED, ctx);

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

    lv_obj_add_event_cb(mute_btn, toolbar_button_cb, LV_EVENT_CLICKED, ctx);

    ctx->toolbar            = toolbar;
    ctx->prev_btn           = prev_btn;
    ctx->next_btn           = next_btn;
    ctx->quality_btn        = quality_btn;
    ctx->mute_btn           = mute_btn;
    ctx->camera_index_label = camera_label;
    ctx->quality_label      = quality_label;
    ctx->mute_label         = mute_label;

    return toolbar;
}

static void create_camera_section(ui_page_cctv_ctx_t* ctx)
{
    ui_room_card_config_t config = {
        .room_id   = "cctv_primary",
        .title     = "Camera",
        .icon_text = LV_SYMBOL_EYE_OPEN,
    };

    ctx->camera_card = ui_room_card_create(ctx->content, &config);
    if (ctx->camera_card == NULL)
    {
        return;
    }

    lv_obj_t* card_obj = ui_room_card_get_obj(ctx->camera_card);
    lv_obj_set_style_pad_gap(card_obj, 24, LV_PART_MAIN);

    lv_obj_t* header = lv_obj_get_child(card_obj, 0);
    if (header != NULL)
    {
        ctx->camera_title_label = lv_obj_get_child(header, 1);
    }
    ctx->camera_specs_label = lv_obj_get_child(card_obj, -1);

    hide_toggle(ctx->camera_card);

    ctx->video_container = lv_obj_create(card_obj);
    lv_obj_remove_style_all(ctx->video_container);
    lv_obj_set_width(ctx->video_container, LV_PCT(100));
    lv_obj_set_height(ctx->video_container, 280);
    lv_obj_set_style_bg_color(ctx->video_container, ui_theme_color_outline(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(ctx->video_container, LV_OPA_30, LV_PART_MAIN);
    lv_obj_set_style_radius(ctx->video_container, 18, LV_PART_MAIN);
    lv_obj_set_style_border_width(ctx->video_container, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(ctx->video_container, 24, LV_PART_MAIN);

    ctx->stream_label = lv_label_create(ctx->video_container);
    lv_label_set_text(ctx->stream_label, "Live feed (stub)");
    lv_obj_set_style_text_font(ctx->stream_label, &lv_font_montserrat_22, LV_PART_MAIN);
    lv_obj_set_style_text_color(ctx->stream_label, ui_theme_color_on_surface(), LV_PART_MAIN);
    lv_obj_center(ctx->stream_label);
}

static void create_events_section(ui_page_cctv_ctx_t* ctx)
{
    ctx->events_row = lv_obj_create(ctx->content);
    lv_obj_remove_style_all(ctx->events_row);
    lv_obj_set_width(ctx->events_row, LV_PCT(100));
    lv_obj_set_style_bg_opa(ctx->events_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(ctx->events_row, 24, LV_PART_MAIN);
    lv_obj_set_flex_flow(ctx->events_row, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(
        ctx->events_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
}

static void create_actions_section(ui_page_cctv_ctx_t* ctx)
{
    ctx->actions_row = lv_obj_create(ctx->content);
    lv_obj_remove_style_all(ctx->actions_row);
    lv_obj_set_width(ctx->actions_row, LV_PCT(100));
    lv_obj_set_style_bg_opa(ctx->actions_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(ctx->actions_row, 24, LV_PART_MAIN);
    lv_obj_set_flex_flow(ctx->actions_row, LV_FLEX_FLOW_ROW_WRAP);
    lv_obj_set_flex_align(
        ctx->actions_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    struct
    {
        const char*           label;
        ui_page_cctv_action_t action;
        lv_obj_t**            slot;
    } buttons[] = {
        {"Open Gate", UI_PAGE_CCTV_ACTION_OPEN_GATE, &ctx->open_gate_button},
        {"Talk", UI_PAGE_CCTV_ACTION_TALK, &ctx->talk_button},
        {"Snapshot", UI_PAGE_CCTV_ACTION_SNAPSHOT, &ctx->snapshot_button},
        {"Timeline ▾", UI_PAGE_CCTV_ACTION_TIMELINE, &ctx->timeline_button},
    };

    for (size_t i = 0; i < (sizeof(buttons) / sizeof(buttons[0])); i++)
    {
        lv_obj_t* button = lv_btn_create(ctx->actions_row);
        lv_obj_remove_style_all(button);
        lv_obj_set_style_bg_color(button, ui_theme_color_surface(), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(button, LV_OPA_80, LV_PART_MAIN);
        lv_obj_set_style_radius(button, 26, LV_PART_MAIN);
        lv_obj_set_style_border_width(button, 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(button, ui_theme_color_outline(), LV_PART_MAIN);
        lv_obj_set_style_pad_hor(button, 28, LV_PART_MAIN);
        lv_obj_set_style_pad_ver(button, 16, LV_PART_MAIN);
        lv_obj_set_style_shadow_width(button, 0, LV_PART_MAIN);
        lv_obj_set_style_text_color(button, ui_theme_color_on_surface(), LV_PART_MAIN);

        lv_obj_t* label = lv_label_create(button);
        lv_label_set_text(label, buttons[i].label);
        lv_obj_set_style_text_font(label, &lv_font_montserrat_18, LV_PART_MAIN);

        lv_obj_add_event_cb(button, action_button_cb, LV_EVENT_CLICKED, ctx);
        *buttons[i].slot = button;
    }
}

static lv_obj_t* create_content(ui_page_cctv_ctx_t* ctx)
{
    lv_obj_t* content = lv_obj_create(ctx->page);
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
    lv_label_set_text(title, "Frigate Security");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, ui_theme_color_on_surface(), LV_PART_MAIN);

    ctx->content = content;

    create_toolbar(ctx);
    create_camera_section(ctx);
    create_events_section(ctx);
    create_actions_section(ctx);

    return content;
}

static void ui_page_cctv_delete_cb(lv_event_t* event)
{
    if (event == NULL)
    {
        return;
    }

    ui_page_cctv_ctx_t* ctx = (ui_page_cctv_ctx_t*)lv_event_get_user_data(event);
    if (ctx == NULL)
    {
        return;
    }

    if (ctx->wallpaper != NULL)
    {
        ui_wallpaper_detach(ctx->wallpaper);
        ctx->wallpaper = NULL;
    }

    replace_string(&ctx->active_camera_id, NULL);
    replace_string(&ctx->active_stream_url, NULL);
    replace_string(&ctx->quality_label_text, NULL);
    clear_event_slots(ctx);

    if (s_ctx == ctx)
    {
        s_ctx = NULL;
    }

    lv_free(ctx);
}

lv_obj_t* ui_page_cctv_create(lv_obj_t* parent)
{
    if (parent == NULL)
    {
        return NULL;
    }

    ui_page_cctv_ctx_t* ctx = (ui_page_cctv_ctx_t*)lv_malloc(sizeof(ui_page_cctv_ctx_t));
    if (ctx == NULL)
    {
        return NULL;
    }
    lv_memset_00(ctx, sizeof(ui_page_cctv_ctx_t));

    ctx->page = lv_obj_create(parent);
    if (ctx->page == NULL)
    {
        lv_free(ctx);
        return NULL;
    }

    lv_obj_remove_style_all(ctx->page);
    lv_obj_set_size(ctx->page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(ctx->page, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_scroll_dir(ctx->page, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(ctx->page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(ctx->page, LV_OBJ_FLAG_CLICKABLE);

    ctx->wallpaper = ui_wallpaper_attach(ctx->page);

    create_content(ctx);
    update_toolbar(ctx);
    update_camera_card(ctx, NULL);
    update_events(ctx, NULL, 0);

    lv_obj_add_event_cb(ctx->page, ui_page_cctv_delete_cb, LV_EVENT_DELETE, ctx);

    s_ctx = ctx;

    return ctx->page;
}

lv_obj_t* ui_page_cctv_get_obj(void)
{
    return s_ctx != NULL ? s_ctx->page : NULL;
}

void ui_page_cctv_set_state(const ui_page_cctv_state_t* state)
{
    if (s_ctx == NULL)
    {
        return;
    }

    const ui_page_cctv_camera_t* camera = NULL;

    if (state != NULL && state->cameras != NULL && state->camera_count > 0)
    {
        s_ctx->camera_count = state->camera_count;
        if (state->active_index < state->camera_count)
        {
            s_ctx->active_index = state->active_index;
        }
        else
        {
            s_ctx->active_index = 0;
        }
        camera = &state->cameras[s_ctx->active_index];
    }
    else
    {
        s_ctx->camera_count = 0;
        s_ctx->active_index = 0;
    }

    if (state != NULL)
    {
        s_ctx->muted = state->muted;
    }
    else
    {
        s_ctx->muted = false;
    }

    if (camera != NULL)
    {
        replace_string(&s_ctx->active_camera_id, camera->camera_id);
        replace_string(&s_ctx->active_stream_url, camera->stream_url);
        s_ctx->audio_supported = camera->audio_supported;
    }
    else
    {
        replace_string(&s_ctx->active_camera_id, NULL);
        replace_string(&s_ctx->active_stream_url, NULL);
        s_ctx->audio_supported = false;
    }

    if (state != NULL)
    {
        replace_string(&s_ctx->quality_label_text, state->quality_label);
    }
    else
    {
        replace_string(&s_ctx->quality_label_text, NULL);
    }

    update_toolbar(s_ctx);
    update_camera_card(s_ctx, camera);
    update_events(
        s_ctx, state != NULL ? state->events : NULL, state != NULL ? state->event_count : 0);
}

void ui_page_cctv_set_events(const ui_page_cctv_event_t* events, size_t event_count)
{
    if (s_ctx == NULL)
    {
        return;
    }

    update_events(s_ctx, events, event_count);
}
