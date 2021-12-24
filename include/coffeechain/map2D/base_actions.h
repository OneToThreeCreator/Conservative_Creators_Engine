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
#define CCE_OPTIONS CCE_EXPORTS
#else
#define CCE_OPTIONS
#endif // Windows

struct moveActionStruct
{
   int32_t x;
   int32_t y;
   uint16_t groupID;
};

struct extendActionStruct
{
   int32_t x;
   int32_t y;
   uint16_t groupID;
};

struct rotateActionStruct
{
   float angle;
   int32_t xOffset;
   int32_t yOffset;
   uint16_t groupID;
};

struct offsetTextureActionStruct
{
   int32_t offsetX;
   int32_t offsetY;
   uint16_t groupID;
};

struct changeColorActionStruct
{
   float red;
   float green;
   float blue;
   float alpha;
   uint16_t groupID;
};

struct setBoolActionStruct
{
   uint16_t boolID;
   cce_enum action;
};

#define CCE_INCREASE_PLOT_NUMBER 0x1
#define CCE_SET_PLOT_NUMBER 0x2

struct setPlotNumberActionStruct
{
   uint16_t value;
   cce_enum action;
};

CCE_OPTIONS void  moveGroupMap2D (uint16_t groupID, int32_t x, int32_t y);
CCE_OPTIONS void  extendGroupMap2D (uint16_t groupID, int32_t x, int32_t y);
CCE_OPTIONS float normalizeAngle (float angleInDegrees);
CCE_OPTIONS void  rotateGroupMap2D (uint8_t groupID, float normalizedAngle, int32_t xOffset, int32_t yOffset);
CCE_OPTIONS void  offsetTextureGroupMap2D (uint8_t groupID, int32_t offsetX, int32_t offsetY);
CCE_OPTIONS void  changeColorGroupMap2D (uint8_t groupID, float r, float g, float b, float a);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // BASE_ACTIONS_H