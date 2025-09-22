/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdbool.h>

namespace custom::integration::wifi
{

    /**
     * @brief Ensure the ESP-Hosted transport and remote Wi-Fi stacks are ready.
     *
     * Performs the conservative start-up sequence for the ESP-Hosted SDIO link.
     * Returns true when the transport and Wi-Fi remote library were successfully
     * initialised.
     */
    bool HostedSafeStart();

}  // namespace custom::integration::wifi
