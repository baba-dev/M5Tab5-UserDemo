/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include "custom/ui/pages/ui_page_rooms.h"
#include "lvgl.h"

#define SNAP_SCREEN_WIDTH  1280
#define SNAP_SCREEN_HEIGHT 720

static lv_color16_t s_snap_draw_buf[SNAP_SCREEN_WIDTH * SNAP_SCREEN_HEIGHT];
static lv_color16_t s_snap_frame_buf[SNAP_SCREEN_WIDTH * SNAP_SCREEN_HEIGHT];

static void snapshot_flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map)
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
        lv_color16_t* dst = &s_snap_frame_buf[(area->y1 + y) * SNAP_SCREEN_WIDTH + area->x1];
        memcpy(dst, &src[y * width], (size_t)width * sizeof(lv_color16_t));
    }

    lv_display_flush_ready(disp);
}

int main(void)
{
    lv_init();

    lv_display_t* disp = lv_display_create(SNAP_SCREEN_WIDTH, SNAP_SCREEN_HEIGHT);
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
    lv_display_set_flush_cb(disp, snapshot_flush_cb);
    lv_display_set_buffers(
        disp, s_snap_draw_buf, NULL, sizeof(s_snap_draw_buf), LV_DISPLAY_RENDER_MODE_DIRECT);

    lv_obj_t* screen = lv_screen_active();
    lv_obj_clean(screen);

    lv_obj_t* page = ui_page_rooms_create(screen);
    if (page == NULL)
    {
        fprintf(stderr, "[rooms_page_snapshot] Failed to create Rooms page\n");
        return 1;
    }

    for (int i = 0; i < 20; i++)
    {
        lv_timer_handler_run_in_period(16);
    }

    lv_refr_now(disp);

    const char* output_path = "tests/ui/output/rooms_three_cards.raw";
    FILE*       file        = fopen(output_path, "wb");
    if (file == NULL)
    {
        fprintf(stderr, "[rooms_page_snapshot] Unable to open %s for writing\n", output_path);
        return 1;
    }

    size_t written = fwrite(s_snap_frame_buf,
                            sizeof(s_snap_frame_buf[0]),
                            SNAP_SCREEN_WIDTH * SNAP_SCREEN_HEIGHT,
                            file);
    fclose(file);

    lv_obj_del(page);
    lv_display_delete(disp);
    lv_deinit();

    if (written != (size_t)(SNAP_SCREEN_WIDTH * SNAP_SCREEN_HEIGHT))
    {
        fprintf(stderr, "[rooms_page_snapshot] Short write to %s\n", output_path);
        return 1;
    }

    return 0;
}
