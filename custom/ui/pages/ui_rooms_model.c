/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#include "ui_rooms_model.h"

const room_entity_t* room_primary_entity(const room_t* room)
{
    if (room == NULL || room->entities == NULL || room->entity_count == 0)
    {
        return NULL;
    }
    return room->entities[0];
}
