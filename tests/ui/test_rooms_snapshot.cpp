/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include <gtest/gtest.h>

extern "C"
{
#include "lvgl.h"
#include "ui_page_rooms.h"
}

namespace
{

    void FlushNoop(lv_display_t* display, const lv_area_t* area, uint8_t* px_map)
    {
        LV_UNUSED(area);
        LV_UNUSED(px_map);
        lv_display_flush_ready(display);
    }

}  // namespace

TEST(RoomsPage, CreatesAndRenders)
{
    lv_init();

    constexpr uint32_t kHorRes      = 320;
    constexpr uint32_t kVerRes      = 180;
    constexpr uint32_t kBufferLines = 20;

    static lv_color_t    buffer[kHorRes * kBufferLines];
    static lv_draw_buf_t draw_buffer;
    lv_result_t          init_result = lv_draw_buf_init(
        &draw_buffer, kHorRes, kBufferLines, LV_COLOR_FORMAT_NATIVE, 0, buffer, sizeof(buffer));
    ASSERT_EQ(init_result, LV_RESULT_OK);

    lv_display_t* display = lv_display_create(kHorRes, kVerRes);
    ASSERT_NE(display, nullptr);
    lv_display_set_draw_buffers(display, &draw_buffer, nullptr);
    lv_display_set_flush_cb(display, FlushNoop);
    lv_display_set_default(display);

    lv_obj_t* screen = lv_screen_active();
    lv_obj_t* page   = ui_page_rooms_create(screen);
    ASSERT_NE(page, nullptr);

    for (int i = 0; i < 5; ++i)
    {
        lv_tick_inc(5);
        lv_timer_handler();
    }

    SUCCEED();
}
