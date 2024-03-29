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
   struct Action    *onEnterActions;
   uint16_t         *onEnterActionsSizes;
   struct Action    *onLeaveActions;
   uint16_t         *onLeaveActionsSizes;
   uint32_t          exitMapsQuantity;
   uint16_t          onEnterActionsQuantity;
   uint16_t          onLeaveActionsQuantity;
};

struct OpenWorldInfoDynamic
{
   struct ExitMap2D *exitMaps;
   struct Action    *onEnterActions;
   uint16_t         *onEnterActionsSizes;
   struct Action    *onLeaveActions;
   uint16_t         *onLeaveActionsSizes;
   uint32_t          exitMapsQuantity;
   uint16_t          onEnterActionsQuantity;
   uint16_t          onLeaveActionsQuantity;
   uint32_t          exitMapsAllocated;
   uint16_t          onEnterActionsSizesAllocated;
   uint16_t          onLeaveActionsSizesAllocated;
};

#endif // CCE_OPENWORLD_H
