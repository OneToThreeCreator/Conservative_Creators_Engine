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

#ifndef MAP2D_INTERNAL_H
#define MAP2D_INTERNAL_H

#include "map2D.h"
#include "../engine_common_internal.h"
#include "log.h"

#define CCE_GLOBAL_OFFSET_MASK 0x10

#ifdef NDEBUG
#define GL_CHECK_ERRORS
#else
#define GL_CHECK_ERRORS cce__openGLErrorPrint(glGetError(), __LINE__, __FILE__)
#endif

struct LoadedTextures
{
   uint32_t ID; /* 0 is invalid */
   uint8_t  dependantMapsQuantity;
   uint8_t  flags; /* 0x80 - to be loaded, 0x40 - after that point there's no busy LoadedTextures */
};

struct Map2Darray
{
   struct Map2D *main;
   struct Map2D **dependies;
};

struct UsedUBO
{
   uint32_t UBO;
   uint8_t flags; /* 0x1 - used, 0x2 - to be cleared; */
};

struct DynamicMap2DElement
{
   int32_t  x;
   int32_t  y;
   uint16_t width;
   uint16_t height;
   struct Texture textureInfo;
   uint16_t  moveGroupsQuantity;
   uint16_t *moveGroups;
   uint16_t  extensionGroupsQuantity;
   uint16_t *extensionGroups;
   uint16_t  collisionGroupsQuantity;
   uint16_t *collisionGroups;
   union
   {
      struct
      {
         // Fragment shader
         uint8_t textureOffsetGroup; /* 0 is texture (more precisely - texture piece) unchangeable */
         uint8_t colorGroup;         /* 0 is color unchangable */
      };
      struct
      {
         uint16_t textureElementReliesOn;
      };
   };
   uint8_t flags;              /* 0x1 - isUsed, 0x2 - hasCollider, 0x4 - toBeProcessed, 0x8 - has2DElement, 0x10 - isGlobalOffset */
   int8_t layer;
   // Vertex shader
   uint8_t moveGroup;          /* 0 is unmovable */
   uint8_t extensionGroup;     /* 0 is unscalable */
   // Geometry shader
   uint8_t rotateGroup;        /* 0 is unrotatable */
};

struct DynamicElementGroup
{
   uint32_t *elementIDs;
   uint16_t  elementsQuantity;
   uint16_t  elementsAllocatedQuantity;
};

struct DynamicCollisionGroup
{
   uint16_t group1;
   uint16_t group2;
   uint8_t  flags; /* 0x1 - busy, 0x2 - group1 is current Map2D's group, 0x4 - the same for group2 */
};

struct DynamicMap2D
{
   struct DynamicMap2DElement   *elements;
   struct DynamicElementGroup   *moveGroups;
   struct cce_ivec2             *moveGroupValues;
   
   struct DynamicElementGroup   *extensionGroups;
   struct cce_ivec2             *extensionGroupValues;
   struct DynamicElementGroup   *collisionGroups; 
   struct DynamicCollisionGroup *collision;
   struct Timer                 *timers;
   struct ElementLogic          *logic;
   
   uint32_t VAO;
   uint32_t VBO;
   uint32_t objectBufferAllocatedSpace; /* usually equals to elementsAllocatedQuantity, or lower */
   uint32_t elementsQuantity;
   uint32_t elementsAllocatedQuantity;
   uint32_t logicQuantity;
   uint32_t logicAllocatedQuantity;
   
   uint16_t UBO_ID;
   uint16_t moveGroupsQuantity;
   uint16_t moveGroupsAllocatedQuantity;
   uint16_t extensionGroupsQuantity;
   uint16_t extensionGroupsAllocatedQuantity;
   uint16_t collisionGroupsQuantity;
   uint16_t collisionGroupsAllocatedQuantity;
   uint16_t collisionQuantity;
   uint16_t collisionAllocatedQuantity;
   uint16_t timersQuantity;
   uint16_t timersAllocatedQuantity;
   uint16_t temporaryBools;
};

char* cce__createNewPathFromOldPath (const char *const oldPath, const char *const appendPath, size_t freeSpaceToLeave);

void cce__baseActionsInit (const struct DynamicMap2D *dynamic_map, struct UsedUBO *UBOs, const GLint *bufferUniformsOffsets, 
                      const GLint *uniformLocations, GLuint shaderProgram, void (*setUniformBufferToDefault)(GLuint, GLint));
void cce__initMap2DLoaders (void (***doAction)(void*));
void cce__setCurrentArrayOfMaps (const struct Map2Darray *maps);
void cce__beginBaseActions (const struct Map2D *map);
void cce__endBaseActions (void);

cce_ubyte cce__fourthLogicTypeFuncMap2D(uint16_t ID, va_list argp);
cce_ubyte cce__fourthLogicTypeFuncDynamicMap2D(uint16_t ID, va_list argp);
uint16_t cce__loadTextureDynamicMap2D (struct DynamicMap2DElement *element);
void cce__processDynamicMap2DElements (void);
uint16_t* cce__loadTexturesMap2D (struct Map2DElement *elements, uint32_t elementsQuantity, uint16_t *texturesLoadedMapReliesOnQuantity);
void cce__releaseTextures (uint16_t *texturesMapReliesOn, uint16_t texturesMapReliesOnQuantity);
uint16_t cce__getFreeUBO (void);
void cce__releaseUBO (uint16_t ID);
void cce__releaseUnusedUBO (uint16_t ID);
const struct DynamicMap2D* cce__initDynamicMap2D (void);
void cce__terminateDynamicMap2D (void);
void cce__terminateEngine2D (void);

/* Action is a function: void action (void *ptr) */
#define processLogicMap2D(map) if (map->logicQuantity) cce__setCurrentTemporaryBools(map->temporaryBools); \
cce__beginBaseActions(map); \
cce__processLogic(map->logicQuantity, map->logic, map->timers, cce_actions, cce__fourthLogicTypeFuncMap2D, map); \
cce__endBaseActions()
#define processLogicDynamicMap2D(dynamicMap, currentMap) cce__setCurrentTemporaryBools(dynamicMap->temporaryBools); cce__beginBaseActions(currentMap); \
cce__processLogic(dynamicMap->logicQuantity, dynamicMap->logic, dynamicMap->timers, cce_actions, cce__fourthLogicTypeFuncDynamicMap2D, currentMap); \
cce__endBaseActions()

#define CCE_COLORGROUP_OFFSET 0u
#define CCE_MOVEGROUP_OFFSET 1u
#define CCE_EXTENSIONGROUP_OFFSET 2u
#define CCE_TEXTUREOFFSET_OFFSET 3u
#define CCE_ROTATIONOFFSET_OFFSET 4u
#define CCE_ROTATEANGLESIN_OFFSET 5u
#define CCE_ROTATEANGLECOS_OFFSET 6u

#endif // MAP2D_INTERNAL_H