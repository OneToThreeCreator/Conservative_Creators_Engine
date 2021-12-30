/*
    CoffeeChain - open source engine for making games.
    Copyright (C) 2020-2021 Andrey Givoronsky

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
    USA
*/

#ifndef ENGINE_COMMON_INTERNAL_H
#define ENGINE_COMMON_INTERNAL_H
#include "engine_common.h"
#include <stdio.h>
//#include "audio.h"
#include "external/glad.h"

// Can only be a power of 2
#define CCE_ALLOCATION_STEP 8u

// Some default resolutions for different aspect ratios
#define CCE_DEFAULT_WINDOW_WIDTH        800u
#define CCE_DEFAULT_WINDOW_WIDTH_3BY2   840u
#define CCE_DEFAULT_WINDOW_WIDTH_14BY9  840u
#define CCE_DEFAULT_WINDOW_WIDTH_7BY3   840u
#define CCE_DEFAULT_WINDOW_HEIGHT_1BY1  800u
#define CCE_DEFAULT_WINDOW_HEIGHT_5BY4  672u
#define CCE_DEFAULT_WINDOW_HEIGHT_4BY3  600u
#define CCE_DEFAULT_WINDOW_HEIGHT_3BY2  560u
#define CCE_DEFAULT_WINDOW_HEIGHT_14BY9 540u
#define CCE_DEFAULT_WINDOW_HEIGHT_8BY5  500u
#define CCE_DEFAULT_WINDOW_HEIGHT_5BY3  480u
#define CCE_DEFAULT_WINDOW_HEIGHT_16BY9 450u
#define CCE_DEFAULT_WINDOW_HEIGHT_2BY1  400u
#define CCE_DEFAULT_WINDOW_HEIGHT_7BY3  360u

#define CCE_DEFAULT_WINDOW_LABEL "CoffeeChainEngine Window"

#if UINT_FAST16_MAX == UINT16_MAX
#define SHIFT_OF_FAST_SIZE 1u
#define BITWIZE_AND_OF_FAST_SIZE 15u
#elif UINT_FAST16_MAX == UINT32_MAX
#define SHIFT_OF_FAST_SIZE 2u
#define BITWIZE_AND_OF_FAST_SIZE 31u
#else
#define SHIFT_OF_FAST_SIZE 3u
#define BITWIZE_AND_OF_FAST_SIZE 63u
#endif

#define CCE_PROCESS_TEMPORARY_BOOLS_ARRAY 0x10
#define CCE_ENGINE_STOP 0x80

#define MAX(x,y) (((x) > (y))?(x):(y))
#define MIN(x,y) (((x) < (y))?(x):(y))

struct UsedTemporaryBools
{
   uint8_t flags; /* 0x1 - used, 0x4 - to be cleared */
   uint_fast16_t *temporaryBools;
};

extern const uint8_t *const cce__flags;

int initEngine (const char *label, uint16_t globalBoolsQuantity);
cce_ubyte checkCollision (int32_t element1_x, int32_t element1_y, int32_t element1_width, int32_t element1_height, int32_t element2_x, int32_t element2_y, int32_t element2_width, int32_t element2_height);
void processLogic (uint32_t logicQuantity, struct ElementLogic *logic, struct Timer *timers, void (**doAction)(void*),
                   cce_ubyte (*fourth_if_func)(uint16_t, va_list), ...);
void terminateEngine (void);
struct ElementGroup* loadGroups (uint16_t groupsQuantity, FILE *map_f);
void writeGroups (uint16_t groupsQuantity, struct ElementGroup *groups, FILE *map_f);
struct ElementLogic* loadLogic (uint8_t logicQuantity, FILE *map_f);
void writeLogic (uint8_t logicQuantity, struct ElementLogic *logic, FILE *map_f);
void callActions (void (**doAction)(void*), uint8_t actionsQuantity, uint32_t *actionsIDs, uint32_t *actionsArgOffsets, cce_void *actionsArg);
uint16_t getFreeTemporaryBools (void);
void releaseTemporaryBools (uint16_t ID);
void releaseUnusedTemporaryBools (uint16_t ID);
void setCurrentTemporaryBools (uint16_t temporaryBoolsID);
void engineUpdate (void);
void doNothing (void);

extern void (*toFullscreen) (void);
extern void (*toWindow) (void);
extern void (*showWindow) (void);
extern void (*swapBuffers) (void);
extern struct cce_uvec2 (*getCurrentStep) (void);

uint8_t checkDirectoryExistance (const char *restrict path);

#endif // ENGINE_COMMON_INTERNAL_H