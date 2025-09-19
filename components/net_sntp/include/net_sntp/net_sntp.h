/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdbool.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C"
{
#endif

    esp_err_t net_sntp_start(const char* server, bool wait_for_sync);
    void      net_sntp_stop(void);

#ifdef __cplusplus
}
#endif
