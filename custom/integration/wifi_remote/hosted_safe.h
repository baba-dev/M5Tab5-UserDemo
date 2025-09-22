/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdbool.h>

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

bool hosted_try_init_with_retries(void);
void hosted_deinit_safe(void);

#ifdef __cplusplus
}
#endif
