/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "custom/ui/pages/ui_page_rooms.h"
#include "lvgl.h"

#define TEST_SCREEN_WIDTH  1280
#define TEST_SCREEN_HEIGHT 720

static lv_color16_t s_draw_buffer[TEST_SCREEN_WIDTH * TEST_SCREEN_HEIGHT];
static lv_color16_t s_frame_buffer[TEST_SCREEN_WIDTH * TEST_SCREEN_HEIGHT];

static void test_flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map)
{
    LV_UNUSED(disp);
    if (area == NULL || px_map == NULL)
    {
        return;
    }

    const lv_color16_t* src    = (const lv_color16_t*)px_map;
    int32_t             width  = (area->x2 - area->x1) + 1;
    int32_t             height = (area->y2 - area->y1) + 1;

    for (int32_t y = 0; y < height; y++)
    {
        lv_color16_t* dst = &s_frame_buffer[(area->y1 + y) * TEST_SCREEN_WIDTH + area->x1];
        memcpy(dst, &src[y * width], (size_t)width * sizeof(lv_color16_t));
    }

    lv_display_flush_ready(disp);
}

typedef struct
{
    int         toggle_count;
    int         open_sheet_count;
    const char* last_room;
    const char* last_entity;
} event_capture_t;

static void capture_event_cb(lv_event_t* event)
{
    if (event == NULL)
    {
        return;
    }

    event_capture_t* capture = (event_capture_t*)lv_event_get_user_data(event);
    if (capture == NULL)
    {
        return;
    }

    const ui_page_rooms_event_data_t* data =
        (const ui_page_rooms_event_data_t*)lv_event_get_param(event);
    lv_event_code_t code = lv_event_get_code(event);

    if (code == UI_PAGE_ROOMS_EVENT_TOGGLE)
    {
        capture->toggle_count++;
    }
    else if (code == UI_PAGE_ROOMS_EVENT_OPEN_SHEET)
    {
        capture->open_sheet_count++;
    }

    if (data != NULL)
    {
        capture->last_room   = data->room_id;
        capture->last_entity = data->entity_id;
    }
}

static bool ensure(bool condition, const char* message)
{
    if (!condition)
    {
        fprintf(stderr, "[rooms_page_test] %s\n", message);
    }
    return condition;
}

int main(void)
{
    lv_init();

    lv_display_t* disp = lv_display_create(TEST_SCREEN_WIDTH, TEST_SCREEN_HEIGHT);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp, test_flush_cb);
    lv_display_set_buffers(
        disp, s_draw_buffer, NULL, sizeof(s_draw_buffer), LV_DISPLAY_RENDER_MODE_DIRECT);

    lv_obj_t* screen = lv_screen_active();
    lv_obj_clean(screen);

    lv_obj_t* page = ui_page_rooms_create(screen);
    if (page == NULL)
    {
        fprintf(stderr, "[rooms_page_test] Failed to create Rooms page\n");
        return 1;
    }

    event_capture_t capture = {0};
    lv_obj_add_event_cb(page, capture_event_cb, UI_PAGE_ROOMS_EVENT_TOGGLE, &capture);
    lv_obj_add_event_cb(page, capture_event_cb, UI_PAGE_ROOMS_EVENT_OPEN_SHEET, &capture);

    lv_timer_handler_run_in_period(5);

    static const size_t k_room_count = 3;
    const char*         room_ids[3]  = {"bakery", "bedroom", "living"};
    const char* entity_ids[3] = {"light.bakery_main", "light.bedroom_main", "light.living_main"};

    for (size_t i = 0; i < k_room_count; i++)
    {
        lv_obj_t* toggle = ui_page_rooms_get_toggle(room_ids[i]);
        if (!ensure(toggle != NULL, "Toggle handle missing"))
        {
            return 1;
        }
        lv_obj_send_event(toggle, LV_EVENT_CLICKED, NULL);
        if (!ensure(capture.toggle_count == (int)(i + 1), "Toggle event not recorded"))
        {
            return 1;
        }
        if (!ensure(capture.last_room != NULL && strcmp(capture.last_room, room_ids[i]) == 0,
                    "Incorrect room id"))
        {
            return 1;
        }
        if (!ensure(capture.last_entity != NULL && strcmp(capture.last_entity, entity_ids[i]) == 0,
                    "Incorrect entity id"))
        {
            return 1;
        }
    }

    for (size_t i = 0; i < k_room_count; i++)
    {
        lv_obj_t* card = ui_page_rooms_get_card(room_ids[i]);
        if (!ensure(card != NULL, "Card handle missing"))
        {
            return 1;
        }
        lv_obj_send_event(card, LV_EVENT_LONG_PRESSED, NULL);
        if (!ensure(capture.open_sheet_count == (int)(i + 1), "Long press event not recorded"))
        {
            return 1;
        }
        if (!ensure(capture.last_room != NULL && strcmp(capture.last_room, room_ids[i]) == 0,
                    "Incorrect room id on long press"))
        {
            return 1;
        }
    }

    lv_obj_del(page);
    lv_display_delete(disp);
    lv_deinit();

    return 0;
}
