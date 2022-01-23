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

#ifndef MAP2D_H
#define MAP2D_H
#include <stdint.h>
#include <stdio.h>
#include "../engine_common.h"

#ifdef __cplusplus
extern C:
{
#endif // __cplusplus


#define CCE_INCORRECT_ENUM 1
#define CCE_ATTEMPT_TO_OVERRIDE_DEFAULT_ELEMENT 2
#define CCE_ELEMENT_DOES_NOT_EXIST 3
#define CCE_OUT_OF_BOUNDS 4


#define CCE_DEFAULT                            0x000
#define CCE_RENDER_CLOSEST_MAP                 CCE_DEFAULT
#define CCE_RENDER_ONLY_CURRENT_MAP            0x001
#define CCE_RENDER_ALL_LOADED_MAPS             0x002
#define CCE_PROCESS_LOGIC_ONLY_FOR_CURRENT_MAP CCE_DEFAULT
#define CCE_PROCESS_LOGIC_FOR_CLOSEST_MAP      0x004
#define CCE_PROCESS_LOGIC_FOR_ALL_MAPS         0x008

typedef uint_fast32_t cce_flag;

#define CCE_MOVE_GROUP 0x10
#define CCE_EXTENSION_GROUP 0x11
#define CCE_COLLISION_GROUP 0x12

#define CCE_COLLISION_LOGIC_ELEMENT   0xB

struct ExitMap2D
{
   int32_t xOffset;
   int32_t yOffset;
   int32_t aBorder;
   int32_t b1Border;
   int32_t b2Border;
   uint32_t ID;
   uint8_t flags; // 0x1 - a is x (otherwise a is y), 0x2 - b is to the south/west from globalOffset
};

struct Map2DCollider
{
   int32_t x;
   int32_t y;
   uint16_t width;
   uint16_t height;
};

struct Map2DElement
{
   int32_t  x;
   int32_t  y;
   uint16_t width;
   uint16_t height;
   struct Texture textureInfo;
   int8_t layer;
   // Vertex shader
// uint8_t moveGroup;          // 0 is unmovable
// uint8_t extensionGroup;     // 0 is unscalable
   // Geometry shader
   uint8_t rotateGroup;        // 0 is unrotatable
   // Fragment shader
   uint8_t textureOffsetGroup; // 0 is texture (more precisely - texture piece) unchangeable
   uint8_t colorGroup;         // 0 is color unchangable
};

struct Map2DElementDev
{
   int32_t  x;
   int32_t  y;
   uint16_t width;
   uint16_t height;
   struct Texture textureInfo;
   int8_t layer;
   // Vertex shader
   uint8_t moveGroup;          // 0 is unmovable
   uint8_t isGlobalOffset;
   uint8_t extensionGroup;     // 0 is unscalable
   // Geometry shader
   uint8_t rotateGroup;        // 0 is unrotatable
   // Fragment shader
   uint8_t textureOffsetGroup; // 0 is texture (more precisely - texture piece) unchangeable
   uint8_t colorGroup;         // 0 is color unchangable
};

struct Map2D
{
   struct Map2DCollider  *colliders;
   struct ElementGroup   *moveGroups;
   struct ElementGroup   *extensionGroups;
   struct ElementGroup   *collisionGroups;
   struct CollisionGroup *collision;
   double                *collisionCache;
   struct Timer          *timers;
   struct ElementLogic   *logic;
   struct ExitMap2D      *exitMaps;
   uint16_t              *texturesMapReliesOn;
   
   uint32_t VAO;
   uint32_t VBO;
   uint32_t elementsQuantity;
   uint32_t collidersQuantity;
   uint32_t logicQuantity;
   
   uint16_t flags; // 0x1 UBO was used
   uint16_t ID;
   uint16_t UBO_ID;
   uint16_t moveGroupsQuantity;
   uint16_t extensionGroupsQuantity;
   uint16_t collisionGroupsQuantity;
   uint16_t collisionQuantity;
   uint16_t timersQuantity;
   uint16_t temporaryBools;
   uint16_t texturesMapReliesOnQuantity;
   
   uint8_t  exitMapsQuantity;
};

struct Map2Ddev
{
   uint16_t ID;
   uint32_t               elementsQuantity;
   uint32_t               elementsWithoutColliderQuantity;
   struct Map2DElement   *elements;
   uint16_t               moveGroupsQuantity;
   struct ElementGroup   *moveGroups;
   uint16_t               extensionGroupsQuantity;
   struct ElementGroup   *extensionGroups;
   uint32_t               collidersQuantity;
   struct Map2DCollider  *colliders;
   uint16_t               collisionGroupsQuantity;
   struct ElementGroup   *collisionGroups;
   uint16_t               collisionQuantity;
   struct CollisionGroup *collision;
   uint16_t               timersQuantity;
   float                 *delaysOfTimers;
   uint32_t               logicQuantity;
   struct ElementLogic   *logic;
   uint8_t                actionsQuantity;
   uint32_t              *actionIDs;
   uint32_t              *actionsArgOffsets;
   cce_void              *actionsArg;
   uint8_t                exitMapsQuantity;
   struct ExitMap2D      *exitMaps;
};

CCE_PUBLIC_OPTIONS void cceSetGridMultiplier (float multiplier);
CCE_PUBLIC_OPTIONS void cceSetMap2Dpath (const char *path);
CCE_PUBLIC_OPTIONS void cceFreeMap2D (struct Map2D *map);
CCE_PUBLIC_OPTIONS struct Map2D* cceLoadMap2D (uint16_t number);
CCE_PUBLIC_OPTIONS struct Map2D* cceMap2DdevToMap2D (struct Map2Ddev *mapdev);
CCE_PUBLIC_OPTIONS void cceFreeMap2Ddev (struct Map2Ddev *map);
CCE_PUBLIC_OPTIONS struct Map2Ddev* cceLoadMap2Ddev (uint16_t number);
CCE_PUBLIC_OPTIONS int cceWriteMap2Ddev (struct Map2Ddev *map, void (*writeFunc)(FILE*));
CCE_PUBLIC_OPTIONS int cceInitEngine2D (uint16_t globalBoolsQuantity, uint32_t textureMaxWidth, uint32_t textureMaxHeight,
                      const char *windowLabel, const char *resourcePath);
CCE_PUBLIC_OPTIONS void cceSetFlags2D (cce_flag flags);
CCE_PUBLIC_OPTIONS uint8_t cceRegisterAction (uint32_t ID, void (*action)(void*));
CCE_PUBLIC_OPTIONS void cceSetTexturesPath (const char *path);
CCE_PUBLIC_OPTIONS int cceEngine2D (void);

// dynamicMap2D

CCE_PUBLIC_OPTIONS uint8_t cceGetGroupValueDynamicMap2D (cce_enum group_type, uint16_t ID, struct cce_ivec2 *variable);
CCE_PUBLIC_OPTIONS struct ElementGroup cceGetGroupDataDynamicMap2D (cce_enum group_type, uint16_t ID);
CCE_PUBLIC_OPTIONS struct Map2DCollider cceGetColliderDataDynamicMap2D (uint32_t ID);
CCE_PUBLIC_OPTIONS struct CollisionGroup cceGetCollisionDataDynamicMap2D (uint16_t ID);
CCE_PUBLIC_OPTIONS uint8_t cceCreateGroupDynamicMap2D (cce_enum group_type, uint16_t elementsQuantity, uint32_t *elementIDs, uint16_t *emptyGroupID);
CCE_PUBLIC_OPTIONS uint8_t cceAddElementInGroupDynamicMap2D (cce_enum group_type, uint16_t ID, uint32_t elementID);
CCE_PUBLIC_OPTIONS void cceReplaceMap2DElementDynamicMap2D (struct Map2DElementDev *element, uint32_t ID, uint8_t hasCollider);
CCE_PUBLIC_OPTIONS uint32_t cceCreateMap2DElementDynamicMap2D (struct Map2DElementDev *element, uint8_t hasCollider);
CCE_PUBLIC_OPTIONS uint8_t cceDeleteElementFromGroupDynamicMap2D (cce_enum group_type, uint16_t ID, uint32_t elementID);
CCE_PUBLIC_OPTIONS void cceDeleteMap2DElementDynamicMap2D (uint32_t ID);
CCE_PUBLIC_OPTIONS uint8_t cceDeleteGroupDynamicMap2D (cce_enum group_type, uint16_t ID);
//CCE_PUBLIC_OPTIONS void cceUpdateColliderDynamicMap2D (struct Map2DCollider *collider, uint32_t ID);
CCE_PUBLIC_OPTIONS void cceReplaceColliderDynamicMap2D (struct Map2DCollider *collider, uint32_t ID);
CCE_PUBLIC_OPTIONS void cceUpdateCollisionDynamicMap2D (uint16_t ID, uint16_t group1ID, cce_ubyte isGroup1BelongsToCurrentMap2D,
                                               uint16_t group2ID, cce_ubyte isGroup2BelongsToCurrentMap2D);
CCE_PUBLIC_OPTIONS uint16_t cceCreateCollisionDynamicMap2D (uint16_t group1ID, cce_ubyte isGroup1BelongsToCurrentMap2D, 
                                      uint16_t group2ID, cce_ubyte isGroup2BelongsToCurrentMap2D);
CCE_PUBLIC_OPTIONS void cceDeleteCollisionDynamicMap2D (uint16_t ID);
CCE_PUBLIC_OPTIONS void cceStartTimerDynamicMap2D (uint16_t ID);
CCE_PUBLIC_OPTIONS void cceSetTimerDelayDynamicMap2D (uint16_t ID, float delay);
CCE_PUBLIC_OPTIONS uint16_t cceCreateTimerDynamicMap2D (float delay);
CCE_PUBLIC_OPTIONS struct Timer cceGetTimerDynamicMap2D (uint16_t ID);
CCE_PUBLIC_OPTIONS void cceUpdateLogicElementsByTruthTableDynamicMap2D (const uint16_t ID, const uint8_t logicElementsQuantity, const uint16_t *const logicElements, const cce_enum *const logicElementTypes, const uint_fast16_t *const truthTable);
CCE_PUBLIC_OPTIONS uint8_t cceUpdateLogicElementsByBooleanExpressionDynamicMap2D (const uint16_t ID, const uint16_t *const logicElements, const cce_enum *const logicElementTypes, const char *const booleanExpression);
CCE_PUBLIC_OPTIONS void cceUpdateLogicActionsDynamicMap2D (const uint16_t ID, const uint8_t actionsQuantity, uint32_t *actionIDs, const void **actionArgs, const uint32_t *const actionArgSizes);
CCE_PUBLIC_OPTIONS uint16_t cceCreateLogicDynamicMap2D (void);

#define cceCheckCollisionMap2D(element1, element2) cceCheckCollision((element1)->x, (element1)->y, (element1)->width, (element1)->height, (element2)->x, (element2)->y, (element2)->width, (element2)->height)
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MAP2D_H
