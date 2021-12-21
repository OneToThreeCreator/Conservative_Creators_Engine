#ifndef MAP2D_H
#define MAP2D_H
#include <stdint.h>
#include <stdio.h>
#include "../engine_common.h"

#ifdef __cplusplus
extern C:
{
#endif // __cplusplus

#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__TOS_WIN__) || defined(__WINDOWS__) || \
    defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN__)
#include "cce_exports.h"
#define CCE_OPTIONS CCE_EXPORTS
#else
#define CCE_OPTIONS
#endif // Windows

#define CCE_INCORRECT_ENUM 1u
#define CCE_ATTEMPT_TO_OVERRIDE_DEFAULT_ELEMENT 2u
#define CCE_ELEMENT_DOES_NOT_EXIST 3u
#define CCE_OUT_OF_BOUNDS 4u


#define CCE_DEFAULT                            0x000
#define CCE_RENDER_CLOSEST_MAP                 CCE_DEFAULT
#define CCE_RENDER_ONLY_CURRENT_MAP            0x001
#define CCE_RENDER_ALL_LOADED_MAPS             0x002
#define CCE_PROCESS_LOGIC_ONLY_FOR_CURRENT_MAP CCE_DEFAULT
#define CCE_PROCESS_LOGIC_FOR_CLOSEST_MAP      0x004
#define CCE_PROCESS_LOGIC_FOR_ALL_MAPS         0x008

#define CCE_PROCESS_TEXTURES 0x010
#define CCE_PROCESS_UBO_ARRAY 0x020

#define CCE_BASIC_ACTIONS_NOT_SET 0x100
#define CCE_INIT CCE_BASIC_ACTIONS_NOT_SET

#define CCE_BASIC_ACTIONS_QUANTITY 16u

typedef uint_fast32_t cce_flag;



#define CCE_MOVE_GROUP 1u
#define CCE_EXTENSION_GROUP 2u
#define CCE_COLLISION_GROUP 3u

#define CCE_COLLISION_LOGIC_ELEMENT   0x3u

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
// uint8_t moveGroup;          // 0 is unmovable            (deprecated)
// uint8_t moveGroup2;         // 0 is unmovable, max is 16 (deprecated!)
// uint8_t extensionGroup;     // 0 is unscalable           (deprecated)
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
   double                *delaysOfTimers;
   uint32_t               logicQuantity;
   struct ElementLogic   *logic;
   uint8_t                actionsQuantity;
   uint32_t              *actionIDs;
   uint32_t              *actionsArgOffsets;
   cce_void              *actionsArg;
   uint8_t                exitMapsQuantity;
   struct ExitMap2D      *exitMaps;
};

CCE_OPTIONS void setMap2Dpath (const char *path);
CCE_OPTIONS void freeMap2D (struct Map2D *map);
CCE_OPTIONS struct Map2D* loadMap2D (uint16_t number);
CCE_OPTIONS struct Map2D* Map2DdevToMap2D (struct Map2Ddev *mapdev);
CCE_OPTIONS void freeMap2Ddev (struct Map2Ddev *map);
CCE_OPTIONS struct Map2Ddev* loadMap2Ddev (uint16_t number);
CCE_OPTIONS int writeMap2Ddev (struct Map2Ddev *map, void (*writeFunc)(FILE*));
CCE_OPTIONS uint8_t initEngine2D (uint16_t globalBoolsQuantity, uint32_t textureMaxWidth, uint32_t textureMaxHeight,
                      const char *windowLabel, const char *resourcePath);
CCE_OPTIONS void setFlags2D (cce_flag flags);
CCE_OPTIONS uint8_t registerAction (uint32_t ID, void (*action)(void*));
CCE_OPTIONS int engine2D (void);

// dynamicMap2D

CCE_OPTIONS uint8_t getGroupValueDynamicMap2D (cce_enum group_type, uint16_t ID, struct cce_ivec2 *variable);
CCE_OPTIONS struct ElementGroup getGroupDataDynamicMap2D (cce_enum group_type, uint16_t ID);
CCE_OPTIONS struct Map2DCollider getColliderDataDynamicMap2D (uint32_t ID);
CCE_OPTIONS struct CollisionGroup getCollisionDataDynamicMap2D (uint16_t ID);
CCE_OPTIONS uint8_t createGroupDynamicMap2D (cce_enum group_type, uint16_t elementsQuantity, uint32_t *elementIDs, uint16_t *emptyGroupID);
CCE_OPTIONS uint8_t addElementInGroupDynamicMap2D (cce_enum group_type, uint16_t ID, uint32_t elementID);
CCE_OPTIONS void replaceMap2DElementDynamicMap2D (struct Map2DElementDev *element, uint32_t ID, uint8_t hasCollider);
CCE_OPTIONS uint32_t createMap2DElementDynamicMap2D (struct Map2DElementDev *element, uint8_t hasCollider);
CCE_OPTIONS uint8_t deleteElementFromGroupDynamicMap2D (cce_enum group_type, uint16_t ID, uint32_t elementID);
CCE_OPTIONS void deleteMap2DElementDynamicMap2D (uint32_t ID);
CCE_OPTIONS uint8_t deleteGroupDynamicMap2D (cce_enum group_type, uint16_t ID);
CCE_OPTIONS void processDynamicMap2DElements (void);
//CCE_OPTIONS void updateColliderDynamicMap2D (struct Map2DCollider *collider, uint32_t ID);
CCE_OPTIONS void replaceColliderDynamicMap2D (struct Map2DCollider *collider, uint32_t ID);
CCE_OPTIONS void updateCollisionDynamicMap2D (uint16_t ID, uint16_t group1ID, cce_ubyte isGroup1BelongsToCurrentMap2D,
                                               uint16_t group2ID, cce_ubyte isGroup2BelongsToCurrentMap2D);
CCE_OPTIONS uint16_t createCollisionDynamicMap2D (uint16_t group1ID, cce_ubyte isGroup1BelongsToCurrentMap2D, 
                                      uint16_t group2ID, cce_ubyte isGroup2BelongsToCurrentMap2D);
CCE_OPTIONS void deleteCollisionDynamicMap2D (uint16_t ID);
CCE_OPTIONS void setTimerDynamicMap2D (uint16_t ID, float delay);
CCE_OPTIONS uint16_t createTimerDynamicMap2D (float delay);
CCE_OPTIONS void resetTimerDynamicMap2D (uint16_t ID);
CCE_OPTIONS int8_t getTimerDynamicMap2D (uint16_t ID);
CCE_OPTIONS void updateLogicElementByTruthTableDynamicMap2D (const uint16_t ID, const uint8_t logicElementsQuantity, const uint16_t *const logicElements, const cce_enum *const logicElementTypes, const uint_fast16_t *const truthTable);
CCE_OPTIONS uint8_t updateLogicElementsByBooleanExpressionDynamicMap2D (const uint16_t ID, const uint16_t *const logicElements, const cce_enum *const logicElementTypes, const char *const booleanExpression);
CCE_OPTIONS void updateLogicActionsDynamicMap2D (const uint16_t ID, const uint8_t actionsQuantity, uint32_t *actionIDs, const void **actionArgs, const size_t *const actionArgSizes);
CCE_OPTIONS uint16_t findFreeLogicDynamicMap2D (void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // MAP2D_H