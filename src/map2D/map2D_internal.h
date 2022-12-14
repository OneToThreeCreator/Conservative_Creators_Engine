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

#ifndef MAP2D_INTERNAL_H
#define MAP2D_INTERNAL_H

#include "../../include/cce/engine_common.h"
#include "../../include/cce/utils.h"
#include "../../include/cce/map2D/map2D.h"
#include "../../include/cce/os_interaction.h"

#include "log.h"
#include <listlib.h>

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define CCE_GLOBAL_OFFSET_MASK 0x10

#define CCE_PROCESS_TEXTURES 0x010
#define CCE_PROCESS_NEAREST_MAPS 0x020
#define CCE_PROCESS_TIMERS 0x040
#define CCE_GLOBAL_OFFSET_CHANGED (CCE_PROCESS_NEAREST_MAPS | CCE_PROCESS_GLOBALOFFSET)

#define CCE_BASIC_ACTIONS_NOT_SET 0x100
#define CCE_INIT CCE_BASIC_ACTIONS_NOT_SET

#define CCE_STORE_COLOR_IN (CCE_STORE_COLOR_IN_RGB | CCE_STORE_COLOR_IN_HSV | CCE_STORE_COLOR_IN_HSL | CCE_STORE_COLOR_IN_HCL)

#ifndef CCE_PATH_RESERVED
#define CCE_PATH_RESERVED 16
#endif

struct RenderingData;

union Collider2D
{
   struct cce_collider_rect2D_16_8 rectangle;
   struct cce_collider_cir2D_16_16 circle;
};

#define CCE_LOADEDTEXTURES_TOBELOADED 0x1u

struct LoadedTextures
{
   char              *path;
   struct cce_u16vec2 size;
   uint8_t            dependantMapsQuantity;
   uint8_t            flags;
};

struct Map2Darray
{
   struct cce_buffer  *main;
   struct cce_buffer **dependant;
};

struct Map2DElementPosition
{
   struct cce_i16vec2 position;
   uint8_t            rotation;
   uint8_t            textureDataOffsetGroup;
   uint16_t           textureDataID;
};

CCE_ARRAY_STRUCT(Map2DElementPositionArray, struct Map2DElementPosition, uint32_t);

struct RenderingInfo
{
   struct RenderingData *data;
};

/*
struct cce_texture2D
{
   struct cce_u16vec2 position;
   struct cce_u16vec2 size;
   uint16_t ID; // 0 is no texture
};
*/

struct ElementInfo
{
   union
   {
      struct cce_u16vec2 texturePosition;
      struct cce_u8vec4  rgba;
   } data;
   struct cce_u16vec2 size;
   uint16_t textureID;
};

struct DynamicRenderingInfo
{
   struct RenderingData             *data;
   struct Map2DElementPositionArray *elements;
   struct ElementInfo               *info;
   uint16_t                          infoQuantity;
   uint8_t                           layersQuantity;
   uint16_t                          infoAllocated;
};

struct CollisionInfo
{
   union Collider2D           *colliders;
   struct ElementGroup        *transformGroups; // Groups can be used to transform colliders (Rotation of colliders is unsupported, use circle colliders instead)
   struct ElementGroup        *collisionGroups;
   struct CollisionGroup      *collision;
   uint64_t                   *collisionCache;  // Timestamp with last bit used as actual cache. Compared with lastTransformationTime
   
   uint32_t                    collidersQuantity;
   uint16_t                    transformGroupsQuantity;
   uint16_t                    collisionGroupsQuantity;
   uint16_t                    collisionQuantity;
};

struct DynamicCollisionInfo
{
   union Collider2D           *colliders;
   struct ElementGroup        *transformGroups; // Groups can be used to transform colliders (Rotation of colliders is unsupported, use circle colliders instead)
   struct ElementGroup        *collisionGroups;
   struct CollisionGroup      *collision;
   uint64_t                   *collisionCache; // Timestamp with last bit used as actual cache. Compared with lastTransformationTime
   
   uint32_t                    collidersQuantity;
   uint16_t                    transformGroupsQuantity;
   uint16_t                    collisionGroupsQuantity;
   uint16_t                    collisionQuantity;
   uint16_t                    transformGroupsAllocated;
   uint32_t                    collidersAllocated;
   uint16_t                    collisionGroupsAllocated;
   uint16_t                    collisionAllocated;
};

struct ActionInfo
{
   struct Action        *onFreeActions;
   uint16_t             *onFreeActionsSizes;
   uint16_t              onFreeActionsQuantity;
};

struct DynamicActionInfo
{
   struct Action        *onFreeActions;
   uint16_t             *onFreeActionsSizes;
   uint16_t              onFreeActionsQuantity;
   uint16_t              onLoadActionsQuantity;
   uint16_t              onLoadActionsSizesAllocated;
   uint16_t              onFreeActionsSizesAllocated;
   struct Action        *onLoadActions;
   uint16_t             *onLoadActionsSizes;
};

struct ResourceInfo
{
   void *resourceData;
   uint8_t resourcesQuantity;
};

struct UsedTexturesInfo
{
   uint16_t *texturesMapDependsOn;
   uint16_t  texturesMapDependsOnQuantity;
   uint16_t  texturesMapDependsOnAllocated;
};

struct TransformationValues
{
   // Last group is global offset
   struct cce_i16vec2 move[256];
   struct cce_i16vec2 scale[255];
   struct cce_i16vec2 rotationOffset[255];
   uint8_t            rotationAngles[255];
   struct cce_i16vec2 textureOffset[255];
   union  cce_color   color[255];
};

struct RendereringFunctions
{
   void (*drawMap2D)(struct RenderingData**, uint32_t);
   struct RenderingData* (*map2DElementsToRenderingBuffer)(const struct Map2DElementPositionArray *elements, uint8_t layersQuantity,
                                                           const struct ElementInfo *elementInfo, uint16_t elementInfoQuantity);
   struct ElementInfo* (*renderingBufferToMap2DElements) (struct RenderingData *data);
   size_t (*getRenderingDataSize) (void);
   struct RenderingData* (*createElementsBuffer)(size_t);
   struct RenderingData* (*resizeElementsBuffer) (struct RenderingData*, size_t);
   void (*deleteMap2DRenderingBuffer)(struct RenderingData*);
   void (*loadTexture)(void*, uint16_t, uint16_t, uint16_t);
   void (*reallocateTextureArray)(uint16_t);
   void (*moveTextureFromOldArray)(uint16_t);
   void (*removeOldArray)(void);
   void (*terminateMap2DRenderer)(void);
};

extern struct TransformationValues cce__groupsCache;
extern struct TransformationValues cce__transformations;
extern cce_flag cce__map2Dflags;
extern struct RendereringFunctions cce__renderingFunctions;
extern struct LoadedTextures *cce__textures;
extern uint16_t cce__texturesQuantity;
extern uint16_t cce__staticMapFunctionSet, cce__dynamicMapFunctionSet;

#define cce__drawMap2D(maps, mapsQuantity) cce__renderingFunctions.drawMap2D(maps, mapsQuantity)
#define cce__map2DElementsToRenderingBuffer(elements, layersQuantity, textureInfo, textureInfoQuantity) \
cce__renderingFunctions.map2DElementsToRenderingBuffer(elements, layersQuantity, textureInfo, textureInfoQuantity)
#define cce__renderingBufferToMap2DElements(data) cce__renderingFunctions.renderingBufferToMap2DElements(data)
#define cce__getRenderingDataSize() cce__renderingFunctions.getRenderingDataSize()
#define cce__createElementsBuffer(size) cce__renderingFunctions.createElementsBuffer(size)
#define cce__resizeElementsBuffer(data, size) cce__renderingFunctions.resizeElementsBuffer(data, size)
#define cce__deleteMap2DRenderingBuffer(data) cce__renderingFunctions.deleteMap2DRenderingBuffer(data)
#define cce__loadTexture(data, width, height, textureID) cce__renderingFunctions.loadTexture(data, width, height, textureID)
#define cce__reallocateTextureArray(size) cce__renderingFunctions.reallocateTextureArray(size)
#define cce__moveTextureFromOldArray(texture) cce__renderingFunctions.moveTextureFromOldArray(texture)
#define cce__removeOldArray() cce__renderingFunctions.removeOldArray()
#define cce__terminateMap2DRenderer() cce__renderingFunctions.terminateMap2DRenderer()

#define CCE_SET_PATH(pathVar, lengthVar, newPath) \
newPath = cceGetAbsolutePath(newPath, CCE_PATH_RESERVED + 1); \
if (newPath == NULL) \
   return; \
pathVar = (char*)newPath; \
lengthVar = strlen(pathVar); \
lengthVar += cceIsPathDelimiter(pathVar[lengthVar]); \
pathVar[lengthVar - 1] = cceNativePathDelimiter; \
pathVar[lengthVar] = '\0'

void cce__initMap2DLoaders (void);
void cce__terminateMap2DLoaders (void);
void cce__setCurrentArrayOfMaps (const struct Map2Darray *maps);

void cce__setAttribPointerVAO (void);
void cce__extendElementBufferIfNecessary (uint32_t minimalSize);

void cce__actionsInit (void);
void cce__actionsTerminate (void);
void cce__releaseTexture (uint16_t textureID);
uint8_t cce__getDynamicElementFlags (uint16_t ID);
void cce__setToBeProcessedDynamicMap2D (void);
void cce__terminateDynamicMap2D (void);
void cce__terminateEngine2D (void);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // MAP2D_INTERNAL_H
