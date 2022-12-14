/*
    Conservative Creator's Engine - open source engine for making games.
    Copyright (C) 2020-2022 Andrey Gaivoronskiy

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

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdio.h>

#include "../include/cce/engine_common.h"

// Fine for games with 16x16 textures, when using integer scaling.
#define CCE_DEFAULT_WINDOW_WIDTH  256u
#define CCE_DEFAULT_WINDOW_HEIGHT 144u

#define CCE_DEFAULT_WINDOW_LABEL "CoffeeChainEngine Window"

#define CCE_ENGINE_STOP 0x80

extern const uint8_t *const cce__flags;
extern uint64_t cce__currentTime, cce__deltaTime;

int cce__initEngine (const char *label);
void cce__terminateEngine (void);
void cce__engineUpdate (void);
void cce__doNothing (void);
void cce__shortToString (char *str, const unsigned short number, const char *strEnd);
int cce__keyIniCallback(void *data, const char *name, const char *value);

extern struct cce_backend_data
{
   void (*engineUpdate) (void);
   void (*terminateEngine) (void);
   uint64_t (*getTime) (void);
   void (*toFullscreen) (void);
   void (*toWindow) (void);
   void (*screenUpdate) (void);
}
cce__engineBackend;

#define cce__toFullscreen() cce__engineBackend.toFullscreen()
#define cce__toWindow() cce__engineBackend.toWindow()
#define cce__screenUpdate() cce__engineBackend.screenUpdate()

extern uint8_t  cce__axesPairChanged;
extern int8_t   cce__axes[8];
extern uint16_t cce__buttonsBitField;
extern uint16_t cce__buttonsBitFieldDiff;
extern void (*cce__keyCallback)(cce_enum key, cce_enum state);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ENGINE_COMMON_INTERNAL_H
