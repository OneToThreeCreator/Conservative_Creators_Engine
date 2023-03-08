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

#ifndef TEXT_RENDERING_H
#define TEXT_RENDERING_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include "../../engine_common.h"
#include "map2D.h"

#define CCE_ASCII_EXT_ENCODING 0x1
#define CCE_UTF8_ENCODING      0x2

typedef uint8_t enc_type;

CCE_API int cceInitTextRendering (enc_type encoding);
CCE_API int cceLoadBitmapFont (const char *cceFontName);
CCE_API extern uint32_t* (*ccePrintString)(char *string, struct Map2DElementDev *elementTemplate, cce_enum elementType, uint8_t isCurrentPosition);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // TEXT_RENDERING_H
