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
#include "../utils.h"

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

#define CCE_MAP2DELEMENT_GLOBALOFFSET CCE_MAP2DELEMENT_GLOBALOFFSET
#define CCE_MAP2DELEMENT_HAS_COLLIDER 0x1
#define CCE_MAP2DELEMENT_HAS_ELEMENT 0x2
#define CCE_MAP2DELEMENT_POSITION_NOT_CURRENT 0x4

#define CCE_RESOURCE_TEXTURE 0

struct cce_action;
struct cce_renderingdata;

union cce_collidermap2D
{
   struct cce_collider_rect2D_16_8 rectangle;
   struct cce_collider_cir2D_16_16 circle;
};

struct cce_elementposition
{
   struct cce_i16vec2 position;
   uint16_t           textureDataID;
   uint8_t            textureDataOffsetGroup;
   uint8_t            __reserved;
};

CCE_ARRAY_STRUCT(cce_elementpositionarray, struct cce_elementposition, uint32_t);

struct cce_renderinginfo
{
   struct cce_renderingdata        *data;
   struct cce_elementpositionarray *positions;
   struct cce_element              *elements;
   uint16_t                         elementsQuantity;
   uint8_t                          layersQuantity;
   uint8_t                          flags;
};

struct cce_dynamicrenderinginfo
{
   struct cce_renderingdata        *data;
   struct cce_elementpositionarray *positions;
   struct cce_element              *elements;
   uint16_t                         elementsQuantity;
   uint8_t                          layersQuantity;
   uint8_t                          flags;
   uint16_t                         elementsAllocated;
};

#define CCE_ELEMENT_IGNORE_CAMERA 0x1
#define CCE_ELEMENT_FLIP_HORIZONTALLY 0x2
#define CCE_ELEMENT_FLIP_VERTICALLY 0x4

struct cce_element
{
   struct cce_i16vec2 position;
   union
   {
      struct cce_u16vec2 texturePosition;
      struct cce_u8vec4  rgba;
   } data;
   struct cce_u16vec2 size;
   uint16_t textureID;
   uint8_t  rotation;
   uint8_t  flags;
};

struct cce_collisioninfo
{
   union Collider2D      *colliders;
   struct ElementGroup   *transformGroups; // Groups can be used to transform colliders (Rotation of colliders is unsupported, use circle colliders instead)
   struct ElementGroup   *collisionGroups;
   struct CollisionGroup *collision;
   uint64_t              *collisionCache;  // Timestamp with last bit used as actual cache. Compared with lastTransformationTime
   
   uint32_t               collidersQuantity;
   uint16_t               transformGroupsQuantity;
   uint16_t               collisionGroupsQuantity;
   uint16_t               collisionQuantity;
};

struct cce_dynamiccollisioninfo
{
   union Collider2D      *colliders;
   struct ElementGroup   *transformGroups; // Groups can be used to transform colliders (Rotation of colliders is unsupported, use circle colliders instead)
   struct ElementGroup   *collisionGroups;
   struct CollisionGroup *collision;
   uint64_t              *collisionCache;  // Timestamp with last bit used as actual cache. Compared with lastTransformationTime
   uint32_t               collidersQuantity;
   uint16_t               transformGroupsQuantity;
   uint16_t               collisionGroupsQuantity;
   uint16_t               collisionQuantity;
   uint16_t               transformGroupsAllocated;
   uint32_t               collidersAllocated;
   uint16_t               collisionGroupsAllocated;
   uint16_t               collisionAllocated;
};

struct cce_actioninfo
{
   struct cce_action *onFreeActions;
   uint16_t          *onFreeActionsSizes;
   uint16_t           onFreeActionsQuantity;
};

struct cce_dynamicactioninfo
{
   struct cce_action *onFreeActions;
   uint16_t          *onFreeActionsSizes;
   uint16_t           onFreeActionsQuantity;
   uint16_t           onFreeActionsSizesAllocated;
   uint16_t           onLoadActionsQuantity;
   uint16_t           onLoadActionsSizesAllocated;
   struct cce_action *onLoadActions;
   uint16_t          *onLoadActionsSizes;
};

struct cce_usedtexinfo
{
   uint16_t *texturesMapDependsOn;
   uint16_t  texturesMapDependsOnQuantity;
   uint16_t  texturesMapDependsOnAllocated;
};

typedef int (*cce_rloadfun)(void *buffer, struct cce_buffer *info, char **names);
typedef char** (*cce_rstorefun)(void *buffer, struct cce_buffer *info);

CCE_API uint32_t cceRegisterMapCustomResourceCallback (cce_rloadfun onLoad, cce_dataparsefun onFree, cce_dataparsefun onCreate, cce_rstorefun onStore, size_t bufferSize);
CCE_API void cceSetMap2Dpath (const char *path);
CCE_API int  cceInitEngine2D (const char *gameINIpath);
CCE_API void cceSetTexturesPath (const char *path);
CCE_API void cceSetRenderingLayerMap2D (uint8_t layer, uint8_t mapLayer, struct cce_buffer *map);
CCE_API void cceRenderMap2D (void);
CCE_API void cceUpdateEngineMap2D (void);
CCE_API int  cceEngine2D (void);
CCE_API void cceSetLoadedMap2D (uint16_t number, struct cce_i32vec2 globalPosition);
CCE_API const char* cceGetResourcePath (void);
CCE_API uint16_t cceLoadTexture (char *path, uint8_t usersQuantity);
CCE_API struct cce_buffer* cceLoadMap2D (char *path);
CCE_API struct cce_buffer* cceLoadMap2Ddynamic (char *path);
CCE_API int  cceWriteMap2Ddynamic (struct cce_buffer *map, char *path);
CCE_API struct cce_buffer* cceCreateMap2Ddynamic (void);
CCE_API void cceSetCameraPosition (struct cce_i16vec2 position);
CCE_API void cceSetViewRotation (uint8_t normalizedAngle);
CCE_API void cceSetPixelsPerCoordinate (uint16_t k);

CCE_API struct cce_elementposition* cceGetElementsPosition (uint8_t layer, uint16_t positionID, uint16_t quantity, struct cce_buffer *map);
CCE_API int cceSetElementsPositionsUpdated (struct cce_elementpositionarray *elementPositions);
CCE_API struct cce_element* cceGetElements (uint16_t ID, uint16_t quantity, struct cce_buffer *map);
CCE_API int cceSetElementsUpdated (struct cce_renderinginfo *info);
CCE_API int cceSetRenderingLayersQuantity (uint8_t layersQuantity, struct cce_buffer *map);
CCE_API struct cce_renderinginfo* cceGetRenderingInfo (struct cce_buffer *map);
CCE_API struct cce_dynamicrenderinginfo* cceGetDynamicRenderingInfo (struct cce_buffer *map);
CCE_API struct cce_elementpositionarray* cceGetElementPositionArray (uint8_t layer, struct cce_buffer *map);
CCE_API void* cceGetResource (uint8_t resource, struct cce_buffer *map);

CCE_API extern const struct cce_u16vec2 *const cceTextureSize;

#define cceFreeMap2D(map)        cceFreeBuffer(map)
#define cceFreeMap2Ddynamic(map) cceFreeBuffer(map)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MAP2D_H
