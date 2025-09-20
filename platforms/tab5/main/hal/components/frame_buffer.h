/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <cstddef>
#include <cstdint>

struct FrameBuffer
{
    uint32_t width;
    uint32_t height;
    uint32_t bytes_per_pixel;
    size_t   size;
    uint8_t* data;
};
