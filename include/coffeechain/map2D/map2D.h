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

#ifndef MAP2D_H
#define MAP2D_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#ifndef stdout
struct __FILE;
typedef struct __FILE FILE;
#endif

#include <stdint.h>
#include "../engine_common.h"

#define CCE_BASIC_ACTIONS_QUANTITY 16u

#define CCE_INCORRECT_ENUM 1
#define CCE_ATTEMPT_TO_OVERRIDE_DEFAULT_ELEMENT 2
#define CCE_ELEMENT_DOES_NOT_EXIST 3
#define CCE_OUT_OF_BOUNDS 4


#define CCE_RENDER_ONLY_CURRENT_MAP            0x001
#define CCE_RENDER_VISIBLE_MAPS                0x002
#define CCE_RENDER_ALL_LOADED_MAPS             0x003
#define CCE_PROCESS_LOGIC_ONLY_FOR_CURRENT_MAP 0x000 // default
#define CCE_DONT_PROCESS_LOGIC                 0x004
#define CCE_PROCESS_LOGIC_FOR_VISIBLE_MAPS     0x008
#define CCE_PROCESS_LOGIC_FOR_ALL_MAPS         0x00C
#define CCE_FORCE_INITIALIZE_MAP_ONLOAD        0x010

#define CCE_DEFAULT 0

typedef uint_fast32_t cce_flag;

#define CCE_MOVE_GROUP 0x10
#define CCE_EXTENSION_GROUP 0x11
#define CCE_COLLISION_GROUP 0x12

#define CCE_COLLISION_LOGIC_ELEMENT   0xB

#define CCE_COLLIDER              0x20
#define CCE_ELEMENT_WITHOUT_COLLIDER 0x40
#define CCE_ELEMENT_WITH_COLLIDER (CCE_COLLIDER | CCE_ELEMENT_WITHOUT_COLLIDER)

#define CCE_POSITION_IS_NOT_CURRENT 0x1
#define CCE_TEXTUREID_IS_NOT_IMAGEID 0x2

struct ExitMap2D
{
   uint32_t ID;
   int32_t xOffset;
   int32_t yOffset;
   int32_t aBorder;
   int32_t b1Border;
   int32_t b2Border;
   uint8_t flags; // 0x1 - a is x (otherwise a is y), 0x2 - b is to the south/west from globalOffset 0
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
   uint8_t textureOffsetGroups[4]; // 0 is texture (more precisely - texture piece) unchangeable
   uint8_t colorGroups[4];         // 0 is color unchangable
   uint8_t rotateGroup;            // 0 is unrotatable
};

struct Map2DElementDev
{
   int32_t  x;
   int32_t  y;
   uint16_t width;
   uint16_t height;
   struct Texture textureInfo;
   uint8_t moveGroups[4];          // 0 is unmovable
   uint8_t extensionGroups[4];     // 0 is unscalable
   uint8_t textureOffsetGroups[4]; // 0 is texture (more precisely - texture piece) unchangeable
   uint8_t colorGroups[4];         // 0 is color unchangable
   uint8_t rotateGroup;            // 0 is unrotatable
   uint8_t isGlobalOffset;
};

struct DynamicElementGroup
{
   uint32_t *elements;
   uint16_t  elementsQuantity;
   uint16_t  elementsQuantityAllocated;
};

/* Map2D intended for editing.
   All fields with QuantityAllocated can be used as CCE_ARRAY (from utils.h)
*/
struct Map2Ddev
{
   uint16_t                    ID;
   uint32_t                    elementsQuantity;
   uint32_t                    elementsWithoutColliderQuantity;
   uint32_t                    elementsQuantityAllocated;
   struct Map2DElement        *elements;
   uint16_t                    moveGroupsQuantity;
   uint16_t                    moveGroupsQuantityAllocated;
   struct DynamicElementGroup *moveGroups;
   uint16_t                    extensionGroupsQuantity;
   uint16_t                    extensionGroupsQuantityAllocated;
   struct DynamicElementGroup *extensionGroups;
   uint32_t                    collidersQuantity;
   uint32_t                    collidersQuantityAllocated;
   struct Map2DCollider       *colliders;
   uint16_t                    collisionGroupsQuantity;
   uint16_t                    collisionGroupsQuantityAllocated;
   struct DynamicElementGroup *collisionGroups;
   uint16_t                    collisionQuantity;
   uint16_t                    collisionQuantityAllocated;
   struct CollisionGroup      *collision;
   uint16_t                    timersQuantity;
   uint16_t                    timersQuantityAllocated;
   float                      *timers;
   uint32_t                    logicQuantity;
   uint32_t                    logicQuantityAllocated;
   struct ElementLogic        *logic;
   uint8_t                     actionsQuantity;
   uint32_t                   *actionIDs;
   uint32_t                   *actionsArgOffsets;
   cce_void                   *actionsArg;
   uint8_t                     exitMapsQuantity;
   uint8_t                     exitMapsQuantityAllocated;
   struct ExitMap2D           *exitMaps;
};

struct Map2D;

CCE_PUBLIC_OPTIONS void cceSetGridMultiplierMap2D (float multiplier);
CCE_PUBLIC_OPTIONS void cceSetMap2Dpath (const char *path);
CCE_PUBLIC_OPTIONS void cceFreeMap2D (struct Map2D *map);
CCE_PUBLIC_OPTIONS struct Map2D* cceLoadMap2D (uint16_t number);
CCE_PUBLIC_OPTIONS struct Map2D* cceMap2DdevToMap2D (struct Map2Ddev *mapdev);
CCE_PUBLIC_OPTIONS void cceFreeMap2Ddev (struct Map2Ddev *map);
CCE_PUBLIC_OPTIONS struct Map2Ddev* cceLoadMap2Ddev (uint16_t number);
CCE_PUBLIC_OPTIONS int cceWriteMap2Ddev (struct Map2Ddev *map, void (*writeFunc)(FILE*));
CCE_PUBLIC_OPTIONS int cceInitEngine2D (uint16_t globalBoolsQuantity, uint32_t textureMaxWidth, uint32_t textureMaxHeight,
                                        const char *windowLabel, const char *resourcePath, cce_flag flags);
CCE_PUBLIC_OPTIONS uint8_t cceRegisterAction (uint32_t ID, void (*action)(void*), void (*endianSwap)(void*));
CCE_PUBLIC_OPTIONS void cceSetTexturesPath (const char *path);
CCE_PUBLIC_OPTIONS int cceEngine2D (void);
CCE_PUBLIC_OPTIONS void cceSetLoadedMap2D (uint16_t number, struct cce_i32vec2 globalPosition);
CCE_PUBLIC_OPTIONS extern const uint16_t *const cceLoadedMap2Dnumber;
CCE_PUBLIC_OPTIONS extern const struct cce_u32vec2 *cceTextureSize;
CCE_PUBLIC_OPTIONS const char* cceGetResourcePath (void);
CCE_PUBLIC_OPTIONS uint16_t cceLoadTexture (char *path);

// dynamicMap2D

CCE_PUBLIC_OPTIONS uint8_t cceGetGroupValueDynamicMap2D (cce_enum group_type, uint16_t ID, struct cce_i32vec2 *variable);
CCE_PUBLIC_OPTIONS struct ElementGroup cceGetGroupDataDynamicMap2D (cce_enum group_type, uint16_t ID);
CCE_PUBLIC_OPTIONS struct Map2DCollider cceGetColliderDataDynamicMap2D (uint32_t ID);
CCE_PUBLIC_OPTIONS struct CollisionGroup cceGetCollisionDataDynamicMap2D (uint16_t ID);
CCE_PUBLIC_OPTIONS uint16_t cceCreateGroupDynamicMap2D (cce_enum group_type, uint16_t elementsQuantity, uint32_t *elementIDs);
CCE_PUBLIC_OPTIONS uint8_t cceAddElementInGroupDynamicMap2D (cce_enum group_type, uint16_t ID, uint32_t elementID);
CCE_PUBLIC_OPTIONS uint8_t cceAddElementInGroupVisibleDynamicMap2D (cce_enum group_type, uint16_t ID, uint32_t elementID);
CCE_PUBLIC_OPTIONS void cceReplaceMap2DElementDynamicMap2D (struct Map2DElementDev *element, uint32_t ID, uint8_t hasCollider, uint8_t isCurrentPosition);
CCE_PUBLIC_OPTIONS uint32_t cceCreateMap2DElementDynamicMap2D (struct Map2DElementDev *element, cce_enum elementType, uint8_t isCurrentPosition);
CCE_PUBLIC_OPTIONS uint32_t* cceCreateMap2DElementsDynamicMap2D (struct Map2DElementDev *elements, uint32_t elementsQuantity, cce_enum elementType, uint8_t isCurrentPosition);
CCE_PUBLIC_OPTIONS uint8_t cceDeleteGroupVisibilityFromElementDynamicMap2D (cce_enum group_type, uint16_t ID, uint32_t elementID);
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
#define cceCheckCollisionMap2DWithOffset(element1, element2, offset) \
cceCheckCollision((element1)->x, (element1)->y, (element1)->width, (element1)->height, (element2)->x + (offset)->x, (element2)->y + (offset)->y, (element2)->width, (element2)->height)
#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MAP2D_H
