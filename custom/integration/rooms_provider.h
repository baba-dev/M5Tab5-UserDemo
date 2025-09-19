/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include "../ui/pages/ui_rooms_model.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Retrieve the current rooms state snapshot.
     *
     * The pointer remains owned by the provider. Callers should treat the
     * returned data as read-only and copy it if a longer lifetime is needed.
     */
    const rooms_state_t* rooms_provider_get_state(void);

    /**
     * @brief Replace the active rooms state snapshot.
     *
     * Passing NULL clears the current snapshot so consumers can reset their
     * views while awaiting new data.
     */
    void rooms_provider_set_state(const rooms_state_t* state);

    /**
     * @brief Restore the provider's built-in mock snapshot.
     */
    void rooms_provider_reset_state(void);

#ifdef __cplusplus
}
#endif
