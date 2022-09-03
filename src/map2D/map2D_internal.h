/*
    CoffeeChain - open source engine for making games.
    Copyright (C) 2020-2022 Andrey Givoronsky

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

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus


#include "../engine_common_internal.h"
#include "../../include/coffeechain/map2D/map2D.h"

#include "log.h"
#include <listlib.h>

#define CCE_GLOBAL_OFFSET_MASK 0x10

#define CCE_PROCESS_TEXTURES 0x010
#define CCE_PROCESS_UBO_ARRAY 0x020
#define CCE_PROCESS_NEAREST_MAPS 0x040
#define CCE_PROCESS_LOADEDMAP2D  0x080
#define CCE_PROCESS_GLOBALOFFSET 0x200
#define CCE_GLOBAL_OFFSET_CHANGED (CCE_PROCESS_NEAREST_MAPS | CCE_PROCESS_GLOBALOFFSET)

#define CCE_BASIC_ACTIONS_NOT_SET 0x100
#define CCE_INIT CCE_BASIC_ACTIONS_NOT_SET

struct RenderingData;

struct DelayedAction
{
   uint32_t actionID;
   uint32_t repeatsLeft;
   struct Timer timer;
};

struct LoadedTextures
{
   union 
   {
      char  name[16];
      char *path;
   };
   struct cce_u16vec2 size;
   uint8_t  dependantMapsQuantity;
   uint8_t  flags;
};

#define CCE_LOADEDTEXTURES_TOBELOADED 0x1u
#define CCE_LOADEDTEXTURES_PATH       0x2u

struct Map2Darray
{
   struct Map2D *main;
   struct Map2D **dependies;
};

struct Map2DElement
{
   struct cce_i32vec2 position;
   struct cce_u16vec2 size;
   struct Texture textureInfo;
   uint8_t transformGroups[4];     // 0 is no transformation is applied
   uint8_t textureOffsetGroups[2]; // 0 is texture (more precisely - texture piece) unchangeable
   uint8_t colorGroups[2];         // 0 is color unchangable
};

struct DynamicMap2DElement
{
   struct cce_i32vec2 position;
   struct cce_u16vec2 size;
   struct Texture textureInfo;
   uint16_t  transformGroupsQuantity;
   uint16_t  collisionGroupsQuantity;
   uint16_t *transformGroups;
   uint16_t *collisionGroups;
   uint16_t  textureElementReliesOn;
   uint8_t   visibleTransformGroups[4];
   uint8_t   textureOffsetGroups[2]; /* 0 is texture (more precisely - texture piece) unchangeable */
   uint8_t   colorGroups[2];         /* 0 is color unchangable */
   uint8_t   flags;                  /* 0x1 - isUsed, 0x2 - hasCollider, 0x4 - toBeProcessed, 0x8 - has2DElement, 0x10 - isGlobalOffset, 0x20 - isCurrentPosition */
};

struct DynamicCollisionGroup
{
   uint16_t group1;
   uint16_t group2;
   uint8_t  flags; /* 0x1 - busy, 0x2 - group1 is current Map2D's group, 0x4 - the same for group2 */
};

struct DynamicMap2D
{
   uint32_t elementsQuantity;
   uint32_t elementsQuantityAllocated;
   struct DynamicMap2DElement   *elements;
   
   uint16_t UBO_ID;
   uint16_t temporaryBools;
   
   uint16_t transformGroupsQuantity;
   uint16_t transformGroupsQuantityAllocated;
   struct DynamicElementGroup   *transformGroups;
   
   uint16_t collisionGroupsQuantity;
   uint16_t collisionGroupsQuantityAllocated;
   uint16_t collisionQuantity;
   uint16_t collisionQuantityAllocated;
   struct DynamicElementGroup   *collisionGroups; 
   struct DynamicCollisionGroup *collision;
   
   uint16_t timersQuantity;
   uint16_t timersQuantityAllocated;
   
   
   uint32_t logicQuantity;
   uint32_t logicQuantityAllocated;
   
   struct Timer                 *timers;
   struct ElementLogic          *logic;
   
   struct list                   delayedActions;
   
   uint32_t objectBufferAllocatedSpace; /* usually equals to elementsQuantityAllocated, or lower */
   
};

struct Map2D
{
   struct RenderingData  *data;
   uint32_t elementsQuantity;
   uint32_t collidersQuantity;
   struct Map2DCollider  *colliders;
   
   uint16_t transformGroupsQuantity;
   uint16_t collisionGroupsQuantity;
   uint16_t collisionQuantity;
   struct ElementGroup   *transformGroups;
   struct ElementGroup   *collisionGroups;
   struct CollisionGroup *collision;
   uint32_t logicQuantity;
   uint16_t timersQuantity;
   uint8_t  exitMapsQuantity;
   uint8_t  staticActionsQuantity;
   struct Timer          *timers;
   struct ElementLogic   *logic;
   struct ExitMap2D      *exitMaps;
   uint16_t              *texturesMapReliesOn;
   uint32_t              *staticActionIDs;
   uint32_t              *staticActionArgOffsets;
   cce_void              *staticActionArgs;
   struct list            delayedActions;

   uint16_t temporaryBools;
   uint16_t texturesMapReliesOnQuantity;
   uint16_t ID;
};

struct TransformationValues
{
   struct cce_i32vec2 move[256];
   struct cce_f32vec2 scale[256];
   struct cce_i32vec2 rotationOffset[256];
   float rotationAngles[256];
   struct cce_i16vec2 textureOffset[255];
   
};

extern void (**cce_actions)(void*);
extern void (**cce_endianSwapActions)(void*);
extern struct cce_i32vec2 cce__globalOffset;

void cce__baseActionsInit (struct DynamicMap2D *dynamic_map, struct UsedUBO *UBOs, const GLint *bufferUniformsOffsets,
                           const GLint *uniformLocations, GLuint shaderProgram, void (*setUniformBufferToDefault)(GLuint, GLint),
                           const GLint *uniformBufferSize, cce_flag *flags);
void cce__initMap2DLoaders (const cce_flag *flagsPointer);
void cce__setCurrentArrayOfMaps (const struct Map2Darray *maps);
void cce__beginBaseActions (struct Map2D *map);
void cce__endBaseActions (void);
void cce__endBaseActionsDynamicMap2D (void);

void cce__setAttribPointerVAO (void);
void cce__elementToMap2DElementVertices (struct Map2DElementVertices *buffer, int32_t x, int32_t y, uint16_t width, uint16_t height,
                                         uint8_t *moveGroups, uint8_t moveGroupsQuantity, uint8_t *extensionGroups, uint8_t extensionGroupsQuantity,
                                         uint8_t globalOffset, uint8_t rotationGroup, struct Texture *textureInfo, uint16_t textureID,
                                         uint8_t *textureOffsetGroups, uint8_t textureOffsetGroupsQuantity, uint8_t *colorGroups, uint8_t colorGroupsQuantity);
void cce__extendElementBufferIfNecessary (uint32_t minimalSize);


#define cce__map2DElementToMap2DElementVertices(buffer, element, moveGroups, extensionGroups, globalOffset) \
cce__elementToMap2DElementVertices(buffer, (element)->x, (element)->y, (element)->width, (element)->height, \
moveGroups, 4, extensionGroups, 4, globalOffset, (element)->rotateGroup, &((element)->textureInfo), (element)->textureInfo.ID, \
(element)->textureOffsetGroups, 4, (element)->colorGroups, 4)

#define cce__dynamicMap2DElementToMap2DElementVertices(buffer, element) \
cce__elementToMap2DElementVertices(buffer, (element)->x, (element)->y, (element)->width, (element)->height, \
(element)->visibleMoveGroups, 4, (element)->visibleExtensionGroups, 4, \
((element)->flags & CCE_GLOBAL_OFFSET_MASK) > 0, (element)->rotateGroup, &((element)->textureInfo), (element)->textureElementReliesOn, \
(element)->textureOffsetGroups, 4, (element)->colorGroups, 4)

cce_ubyte cce__checkCollision (const uint32_t *group1firstID, uint16_t groups1quantity, const uint32_t *group2firstID, uint16_t groups2quantity,
                               const cce_void *elements1, size_t element1size, const cce_void *elements2, size_t element2size);

cce_ubyte cce__checkCollisionWithOffset (const uint32_t *group1firstID, uint16_t groups1quantity, const uint32_t *group2firstID, uint16_t groups2quantity,
                                         const cce_void *elements1, size_t element1size, const struct cce_i32vec2 *elements1offset,
                                         const cce_void *elements2, size_t element2size, const struct cce_i32vec2 *elements2offset);
cce_ubyte cce__fourthLogicTypeFuncMap2D(uint16_t ID, va_list argp);
cce_ubyte cce__fourthLogicTypeFuncDynamicMap2D(uint16_t ID, va_list argp);
cce_ubyte cce__checkCollisionDynamicMap2DmultipleMaps (uint16_t ID, struct Map2D *map, struct Map2D **maps, size_t mapsQuantity, const struct cce_i32vec2 *mapOffsets, size_t mapOffsetsSize);
uint16_t cce__loadTexture (uint32_t ID);
void cce__processDynamicMap2DElements (void);
uint16_t* cce__loadTexturesMap2D (struct Map2DElement *elements, uint32_t elementsQuantity, uint16_t *texturesLoadedMapReliesOnQuantity);
void cce__releaseTextures (uint16_t *texturesMapReliesOn, uint16_t texturesMapReliesOnQuantity);
void cce__releaseTexture (uint16_t textureID);
void cce__initLogicMap2D (struct Map2D *map);
uint16_t cce__getFreeUBO (void);
void cce__releaseUBO (uint16_t ID);
void cce__releaseUnusedUBO (uint16_t ID);
void cce__allocateUBObuffers (uint16_t uboID, uint16_t moveGroupsQuantity, uint16_t extensionGroupsQuantity);
struct UsedUBO* cce__getFreeUBOdata (uint16_t ID);
struct DynamicMap2D* cce__initDynamicMap2D (GLuint EBO);
uint8_t cce__getDynamicElementFlags (uint16_t ID);
void cce__setToBeProcessedDynamicMap2D (void);
void cce__terminateDynamicMap2D (void);
void cce__terminateEngine2D (void);

/* Action is a function: void action (void *ptr) */
#define cce__processLogicMap2D(map) if ((map)->logicQuantity) cce__setCurrentTemporaryBools((map)->temporaryBools); \
cce__beginBaseActions(map); \
cce__processLogic((map)->logicQuantity, (map)->logic, (map)->timers, cce_actions, cce__fourthLogicTypeFuncMap2D, map); \
cce__endBaseActions()
#define cce__processLogicDynamicMap2D(dynamicMap, currentMap, cce__fourthLogicTypeFunc, ...) cce__setCurrentTemporaryBools((dynamicMap)->temporaryBools); cce__beginBaseActions(currentMap); \
cce__processLogic((dynamicMap)->logicQuantity, (dynamicMap)->logic, (dynamicMap)->timers, cce_actions, cce__fourthLogicTypeFunc, __VA_ARGS__); \
cce__endBaseActions(); \
cce__endBaseActionsDynamicMap2D()

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // MAP2D_INTERNAL_H
