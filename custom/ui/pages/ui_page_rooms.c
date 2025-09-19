/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ui_page_rooms.h"

#include <stdbool.h>
#include <string.h>

#include "../../integration/rooms_provider.h"
#include "../ui_theme.h"
#include "../ui_wallpaper.h"
#include "../widgets/ui_room_card.h"

#define ROOM_CARD_COUNT 3

typedef struct
{
    const char* room_id;
    const char* title;
    const char* icon_text;
} room_card_descriptor_t;

static const room_card_descriptor_t k_room_cards[ROOM_CARD_COUNT] = {
    {"bakery", "Bakery", LV_SYMBOL_SHUFFLE},
    {"bedroom", "Bedroom", LV_SYMBOL_BELL},
    {"living", "Living Room", LV_SYMBOL_HOME},
};

typedef struct
{
    lv_obj_t*       page;
    lv_obj_t*       content;
    lv_obj_t*       toolbar;
    lv_obj_t*       grid;
    ui_room_card_t* cards[ROOM_CARD_COUNT];
    struct
    {
        const char* room_id;
        const char* entity_id;
    } slots[ROOM_CARD_COUNT];
    ui_wallpaper_t* wallpaper;
    bool            intro_played;
} ui_page_rooms_ctx_t;

static ui_page_rooms_ctx_t* s_ctx = NULL;

static void ui_page_rooms_delete_cb(lv_event_t* event)
{
    if (event == NULL)
    {
        return;
    }

    ui_page_rooms_ctx_t* ctx = (ui_page_rooms_ctx_t*)lv_event_get_user_data(event);
    if (ctx == NULL)
    {
        return;
    }

    if (ctx->wallpaper != NULL)
    {
        ui_wallpaper_detach(ctx->wallpaper);
        ctx->wallpaper = NULL;
    }

    if (s_ctx == ctx)
    {
        s_ctx = NULL;
    }

    lv_free(ctx);
}

static void grid_style_init(lv_obj_t* grid)
{
    lv_obj_remove_style_all(grid);
    lv_obj_set_size(grid, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(grid, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_all(grid, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(grid, 16, LV_PART_MAIN);
}

static void toolbar_style_init(lv_obj_t* toolbar)
{
    lv_obj_remove_style_all(toolbar);
    lv_obj_set_size(toolbar, LV_PCT(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(toolbar, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(toolbar, 24, LV_PART_MAIN);
}

static void toolbar_translate_exec_cb(void* var, int32_t v)
{
    lv_obj_t* obj = (lv_obj_t*)var;
    if (obj == NULL)
    {
        return;
    }
    lv_obj_set_style_translate_y(obj, (lv_coord_t)v, LV_PART_MAIN);
}

static void toolbar_opa_exec_cb(void* var, int32_t v)
{
    lv_obj_t* obj = (lv_obj_t*)var;
    if (obj == NULL)
    {
        return;
    }
    lv_obj_set_style_opa(obj, (lv_opa_t)v, LV_PART_MAIN);
}

static void play_toolbar_intro(lv_obj_t* toolbar)
{
    if (toolbar == NULL)
    {
        return;
    }

    lv_obj_set_style_translate_y(toolbar, -28, LV_PART_MAIN);
    lv_obj_set_style_opa(toolbar, LV_OPA_TRANSP, LV_PART_MAIN);

    lv_anim_t slide;
    lv_anim_init(&slide);
    lv_anim_set_var(&slide, toolbar);
    lv_anim_set_time(&slide, 180);
    lv_anim_set_values(&slide, -28, 0);
    lv_anim_set_exec_cb(&slide, toolbar_translate_exec_cb);
    lv_anim_start(&slide);

    lv_anim_t fade;
    lv_anim_init(&fade);
    lv_anim_set_var(&fade, toolbar);
    lv_anim_set_time(&fade, 180);
    lv_anim_set_values(&fade, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_exec_cb(&fade, toolbar_opa_exec_cb);
    lv_anim_start(&fade);
}

static void toggle_clicked_cb(lv_event_t* event)
{
    if (event == NULL || s_ctx == NULL)
    {
        return;
    }

    ui_room_card_t* card = (ui_room_card_t*)lv_event_get_user_data(event);
    if (card == NULL)
    {
        return;
    }

    ui_room_card_play_toggle_feedback(card);

    ui_page_rooms_event_data_t data = {
        .signal    = UI_PAGE_ROOMS_SIGNAL_TOGGLE,
        .room_id   = ui_room_card_get_room_id(card),
        .entity_id = ui_room_card_get_entity_id(card),
    };

    lv_obj_send_event(s_ctx->page, UI_PAGE_ROOMS_EVENT_TOGGLE, &data);
}

static void card_long_press_cb(lv_event_t* event)
{
    if (event == NULL || s_ctx == NULL)
    {
        return;
    }
    ui_room_card_t* card = (ui_room_card_t*)lv_event_get_user_data(event);
    if (card == NULL)
    {
        return;
    }

    ui_page_rooms_event_data_t data = {
        .signal    = UI_PAGE_ROOMS_SIGNAL_OPEN_SHEET,
        .room_id   = ui_room_card_get_room_id(card),
        .entity_id = ui_room_card_get_entity_id(card),
    };

    lv_obj_send_event(s_ctx->page, UI_PAGE_ROOMS_EVENT_OPEN_SHEET, &data);
}

static void create_cards(ui_page_rooms_ctx_t* ctx)
{
    static const lv_coord_t cols[] = {
        LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_FR(1), LV_GRID_TEMPLATE_LAST};
    static const lv_coord_t rows[] = {LV_GRID_CONTENT, LV_GRID_TEMPLATE_LAST};

    ctx->grid = lv_obj_create(ctx->content);
    grid_style_init(ctx->grid);
    lv_obj_set_grid_dsc_array(ctx->grid, cols, rows);

    for (size_t i = 0; i < ROOM_CARD_COUNT; i++)
    {
        const room_card_descriptor_t* desc   = &k_room_cards[i];
        ui_room_card_config_t         config = {
                    .room_id   = desc->room_id,
                    .title     = desc->title,
                    .icon_text = desc->icon_text,
        };

        ctx->cards[i]      = ui_room_card_create(ctx->grid, &config);
        lv_obj_t* card_obj = ui_room_card_get_obj(ctx->cards[i]);
        lv_obj_set_grid_cell(card_obj, LV_GRID_ALIGN_STRETCH, i, 1, LV_GRID_ALIGN_START, 0, 1);
        lv_obj_clear_flag(card_obj, LV_OBJ_FLAG_SCROLLABLE);

        lv_obj_add_event_cb(ui_room_card_get_toggle(ctx->cards[i]),
                            toggle_clicked_cb,
                            LV_EVENT_CLICKED,
                            ctx->cards[i]);
        lv_obj_add_event_cb(card_obj, card_long_press_cb, LV_EVENT_LONG_PRESSED, ctx->cards[i]);

        ctx->slots[i].room_id   = desc->room_id;
        ctx->slots[i].entity_id = NULL;
    }
}

static const room_t* find_room(const rooms_state_t* state, const char* room_id)
{
    if (state == NULL || state->rooms == NULL || room_id == NULL)
    {
        return NULL;
    }
    for (size_t i = 0; i < state->room_count; i++)
    {
        const room_t* room = &state->rooms[i];
        if (room->room_id != NULL && strcmp(room->room_id, room_id) == 0)
        {
            return room;
        }
    }
    return NULL;
}

static void apply_room_to_card(ui_page_rooms_ctx_t* ctx, size_t index, const room_t* room)
{
    if (ctx == NULL || index >= ROOM_CARD_COUNT || ctx->cards[index] == NULL)
    {
        return;
    }

    ui_room_card_state_t card_state = {
        .on                = false,
        .available         = false,
        .temp_c            = 24,
        .humidity          = 48,
        .primary_entity_id = NULL,
    };

    if (room != NULL)
    {
        const room_entity_t* entity = room_primary_entity(room);
        if (entity != NULL)
        {
            card_state.on                = entity->on;
            card_state.available         = entity->available;
            card_state.primary_entity_id = entity->entity_id;
        }
        card_state.temp_c   = room->temp_c;
        card_state.humidity = room->humidity;
    }

    ui_room_card_set_state(ctx->cards[index], &card_state);
    ctx->slots[index].entity_id = card_state.primary_entity_id;
}

static void play_intro(ui_page_rooms_ctx_t* ctx)
{
    if (ctx == NULL || ctx->intro_played)
    {
        return;
    }

    play_toolbar_intro(ctx->toolbar);

    for (size_t i = 0; i < ROOM_CARD_COUNT; i++)
    {
        ui_room_card_play_enter_anim(ctx->cards[i], (uint32_t)(i * 40));
    }

    ctx->intro_played = true;
}

static lv_obj_t* create_content(lv_obj_t* page, ui_page_rooms_ctx_t* ctx)
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

    ctx->content = content;

    ctx->toolbar = lv_obj_create(content);
    toolbar_style_init(ctx->toolbar);

    lv_obj_t* title = lv_label_create(ctx->toolbar);
    lv_label_set_text(title, "Rooms");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_32, LV_PART_MAIN);
    lv_obj_set_style_text_color(title, ui_theme_color_on_surface(), LV_PART_MAIN);

    create_cards(ctx);

    return content;
}

lv_obj_t* ui_page_rooms_create(lv_obj_t* parent)
{
    if (parent == NULL)
    {
        return NULL;
    }

    ui_page_rooms_ctx_t* ctx = (ui_page_rooms_ctx_t*)lv_malloc(sizeof(ui_page_rooms_ctx_t));
    if (ctx == NULL)
    {
        return NULL;
    }
    memset(ctx, 0, sizeof(ui_page_rooms_ctx_t));

    lv_obj_t* page = lv_obj_create(parent);
    lv_obj_remove_style_all(page);
    lv_obj_set_size(page, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(page, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_scroll_dir(page, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(page, LV_SCROLLBAR_MODE_OFF);
    lv_obj_add_flag(page, LV_OBJ_FLAG_CLICKABLE);

    ctx->page    = page;
    ctx->content = create_content(page, ctx);

    ctx->wallpaper = ui_wallpaper_attach(page);
    lv_obj_add_event_cb(page, ui_page_rooms_delete_cb, LV_EVENT_DELETE, ctx);

    s_ctx = ctx;

    ui_page_rooms_set_state(rooms_provider_get_state());
    play_intro(ctx);

    return page;
}

void ui_page_rooms_set_state(const rooms_state_t* state)
{
    if (s_ctx == NULL)
    {
        return;
    }

    for (size_t i = 0; i < ROOM_CARD_COUNT; i++)
    {
        const char*   room_id = s_ctx->slots[i].room_id;
        const room_t* room    = find_room(state, room_id);
        apply_room_to_card(s_ctx, i, room);
    }
}

static ui_room_card_t* find_card_by_room(const char* room_id)
{
    if (s_ctx == NULL || room_id == NULL)
    {
        return NULL;
    }

    for (size_t i = 0; i < ROOM_CARD_COUNT; i++)
    {
        if (s_ctx->slots[i].room_id != NULL && strcmp(s_ctx->slots[i].room_id, room_id) == 0)
        {
            return s_ctx->cards[i];
        }
    }
    return NULL;
}

lv_obj_t* ui_page_rooms_get_card(const char* room_id)
{
    ui_room_card_t* card = find_card_by_room(room_id);
    return card != NULL ? ui_room_card_get_obj(card) : NULL;
}

lv_obj_t* ui_page_rooms_get_toggle(const char* room_id)
{
    ui_room_card_t* card = find_card_by_room(room_id);
    return card != NULL ? ui_room_card_get_toggle(card) : NULL;
}
