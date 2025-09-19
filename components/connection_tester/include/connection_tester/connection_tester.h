/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    esp_err_t connection_tester_http_get(const char* url, int timeout_ms, int* status_code);

#ifdef __cplusplus
}
#endif
