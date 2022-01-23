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

#ifndef BASE_ACTIONS_H
#define BASE_ACTIONS_H
#include <stdint.h>
#include "../engine_common.h"

#ifdef __cplusplus
extern C:
{
#endif // __cplusplus

#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__TOS_WIN__) || defined(__WINDOWS__) || \
    defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN__)
#include "cce_exports.h"
#define CCE_PUBLIC_OPTIONS CCE_EXPORTS
#else
#define CCE_PUBLIC_OPTIONS
#endif // Windows

#define CCE_CURRENT_MAP2D 0x1
#define CCE_DYNAMIC_MAP2D 0x2

#define CCE_SET   CCE_ENABLE_BOOL
#define CCE_SHIFT CCE_SWITCH_BOOL

/*
   Every structure here has 32-bit alignment. Using 64-bit aligned structure is not recommended, due to possibility of misaligned reading
*/

struct moveActionStruct
{
   int32_t x;
   int32_t y;
   uint16_t groupID;
   cce_enum action;
   cce_enum mapType;
};

struct extendActionStruct
{
   int32_t x;
   int32_t y;
   uint16_t groupID;
   cce_enum action;
   cce_enum mapType;
};

struct rotateActionStruct
{
   float angle;
   int32_t xOffset;
   int32_t yOffset;
   uint8_t groupID;
   cce_enum mapType;
   uint16_t __pad;
};

struct offsetTextureActionStruct
{
   int32_t offsetX;
   int32_t offsetY;
   uint8_t groupID;
   cce_enum mapType;
   uint16_t __pad;
};

struct changeColorActionStruct
{
   float red;
   float green;
   float blue;
   float alpha;
   uint8_t groupID;
   cce_enum mapType;
   uint16_t __pad;
};

struct setBoolActionStruct
{
   uint16_t boolID;
   cce_enum action;
   uint8_t __pad;
};

struct setPlotNumberActionStruct
{
   uint16_t value;
   cce_enum action;
   uint8_t __pad;
};

struct startTimerActionStruct
{
   uint16_t ID;
   cce_enum mapType;
   uint8_t __pad;
};

struct setDynamicTimerDelayActionStruct
{
   float delay;
   uint16_t ID;
   cce_enum action;
   uint8_t __pad;
};

CCE_PUBLIC_OPTIONS void  cceMoveGlobalOffsetGroupMap2D (int32_t x, int32_t y, cce_enum actionType);
CCE_PUBLIC_OPTIONS void  cceMoveGroupMap2D (uint16_t groupID, int32_t x, int32_t y, cce_enum actionType, cce_enum mapType);
CCE_PUBLIC_OPTIONS void  cceExtendGroupMap2D (uint16_t groupID, int32_t x, int32_t y, cce_enum actionType, cce_enum mapType);
CCE_PUBLIC_OPTIONS float cceNormalizeAngle (float angleInDegrees);
CCE_PUBLIC_OPTIONS void  cceRotateGroupMap2D (uint8_t groupID, float normalizedAngle, int32_t xOffset, int32_t yOffset, cce_enum mapType);
CCE_PUBLIC_OPTIONS void  cceOffsetTextureGroupMap2D (uint8_t groupID, int32_t offsetX, int32_t offsetY, cce_enum mapType);
CCE_PUBLIC_OPTIONS void  cceChangeColorGroupMap2D (uint8_t groupID, float r, float g, float b, float a, cce_enum mapType);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // BASE_ACTIONS_H
