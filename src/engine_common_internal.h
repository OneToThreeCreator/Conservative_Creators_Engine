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

#if !defined(CCE_ALLOCATION_STEP)
#define CCE_ALLOCATION_STEP 8u
#elif ((CCE_ALLOCATION_STEP) & ((CCE_ALLOCATION_STEP) - 1)) != 0
#error "CCE_ALLOCATION_STEP must be a power of 2"
#endif

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

#define MACRO_TO_STR_IMPL(x) #x 
#define MACRO_TO_STR(x) MACRO_TO_STR_IMPL(x)

#define CCE_CEIL_SIZE_TO_ALLOCATION_STEP(s) ((s & ~(CCE_ALLOCATION_STEP - 1u)) + CCE_ALLOCATION_STEP * ((s & (CCE_ALLOCATION_STEP - 1u)) > 0))

struct UsedTemporaryBools
{
   uint8_t flags; /* 0x1 - used, 0x2 - to be cleared */
   uint_fast16_t *temporaryBools;
};

extern const uint8_t *const cce__flags;

int cce__initEngine (const char *label, uint16_t globalBoolsQuantity);
void cce__processLogic (uint32_t logicQuantity, struct ElementLogic *logic, struct Timer *timers, void (**doAction)(void*),
                   cce_ubyte (*fourth_if_func)(uint16_t, va_list), ...);
void cce__terminateEngine (void);
struct ElementGroup* cce__loadGroups (uint16_t groupsQuantity, FILE *map_f);
void cce__writeGroups (uint16_t groupsQuantity, struct ElementGroup *groups, FILE *map_f);
struct ElementLogic* cce__loadLogic (uint32_t logicQuantity, FILE *map_f, void (**endianConvertAction)(void*));
void cce__writeLogic (uint32_t logicQuantity, struct ElementLogic *logic, FILE *map_f, void (**endianConvertAction)(void*));
void cce__callActions (void (**doAction)(void*), uint8_t actionsQuantity, uint32_t *actionsIDs, uint32_t *actionsArgOffsets, cce_void *actionsArg);
uint16_t cce__getFreeTemporaryBools (void);
void cce__releaseTemporaryBools (uint16_t ID);
void cce__releaseUnusedTemporaryBools (uint16_t ID);
void cce__setCurrentTemporaryBools (uint16_t temporaryBoolsID);
void cce__engineUpdate (void);
void cce__doNothing (void);
void cce__initEndianConversion (void);

extern void (*cce__toFullscreen) (void);
extern void (*cce__toWindow) (void);
extern void (*cce__showWindow) (void);
extern void (*cce__swapBuffers) (void);
extern struct cce_uvec2 (*cce__getCurrentStep) (void);

#endif // ENGINE_COMMON_INTERNAL_H
