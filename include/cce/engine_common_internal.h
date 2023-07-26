/*
   Conservative Creator's Engine - open source engine for making games.
   Copyright © 2020-2023 Andrey Gaivoronskiy

   This file is part of Conservative Creator's Engine.

   Conservative Creator's Engine is free software: you can redistribute it and/or modify it under 
   the terms of the GNU Lesser General Public License as published by the Free Software Foundation,
   either version 2 of the License, or (at your option) any later version.

   Conservative Creator's Engine is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
   PURPOSE. See the GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License along
   with Conservative Creator's Engine. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ENGINE_COMMON_INTERNAL_H
#define ENGINE_COMMON_INTERNAL_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdio.h>

#include "engine_common.h"

#define CCE_DEFAULT_WINDOW_LABEL "CCE Window"

#define CCE_ENGINE_STOP 0x80

extern const uint8_t *const cce__flags;
extern uint32_t cce__currentTime, cce__deltaTime;

CCE_API void cce__loadKeyboardBindingsBackendPlugin (int (*loadKeysFn)(void*), struct cce_ini_keys *buffer);
CCE_API void cce__registerBackend (const char *lowercasename, void *data, int (*iniCallback)(void*, const char*, const char*), int (*init)(void*), int (*postinit)(void), void (*term)(void), uint8_t flags);

extern struct cce_backend_data
{
   void (*engineUpdate) (void);
   void (*toFullscreen) (void);
   void (*toWindow) (void);
}
cce__engineBackend;

#define cce__toFullscreen() cce__engineBackend.toFullscreen()
#define cce__toWindow() cce__engineBackend.toWindow()

extern uint8_t  cce__axesPairChanged;
extern int8_t   cce__axes[8];
extern uint16_t cce__buttonsBitField;
extern uint16_t cce__buttonsBitFieldDiff;
extern void (*cce__keyCallback)(cce_enum key, cce_enum state);

extern struct cce_u16vec2 cce__gameResolution;

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ENGINE_COMMON_INTERNAL_H
