/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include <gtest/gtest.h>

extern "C"
{
#include "lvgl.h"
#include "ui_page_settings.h"
}

namespace
{

    void FlushNoop(lv_display_t* display, const lv_area_t* area, uint8_t* px_map)
    {
        LV_UNUSED(area);
        LV_UNUSED(px_map);
        lv_display_flush_ready(display);
    }

    void pump_lvgl(uint32_t iterations)
    {
        for (uint32_t i = 0; i < iterations; ++i)
        {
            lv_tick_inc(5);
            lv_timer_handler();
        }
    }

}  // namespace

TEST(SettingsPage, StatusLabelsReflectUpdates)
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
    ASSERT_NE(screen, nullptr);

    lv_obj_t* page = ui_page_settings_create(screen);
    ASSERT_NE(page, nullptr);

    pump_lvgl(4);

    EXPECT_STREQ("Idle", ui_page_settings_get_update_status());
    EXPECT_STREQ("Idle", ui_page_settings_get_diagnostics_status());
    EXPECT_STREQ("Idle", ui_page_settings_get_backup_status());

    ui_page_settings_set_update_status("OTA checking...");
    ui_page_settings_set_diagnostics_status("Diagnostics running");
    ui_page_settings_set_backup_status("Backup queued");
    pump_lvgl(4);

    EXPECT_STREQ("OTA checking...", ui_page_settings_get_update_status());
    EXPECT_STREQ("Diagnostics running", ui_page_settings_get_diagnostics_status());
    EXPECT_STREQ("Backup queued", ui_page_settings_get_backup_status());

    ui_page_settings_set_backup_status(nullptr);
    pump_lvgl(2);
    EXPECT_STREQ("", ui_page_settings_get_backup_status());
}
