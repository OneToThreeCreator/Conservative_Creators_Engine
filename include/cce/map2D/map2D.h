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

#ifndef MAP2D_H
#define MAP2D_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdio.h>
#include <stdint.h>
#include "../engine_common.h"
#include "../engine_common_IO.h"

#define CCE_BASIC_ACTIONS_QUANTITY 16u

#define CCE_INCORRECT_ENUM 1
#define CCE_ATTEMPT_TO_OVERRIDE_DEFAULT_ELEMENT 2
#define CCE_ELEMENT_DOES_NOT_EXIST 3
#define CCE_OUT_OF_BOUNDS 4


#define CCE_RECTANGLE_COLLIDER                     0x000 // Default
#define CCE_CIRCLE_COLLIDER                        0x001
#define CCE_RETURN_FALLBACK_ON_MAP_LOADING_FAILURE 0x000
#define CCE_RETURN_NULL_ON_MAP_LOADING_FAILURE     0x002
#define CCE_STORE_COLOR_IN_RGB                     0x000 // Default
#define CCE_STORE_COLOR_IN_HSV                     0x020
#define CCE_STORE_COLOR_IN_HSL                     0x040
#define CCE_STORE_COLOR_IN_HCL                     0x060

#define CCE_MAP2D_STATIC  0x1
#define CCE_MAP2D_DYNAMIC 0x2

#define CCE_DEFAULT 0

typedef uint_fast32_t cce_flag;

#define CCE_TRANSFORM_GROUP 0x10
#define CCE_COLLISION_GROUP 0x11

#define CCE_COLLISION_LOGIC_ELEMENT   0xB


struct ElementGroup
{   
   uint32_t *elements;
   uint16_t  elementsQuantity;
};

struct CollisionGroup
{
   uint16_t group1;
   uint16_t group2;
};

#define CCE_MAP2DELEMENT_GLOBALOFFSET CCE_MAP2DELEMENT_GLOBALOFFSET
#define CCE_MAP2DELEMENT_HAS_COLLIDER 0x1
#define CCE_MAP2DELEMENT_HAS_ELEMENT 0x2
#define CCE_MAP2DELEMENT_POSITION_NOT_CURRENT 0x4

struct Action;

typedef int (*cce_rloadfun)(void *buffer, struct cce_buffer *info, char **names);
typedef char** (*cce_rstorefun)(void *buffer, struct cce_buffer *info);

CCE_PUBLIC_OPTIONS uint32_t cceRegisterMapCustomResourceCallback (cce_rloadfun onLoad, cce_dataparsefun onFree, cce_dataparsefun onCreate, cce_rstorefun onStore, size_t bufferSize);
CCE_PUBLIC_OPTIONS void cceSetMap2Dpath (const char *path);
CCE_PUBLIC_OPTIONS int cceInitEngine2D (const char *gameINIpath);
CCE_PUBLIC_OPTIONS void cceSetTexturesPath (const char *path);
CCE_PUBLIC_OPTIONS void cceRenderingLayerSetMap2D (uint8_t layer, uint8_t mapLayer, struct cce_buffer *map);
CCE_PUBLIC_OPTIONS void cceRenderMap2D (void);
CCE_PUBLIC_OPTIONS void cceUpdateEngineMap2D (void);
CCE_PUBLIC_OPTIONS int cceEngine2D (void);
CCE_PUBLIC_OPTIONS void cceSetLoadedMap2D (uint16_t number, struct cce_i32vec2 globalPosition);
CCE_PUBLIC_OPTIONS const char* cceGetResourcePath (void);
CCE_PUBLIC_OPTIONS uint16_t cceLoadTexture (char *path);
CCE_PUBLIC_OPTIONS struct cce_buffer* cceLoadMap2D(char *path);
CCE_PUBLIC_OPTIONS struct cce_buffer* cceLoadMap2Ddynamic(char *path);
CCE_PUBLIC_OPTIONS int cceWriteMap2Ddynamic (struct cce_buffer *map, char *path);

CCE_PUBLIC_OPTIONS extern const struct cce_u16vec2 *const cceTextureSize;

#define cceFreeMap2D(map)        cceFreeBuffer(map)
#define cceFreeMap2Ddynamic(map) cceFreeBuffer(map)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MAP2D_H
