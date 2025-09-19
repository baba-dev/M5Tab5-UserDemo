/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef int32_t esp_err_t;

#define ESP_OK                  0
#define ESP_FAIL                -1
#define ESP_ERR_NO_MEM          0x103
#define ESP_ERR_INVALID_ARG     0x102
#define ESP_ERR_INVALID_STATE   0x107
#define ESP_ERR_INVALID_SIZE    0x109
#define ESP_ERR_INVALID_VERSION 0x10B
#define ESP_ERR_NOT_FOUND       0x1102
#define ESP_ERR_NVS_NOT_FOUND   0x1102

#ifdef __cplusplus
}
#endif
