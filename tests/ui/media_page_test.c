/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "custom/ui/pages/ui_page_media.h"
#include "lvgl.h"

static const uint8_t k_dummy_wallpaper_data[] = {0x00, 0x00};

const lv_image_dsc_t launcher_bg = {
#if LV_BIG_ENDIAN
    .header = {.stride = 2,
               .h      = 1,
               .w      = 1,
               .flags  = 0,
               .cf     = LV_COLOR_FORMAT_RGB565,
               .magic  = LV_IMAGE_HEADER_MAGIC},
#else
    .header = {.magic      = LV_IMAGE_HEADER_MAGIC,
               .cf         = LV_COLOR_FORMAT_RGB565,
               .flags      = 0,
               .w          = 1,
               .h          = 1,
               .stride     = 2,
               .reserved_2 = 0},
#endif
    .data_size = sizeof(k_dummy_wallpaper_data),
    .data      = k_dummy_wallpaper_data,
    .reserved  = NULL,
};

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
    int         previous_count;
    int         play_pause_count;
    int         next_count;
    int         volume_count;
    int         scene_count;
    uint8_t     last_volume;
    const char* last_scene_id;
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

    const ui_page_media_event_t* data = (const ui_page_media_event_t*)lv_event_get_param(event);
    if (data == NULL)
    {
        return;
    }

    switch (data->signal)
    {
        case UI_PAGE_MEDIA_SIGNAL_PREVIOUS:
            capture->previous_count++;
            break;
        case UI_PAGE_MEDIA_SIGNAL_PLAY_PAUSE:
            capture->play_pause_count++;
            break;
        case UI_PAGE_MEDIA_SIGNAL_NEXT:
            capture->next_count++;
            break;
        case UI_PAGE_MEDIA_SIGNAL_VOLUME:
            capture->volume_count++;
            capture->last_volume = data->volume;
            break;
        case UI_PAGE_MEDIA_SIGNAL_TRIGGER_SCENE:
            capture->scene_count++;
            capture->last_scene_id = data->scene_id;
            break;
    }
}

static bool ensure(bool condition, const char* message)
{
    if (!condition)
    {
        fprintf(stderr, "[media_page_test] %s\n", message);
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

    lv_obj_t* page = ui_page_media_create(screen);
    if (!ensure(page != NULL, "Failed to create Media page"))
    {
        return 1;
    }

    event_capture_t capture = {0};
    lv_obj_add_event_cb(page, capture_event_cb, UI_PAGE_MEDIA_EVENT_COMMAND, &capture);

    lv_timer_handler_run_in_period(5);

    lv_obj_t* previous_btn = ui_page_media_get_previous_button();
    lv_obj_t* play_btn     = ui_page_media_get_play_pause_button();
    lv_obj_t* next_btn     = ui_page_media_get_next_button();
    lv_obj_t* volume       = ui_page_media_get_volume_slider();

    if (!ensure(previous_btn != NULL && play_btn != NULL && next_btn != NULL,
                "Transport controls missing"))
    {
        return 1;
    }

    lv_obj_send_event(previous_btn, LV_EVENT_CLICKED, NULL);
    lv_obj_send_event(play_btn, LV_EVENT_CLICKED, NULL);
    lv_obj_send_event(next_btn, LV_EVENT_CLICKED, NULL);

    if (!ensure(capture.previous_count == 1, "Previous event not captured"))
    {
        return 1;
    }
    if (!ensure(capture.play_pause_count == 1, "Play/Pause event not captured"))
    {
        return 1;
    }
    if (!ensure(capture.next_count == 1, "Next event not captured"))
    {
        return 1;
    }

    if (!ensure(volume != NULL, "Volume slider missing"))
    {
        return 1;
    }

    lv_slider_set_value(volume, 72, LV_ANIM_OFF);
    lv_obj_send_event(volume, LV_EVENT_VALUE_CHANGED, NULL);
    if (!ensure(capture.volume_count >= 1, "Volume event not captured"))
    {
        return 1;
    }
    if (!ensure(capture.last_volume == 72, "Volume value incorrect"))
    {
        return 1;
    }

    size_t scene_count = ui_page_media_get_scene_count();
    if (!ensure(scene_count > 0, "No quick scenes registered"))
    {
        return 1;
    }

    lv_obj_t* scene_btn = ui_page_media_get_scene_button(0);
    if (!ensure(scene_btn != NULL, "Scene button missing"))
    {
        return 1;
    }

    lv_obj_send_event(scene_btn, LV_EVENT_CLICKED, NULL);
    if (!ensure(capture.scene_count == 1, "Scene event not captured"))
    {
        return 1;
    }
    if (!ensure(capture.last_scene_id != NULL, "Scene id missing"))
    {
        return 1;
    }
    if (!ensure(strcmp(capture.last_scene_id, "scene.morning_mix") == 0, "Unexpected scene id"))
    {
        return 1;
    }

    return 0;
}
