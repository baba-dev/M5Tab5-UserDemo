/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "rooms_provider.h"

#include <stddef.h>

static room_entity_t ENT_BAKERY_MAIN = {
    .entity_id = "light.bakery_main",
    .kind      = ROOM_ENTITY_LIGHT,
    .available = true,
    .on        = false,
    .value     = -1,
};

static room_entity_t ENT_BEDROOM_MAIN = {
    .entity_id = "light.bedroom_main",
    .kind      = ROOM_ENTITY_LIGHT,
    .available = true,
    .on        = true,
    .value     = 75,
};

static room_entity_t ENT_LIVING_MAIN = {
    .entity_id = "light.living_main",
    .kind      = ROOM_ENTITY_LIGHT,
    .available = true,
    .on        = false,
    .value     = -1,
};

static room_entity_t* ROOM0_ENTS[] = {&ENT_BAKERY_MAIN};
static room_entity_t* ROOM1_ENTS[] = {&ENT_BEDROOM_MAIN};
static room_entity_t* ROOM2_ENTS[] = {&ENT_LIVING_MAIN};

static room_t ROOMS[] = {
    {
        .room_id      = "bakery",
        .name         = "Bakery",
        .entities     = (room_entity_t**)ROOM0_ENTS,
        .entity_count = sizeof(ROOM0_ENTS) / sizeof(ROOM0_ENTS[0]),
        .temp_c       = 24,
        .humidity     = 48,
    },
    {
        .room_id      = "bedroom",
        .name         = "Bedroom",
        .entities     = (room_entity_t**)ROOM1_ENTS,
        .entity_count = sizeof(ROOM1_ENTS) / sizeof(ROOM1_ENTS[0]),
        .temp_c       = 23,
        .humidity     = 50,
    },
    {
        .room_id      = "living",
        .name         = "Living Room",
        .entities     = (room_entity_t**)ROOM2_ENTS,
        .entity_count = sizeof(ROOM2_ENTS) / sizeof(ROOM2_ENTS[0]),
        .temp_c       = 25,
        .humidity     = 45,
    },
};

static rooms_state_t DEFAULT_STATE = {
    .rooms      = ROOMS,
    .room_count = sizeof(ROOMS) / sizeof(ROOMS[0]),
};

static const rooms_state_t* s_current_state = &DEFAULT_STATE;

const rooms_state_t* rooms_provider_get_state(void)
{
    return s_current_state;
}

void rooms_provider_set_state(const rooms_state_t* state)
{
    s_current_state = state;
}

void rooms_provider_reset_state(void)
{
    s_current_state = &DEFAULT_STATE;
}
