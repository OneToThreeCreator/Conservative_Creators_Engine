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

#ifndef CCE_OPENWORLD_H
#define CCE_OPENWORLD_H

#include <stdint.h>

#define CCE_EXITMAP2D_A_IS_X 0x1
#define CCE_EXITMAP2D_ONLESS_TRANSITION 0x2

struct ExitMap2D
{
   char   *mapName;
   int32_t xOffset;
   int32_t yOffset;
   int32_t aBorder;
   int32_t b1Border;
   int32_t b2Border;
   uint8_t flags; // 0x1 - a is x (otherwise a is y), 0x2 - b is to the south/west from globalOffset 0
};

struct OpenWorldInfo
{
   struct ExitMap2D *exitMaps;
   uint32_t exitMapsQuantity;
   uint32_t exitMapsAllocated;
};

#endif // CCE_OPENWORLD_H
