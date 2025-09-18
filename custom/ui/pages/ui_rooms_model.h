/*
 * SPDX-FileCopyrightText: 2025 M5Stack Technology CO LTD
 *
 * SPDX-License-Identifier: MIT
 */
#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    typedef enum
    {
        ROOM_ENTITY_LIGHT = 0,
        ROOM_ENTITY_SWITCH,
        ROOM_ENTITY_SENSOR,
    } room_entity_kind_t;

    typedef struct room_entity_t
    {
        const char*        entity_id;
        room_entity_kind_t kind;
        bool               available;
        bool               on;
        int32_t            value;
    } room_entity_t;

    typedef struct room_t
    {
        const char*     room_id;
        const char*     name;
        room_entity_t** entities;
        size_t          entity_count;
        int8_t          temp_c;
        uint8_t         humidity;
    } room_t;

    typedef struct rooms_state_t
    {
        room_t* rooms;
        size_t  room_count;
    } rooms_state_t;

    const room_entity_t* room_primary_entity(const room_t* room);

#ifdef __cplusplus
}
#endif
