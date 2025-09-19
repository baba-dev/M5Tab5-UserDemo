/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "lvgl.h"

static const uint8_t kLauncherBgStubData[] = {0x00, 0x00};

const lv_image_dsc_t launcher_bg = {
    .header =
        {
            .cf    = LV_COLOR_FORMAT_RGB565,
            .magic = LV_IMAGE_HEADER_MAGIC,
            .w     = 1,
            .h     = 1,
        },
    .data_size = sizeof(kLauncherBgStubData),
    .data      = kLauncherBgStubData,
};
