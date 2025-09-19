/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ui_page_media.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "../ui_theme.h"
#include "../ui_wallpaper.h"
#include "../widgets/ui_room_card.h"

typedef struct
{
    lv_obj_t* button;
    lv_obj_t* label;
    char*     scene_id;
} ui_page_media_scene_slot_t;

typedef struct
{
    lv_obj_t*                  page;
    lv_obj_t*                  content;
    ui_wallpaper_t*            wallpaper;
    ui_room_card_t*            now_playing_card;
    ui_room_card_t*            scenes_card;
    lv_obj_t*                  track_title;
    lv_obj_t*                  track_artist;
    lv_obj_t*                  track_source;
    lv_obj_t*                  album_art;
    lv_obj_t*                  previous_btn;
    lv_obj_t*                  play_pause_btn;
    lv_obj_t*                  play_pause_label;
    lv_obj_t*                  next_btn;
    lv_obj_t*                  volume_slider;
    ui_page_media_scene_slot_t scenes[UI_PAGE_MEDIA_MAX_SCENES];
    size_t                     scene_count;
    bool                       slider_updating;
    bool                       playing;
} ui_page_media_ctx_t;

static ui_page_media_ctx_t* s_ctx = NULL;

static void ui_page_media_clear_scenes(ui_page_media_ctx_t* ctx)
{
    if (ctx == NULL)
    {
        return;
    }

    for (size_t i = 0; i < UI_PAGE_MEDIA_MAX_SCENES; i++)
    {
        if (ctx->scenes[i].scene_id != NULL)
        {
            lv_free(ctx->scenes[i].scene_id);
            ctx->scenes[i].scene_id = NULL;
        }
        if (ctx->scenes[i].button != NULL)
        {
            lv_obj_add_flag(ctx->scenes[i].button, LV_OBJ_FLAG_HIDDEN);
        }
    }
    ctx->scene_count = 0;
}

static char* ui_page_media_strdup(const char* value)
{
    if (value == NULL)
    {
        return NULL;
    }

    size_t length = strlen(value);
    char*  copy   = (char*)lv_malloc(length + 1);
    if (copy == NULL)
    {
        return NULL;
    }

    memcpy(copy, value, length);
    copy[length] = '\0';
    return copy;
}

static void ui_page_media_delete_cb(lv_event_t* event)
{
    if (event == NULL)
    {
        return;
    }

    ui_page_media_ctx_t* ctx = (ui_page_media_ctx_t*)lv_event_get_user_data(event);
    if (ctx == NULL)
    {
        return;
    }

    if (ctx->wallpaper != NULL)
    {
        ui_wallpaper_detach(ctx->wallpaper);
        ctx->wallpaper = NULL;
    }

    ui_page_media_clear_scenes(ctx);

    if (s_ctx == ctx)
    {
        s_ctx = NULL;
    }

    lv_free(ctx);
}

static lv_obj_t* ui_page_create_content(lv_obj_t* page)
{
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

static void ui_page_media_emit_event(ui_page_media_ctx_t*   ctx,
                                     ui_page_media_signal_t signal,
                                     uint8_t                volume,
                                     const char*            scene_id)
{
    if (ctx == NULL || ctx->page == NULL)
    {
        return;
    }

    ui_page_media_event_t event_data = {
        .signal   = signal,
        .scene_id = scene_id,
        .volume   = volume,
    };

    lv_obj_send_event(ctx->page, UI_PAGE_MEDIA_EVENT_COMMAND, &event_data);
}

static void ui_page_media_transport_cb(lv_event_t* event)
{
    if (event == NULL)
    {
        return;
    }

    ui_page_media_ctx_t* ctx = (ui_page_media_ctx_t*)lv_event_get_user_data(event);
    if (ctx == NULL)
    {
        return;
    }

    lv_obj_t* target = lv_event_get_target(event);
    if (target == NULL)
    {
        return;
    }

    if (target == ctx->previous_btn)
    {
        ui_page_media_emit_event(ctx, UI_PAGE_MEDIA_SIGNAL_PREVIOUS, 0, NULL);
    }
    else if (target == ctx->play_pause_btn)
    {
        ui_page_media_emit_event(ctx, UI_PAGE_MEDIA_SIGNAL_PLAY_PAUSE, 0, NULL);
    }
    else if (target == ctx->next_btn)
    {
        ui_page_media_emit_event(ctx, UI_PAGE_MEDIA_SIGNAL_NEXT, 0, NULL);
    }
}

static void ui_page_media_volume_cb(lv_event_t* event)
{
    if (event == NULL)
    {
        return;
    }

    ui_page_media_ctx_t* ctx = (ui_page_media_ctx_t*)lv_event_get_user_data(event);
    if (ctx == NULL || ctx->volume_slider == NULL)
    {
        return;
    }

    if (ctx->slider_updating)
    {
        return;
    }

    int32_t value = lv_slider_get_value(ctx->volume_slider);
    if (value < 0)
    {
        value = 0;
    }
    else if (value > 100)
    {
        value = 100;
    }

    ui_page_media_emit_event(ctx, UI_PAGE_MEDIA_SIGNAL_VOLUME, (uint8_t)value, NULL);
}

static void ui_page_media_scene_cb(lv_event_t* event)
{
    if (event == NULL)
    {
        return;
    }

    ui_page_media_ctx_t* ctx = (ui_page_media_ctx_t*)lv_event_get_user_data(event);
    if (ctx == NULL)
    {
        return;
    }

    lv_obj_t* target = lv_event_get_target(event);
    if (target == NULL)
    {
        return;
    }

    for (size_t i = 0; i < ctx->scene_count; i++)
    {
        if (ctx->scenes[i].button == target)
        {
            ui_page_media_emit_event(
                ctx, UI_PAGE_MEDIA_SIGNAL_TRIGGER_SCENE, 0, ctx->scenes[i].scene_id);
            break;
        }
    }
}

static void ui_page_media_configure_scene_button(lv_obj_t* button)
{
    if (button == NULL)
    {
        return;
    }

    lv_obj_remove_style_all(button);
    lv_obj_set_style_radius(button, 14, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, ui_theme_color_surface(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(button, LV_OPA_60, LV_PART_MAIN);
    lv_obj_set_style_border_width(button, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(button, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(button, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(button, 16, LV_PART_MAIN);
    lv_obj_set_flex_grow(button, 1);
    lv_obj_clear_flag(button, LV_OBJ_FLAG_SCROLLABLE);
}

static void ui_page_media_configure_transport_button(lv_obj_t* button)
{
    if (button == NULL)
    {
        return;
    }

    lv_obj_remove_style_all(button);
    lv_obj_set_flex_grow(button, 1);
    lv_obj_set_height(button, 56);
    lv_obj_set_style_radius(button, 18, LV_PART_MAIN);
    lv_obj_set_style_bg_color(button, ui_theme_color_accent(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(button, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_border_width(button, 0, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(button, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_hor(button, 24, LV_PART_MAIN);
    lv_obj_set_style_pad_ver(button, 8, LV_PART_MAIN);
    lv_obj_clear_flag(button, LV_OBJ_FLAG_SCROLLABLE);
}

lv_obj_t* ui_page_media_create(lv_obj_t* parent)
{
    if (parent == NULL)
    {
        return NULL;
    }

    ui_page_media_ctx_t* ctx = (ui_page_media_ctx_t*)lv_malloc(sizeof(ui_page_media_ctx_t));
    if (ctx == NULL)
    {
        return NULL;
    }

    memset(ctx, 0, sizeof(ui_page_media_ctx_t));

    lv_obj_t* page = lv_obj_create(parent);
    if (page == NULL)
    {
        lv_free(ctx);
        return NULL;
    }

    ctx->page = page;

    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_scroll_dir(page, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(page, LV_OBJ_FLAG_CLICKABLE);

    ctx->wallpaper = ui_wallpaper_attach(page);
    lv_obj_add_event_cb(page, ui_page_media_delete_cb, LV_EVENT_DELETE, ctx);

    ctx->content = ui_page_create_content(page);

    ui_room_card_config_t now_playing_config = {
        .room_id   = "media.now_playing",
        .title     = "Now Playing",
        .icon_text = LV_SYMBOL_AUDIO,
    };

    ctx->now_playing_card = ui_room_card_create(ctx->content, &now_playing_config);
    if (ctx->now_playing_card != NULL)
    {
        lv_obj_t* card_obj = ui_room_card_get_obj(ctx->now_playing_card);
        if (card_obj != NULL)
        {
            lv_obj_t* toggle = ui_room_card_get_toggle(ctx->now_playing_card);
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

            ctx->album_art = lv_obj_create(info_row);
            lv_obj_remove_style_all(ctx->album_art);
            lv_obj_set_size(ctx->album_art, 240, 240);
            lv_obj_set_style_bg_color(ctx->album_art, ui_theme_color_surface(), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(ctx->album_art, LV_OPA_70, LV_PART_MAIN);
            lv_obj_set_style_radius(ctx->album_art, 16, LV_PART_MAIN);
            lv_obj_set_style_border_width(ctx->album_art, 0, LV_PART_MAIN);
            lv_obj_clear_flag(ctx->album_art, LV_OBJ_FLAG_SCROLLABLE);

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

            ctx->track_title = lv_label_create(track_info);
            lv_obj_set_style_text_font(ctx->track_title, &lv_font_montserrat_26, LV_PART_MAIN);
            lv_obj_set_style_text_color(
                ctx->track_title, ui_theme_color_on_surface(), LV_PART_MAIN);
            lv_label_set_long_mode(ctx->track_title, LV_LABEL_LONG_WRAP);
            lv_obj_set_width(ctx->track_title, LV_PCT(100));

            ctx->track_artist = lv_label_create(track_info);
            lv_obj_set_style_text_font(ctx->track_artist, &lv_font_montserrat_20, LV_PART_MAIN);
            lv_obj_set_style_text_color(ctx->track_artist, ui_theme_color_muted(), LV_PART_MAIN);
            lv_label_set_long_mode(ctx->track_artist, LV_LABEL_LONG_WRAP);
            lv_obj_set_width(ctx->track_artist, LV_PCT(100));

            ctx->track_source = lv_label_create(track_info);
            lv_obj_set_style_text_font(ctx->track_source, &lv_font_montserrat_18, LV_PART_MAIN);
            lv_obj_set_style_text_color(ctx->track_source, ui_theme_color_muted(), LV_PART_MAIN);
            lv_label_set_long_mode(ctx->track_source, LV_LABEL_LONG_WRAP);
            lv_obj_set_width(ctx->track_source, LV_PCT(100));

            lv_obj_t* transport_row = lv_obj_create(card_obj);
            lv_obj_remove_style_all(transport_row);
            lv_obj_set_width(transport_row, LV_PCT(100));
            lv_obj_set_style_bg_opa(transport_row, LV_OPA_TRANSP, LV_PART_MAIN);
            lv_obj_set_style_pad_gap(transport_row, 16, LV_PART_MAIN);
            lv_obj_set_flex_flow(transport_row, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(
                transport_row, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_clear_flag(transport_row, LV_OBJ_FLAG_SCROLLABLE);

            ctx->previous_btn = lv_btn_create(transport_row);
            ui_page_media_configure_transport_button(ctx->previous_btn);
            lv_obj_add_event_cb(
                ctx->previous_btn, ui_page_media_transport_cb, LV_EVENT_CLICKED, ctx);
            lv_obj_t* prev_label = lv_label_create(ctx->previous_btn);
            lv_label_set_text(prev_label, "Prev");
            lv_obj_center(prev_label);
            lv_obj_set_style_text_font(prev_label, &lv_font_montserrat_18, LV_PART_MAIN);
            lv_obj_set_style_text_color(prev_label, lv_color_white(), LV_PART_MAIN);

            ctx->play_pause_btn = lv_btn_create(transport_row);
            ui_page_media_configure_transport_button(ctx->play_pause_btn);
            lv_obj_add_event_cb(
                ctx->play_pause_btn, ui_page_media_transport_cb, LV_EVENT_CLICKED, ctx);
            ctx->play_pause_label = lv_label_create(ctx->play_pause_btn);
            lv_label_set_text(ctx->play_pause_label, "Play");
            lv_obj_center(ctx->play_pause_label);
            lv_obj_set_style_text_font(ctx->play_pause_label, &lv_font_montserrat_18, LV_PART_MAIN);
            lv_obj_set_style_text_color(ctx->play_pause_label, lv_color_white(), LV_PART_MAIN);

            ctx->next_btn = lv_btn_create(transport_row);
            ui_page_media_configure_transport_button(ctx->next_btn);
            lv_obj_add_event_cb(ctx->next_btn, ui_page_media_transport_cb, LV_EVENT_CLICKED, ctx);
            lv_obj_t* next_label = lv_label_create(ctx->next_btn);
            lv_label_set_text(next_label, "Next");
            lv_obj_center(next_label);
            lv_obj_set_style_text_font(next_label, &lv_font_montserrat_18, LV_PART_MAIN);
            lv_obj_set_style_text_color(next_label, lv_color_white(), LV_PART_MAIN);

            ctx->volume_slider = lv_slider_create(transport_row);
            lv_obj_set_flex_grow(ctx->volume_slider, 2);
            lv_obj_set_height(ctx->volume_slider, 36);
            lv_slider_set_range(ctx->volume_slider, 0, 100);
            lv_obj_clear_flag(ctx->volume_slider, LV_OBJ_FLAG_SCROLLABLE);
            lv_obj_set_style_bg_color(ctx->volume_slider, ui_theme_color_muted(), LV_PART_MAIN);
            lv_obj_set_style_bg_opa(ctx->volume_slider, LV_OPA_30, LV_PART_MAIN);
            lv_obj_set_style_radius(ctx->volume_slider, 18, LV_PART_MAIN);
            lv_obj_set_style_border_width(ctx->volume_slider, 0, LV_PART_MAIN);
            lv_obj_set_style_bg_color(
                ctx->volume_slider, ui_theme_color_accent(), LV_PART_INDICATOR);
            lv_obj_set_style_bg_opa(ctx->volume_slider, LV_OPA_COVER, LV_PART_INDICATOR);
            lv_obj_set_style_radius(ctx->volume_slider, 18, LV_PART_INDICATOR);
            lv_obj_add_event_cb(
                ctx->volume_slider, ui_page_media_volume_cb, LV_EVENT_VALUE_CHANGED, ctx);
        }
    }

    ui_room_card_config_t scenes_config = {
        .room_id   = "media.quick_scenes",
        .title     = "Quick Scenes",
        .icon_text = LV_SYMBOL_LIST,
    };

    ctx->scenes_card = ui_room_card_create(ctx->content, &scenes_config);
    if (ctx->scenes_card != NULL)
    {
        lv_obj_t* card_obj = ui_room_card_get_obj(ctx->scenes_card);
        if (card_obj != NULL)
        {
            lv_obj_t* toggle = ui_room_card_get_toggle(ctx->scenes_card);
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

            for (size_t i = 0; i < UI_PAGE_MEDIA_MAX_SCENES; i++)
            {
                ctx->scenes[i].button = lv_btn_create(scene_grid);
                ui_page_media_configure_scene_button(ctx->scenes[i].button);
                ctx->scenes[i].label = lv_label_create(ctx->scenes[i].button);
                lv_obj_center(ctx->scenes[i].label);
                lv_obj_set_style_text_font(
                    ctx->scenes[i].label, &lv_font_montserrat_18, LV_PART_MAIN);
                lv_obj_set_style_text_color(
                    ctx->scenes[i].label, ui_theme_color_on_surface(), LV_PART_MAIN);
                lv_obj_add_event_cb(
                    ctx->scenes[i].button, ui_page_media_scene_cb, LV_EVENT_CLICKED, ctx);
            }
        }
    }

    s_ctx = ctx;

    static const ui_page_media_scene_t k_default_scenes[UI_PAGE_MEDIA_MAX_SCENES] = {
        {.scene_id = "scene.morning_mix", .title = "Morning"},
        {.scene_id = "scene.movie_time", .title = "Movie"},
        {.scene_id = "scene.night_relax", .title = "Night"},
        {.scene_id = "scene.party_mode", .title = "Party"},
    };

    ui_page_media_set_quick_scenes(k_default_scenes, UI_PAGE_MEDIA_MAX_SCENES);

    ui_page_media_now_playing_t placeholder = {
        .media_id = "media.placeholder",
        .title    = "Placeholder Track",
        .artist   = "Artist Name",
        .source   = "Source · Placeholder",
        .playing  = true,
        .volume   = 40,
    };

    ui_page_media_set_now_playing(&placeholder);

    return page;
}

lv_obj_t* ui_page_media_get_obj(void)
{
    return (s_ctx != NULL) ? s_ctx->page : NULL;
}

lv_obj_t* ui_page_media_get_previous_button(void)
{
    return (s_ctx != NULL) ? s_ctx->previous_btn : NULL;
}

lv_obj_t* ui_page_media_get_play_pause_button(void)
{
    return (s_ctx != NULL) ? s_ctx->play_pause_btn : NULL;
}

lv_obj_t* ui_page_media_get_next_button(void)
{
    return (s_ctx != NULL) ? s_ctx->next_btn : NULL;
}

lv_obj_t* ui_page_media_get_volume_slider(void)
{
    return (s_ctx != NULL) ? s_ctx->volume_slider : NULL;
}

lv_obj_t* ui_page_media_get_scene_button(size_t index)
{
    if (s_ctx == NULL || index >= UI_PAGE_MEDIA_MAX_SCENES)
    {
        return NULL;
    }
    return s_ctx->scenes[index].button;
}

size_t ui_page_media_get_scene_count(void)
{
    return (s_ctx != NULL) ? s_ctx->scene_count : 0U;
}

void ui_page_media_set_now_playing(const ui_page_media_now_playing_t* now_playing)
{
    if (s_ctx == NULL)
    {
        return;
    }

    const char* title  = (now_playing != NULL && now_playing->title != NULL) ? now_playing->title
                                                                             : "Nothing Playing";
    const char* artist = (now_playing != NULL && now_playing->artist != NULL) ? now_playing->artist
                                                                              : "Artist Unknown";
    const char* source = (now_playing != NULL && now_playing->source != NULL) ? now_playing->source
                                                                              : "Source · Unknown";

    if (s_ctx->track_title != NULL)
    {
        lv_label_set_text(s_ctx->track_title, title);
    }
    if (s_ctx->track_artist != NULL)
    {
        lv_label_set_text(s_ctx->track_artist, artist);
    }
    if (s_ctx->track_source != NULL)
    {
        lv_label_set_text(s_ctx->track_source, source);
    }

    bool playing   = (now_playing != NULL) ? now_playing->playing : false;
    s_ctx->playing = playing;
    if (s_ctx->play_pause_label != NULL)
    {
        lv_label_set_text(s_ctx->play_pause_label, playing ? "Pause" : "Play");
    }

    if (s_ctx->volume_slider != NULL && now_playing != NULL)
    {
        uint8_t volume = now_playing->volume;
        if (volume > 100)
        {
            volume = 100;
        }

        s_ctx->slider_updating = true;
        lv_slider_set_value(s_ctx->volume_slider, volume, LV_ANIM_OFF);
        s_ctx->slider_updating = false;
    }
}

void ui_page_media_set_quick_scenes(const ui_page_media_scene_t* scenes, size_t scene_count)
{
    if (s_ctx == NULL)
    {
        return;
    }

    ui_page_media_clear_scenes(s_ctx);

    if (scenes == NULL || scene_count == 0)
    {
        return;
    }

    size_t count =
        (scene_count > UI_PAGE_MEDIA_MAX_SCENES) ? UI_PAGE_MEDIA_MAX_SCENES : scene_count;
    for (size_t i = 0; i < count; i++)
    {
        s_ctx->scenes[i].scene_id = ui_page_media_strdup(scenes[i].scene_id);
        if (s_ctx->scenes[i].button != NULL)
        {
            lv_obj_clear_flag(s_ctx->scenes[i].button, LV_OBJ_FLAG_HIDDEN);
        }
        if (s_ctx->scenes[i].label != NULL)
        {
            const char* title = (scenes[i].title != NULL) ? scenes[i].title : "Scene";
            lv_label_set_text(s_ctx->scenes[i].label, title);
        }
    }

    s_ctx->scene_count = count;
}
