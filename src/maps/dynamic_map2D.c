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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include "../../include/coffeechain/engine_common.h"
#include "../../include/coffeechain/utils.h"
#include "../../include/coffeechain/os_interaction.h"
#include "../../include/coffeechain/map2D/map2D.h"

#include "../engine_common_internal.h"
#include "map2D_internal.h"

#define CCE_DYNAMIC_MAP2D_TO_BE_PROCESSED 0x1

static struct DynamicMap2D *g_dynamicMap = NULL;
static struct DynamicMap2DElement nullElement = {0, 0, 0u, 0u, {{0, 0}, {0, 0}, 0u}, 0u, 0u, NULL, NULL, NULL, 0u, 0u, {0}, {0}, {0}, {0}, 0u, 0x4u};
static struct UsedUBO *usedUBO;
static cce_ubyte g_flags;

void cce__setToBeProcessedDynamicMap2D (void)
{
   g_flags |= CCE_DYNAMIC_MAP2D_TO_BE_PROCESSED;
}

static inline void bindVBOtoVAO (GLuint VBO, GLuint VAO)
{
   glBindVertexArray(VAO);
   GL_CHECK_ERRORS;
   glBindBuffer(GL_ARRAY_BUFFER, VBO);
   GL_CHECK_ERRORS;
   
   cce__setAttribPointerVAO();
   
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   GL_CHECK_ERRORS;
   glBindVertexArray(0);
   GL_CHECK_ERRORS;
}

struct DynamicMap2D* cce__initDynamicMap2D (GLuint EBO)
{
   g_dynamicMap = (struct DynamicMap2D*) malloc(sizeof(struct DynamicMap2D));
   g_dynamicMap->elementsQuantity = 0u;
   CCE_ALLOC_ARRAY(g_dynamicMap->elements);
   for (struct DynamicMap2DElement *iterator = g_dynamicMap->elements, *end = g_dynamicMap->elements + g_dynamicMap->elementsQuantityAllocated; iterator < end; ++iterator)
   {
      iterator->flags = 0x0;
   }
   g_dynamicMap->objectBufferAllocatedSpace = g_dynamicMap->elementsQuantityAllocated;
   glGenVertexArrays(1, &g_dynamicMap->VAO);
   GL_CHECK_ERRORS;
   glGenBuffers(1, &g_dynamicMap->VBO);
   GL_CHECK_ERRORS;
   glBindVertexArray(g_dynamicMap->VAO);
   GL_CHECK_ERRORS;
   glBindBuffer(GL_ARRAY_BUFFER, g_dynamicMap->VBO);
   GL_CHECK_ERRORS;
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
   GL_CHECK_ERRORS;
   glBufferData(GL_ARRAY_BUFFER, (sizeof(struct Map2DElementVertices) * 4 * g_dynamicMap->objectBufferAllocatedSpace), NULL, GL_DYNAMIC_DRAW);
   GL_CHECK_ERRORS;
   cce__setAttribPointerVAO();
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   GL_CHECK_ERRORS;
   glBindVertexArray(0);
   GL_CHECK_ERRORS;
   g_dynamicMap->moveGroupsQuantity = 0u;
   CCE_ALLOC_ARRAY_ZEROED(g_dynamicMap->moveGroups);
   g_dynamicMap->extensionGroupsQuantity = 0u;
   CCE_ALLOC_ARRAY_ZEROED(g_dynamicMap->extensionGroups);
   g_dynamicMap->collisionGroupsQuantity = 0u;
   CCE_ALLOC_ARRAY_ZEROED(g_dynamicMap->collisionGroups);
   g_dynamicMap->collisionQuantity = 0u;
   CCE_ALLOC_ARRAY_ZEROED(g_dynamicMap->collision);
   g_dynamicMap->timersQuantity = 0u;
   CCE_ALLOC_ARRAY(g_dynamicMap->timers);
   g_dynamicMap->logicQuantity = 0u;
   CCE_ALLOC_ARRAY_ZEROED(g_dynamicMap->logic);
   
   g_dynamicMap->temporaryBools = cce__getFreeTemporaryBools();
   
   g_dynamicMap->UBO_ID = cce__getFreeUBO();
   g_dynamicMap->delayedActions = LL_LIST_INIT(LL_SINGLELINKED);
   
   usedUBO = cce__getFreeUBOdata(g_dynamicMap->UBO_ID);
   usedUBO->moveGroupValues = calloc(CCE_ALLOCATION_STEP, sizeof(struct cce_i32vec2));
   usedUBO->moveGroupValuesQuantity = CCE_ALLOCATION_STEP;
   usedUBO->extensionGroupValues = calloc(CCE_ALLOCATION_STEP, sizeof(*(usedUBO->extensionGroupValues)));
   usedUBO->extensionGroupValuesQuantity = CCE_ALLOCATION_STEP;
   return g_dynamicMap;
}

CCE_PUBLIC_OPTIONS uint8_t cceGetGroupValueDynamicMap2D (cce_enum group_type, uint16_t ID, struct cce_i32vec2 *variable)
{
   switch (group_type)
   {
      case CCE_MOVE_GROUP:
      {
         if (ID >= (g_dynamicMap->moveGroupsQuantity))
            return CCE_OUT_OF_BOUNDS;
         *variable = *(usedUBO->moveGroupValues + ID);
         break;
      }
      case CCE_EXTENSION_GROUP:
      {
         if (ID >= (g_dynamicMap->extensionGroupsQuantity))
            return CCE_OUT_OF_BOUNDS;
         *variable = (struct cce_i32vec2) {(usedUBO->extensionGroupValues + ID)->x, (usedUBO->extensionGroupValues + ID)->y};
         break;
      }
      default:
      {
         return CCE_INCORRECT_ENUM;
      }
   }
   return 0u;
}

static inline uint8_t* getElementGroupVisiblePointersDynamicMap2D (cce_enum group_type, struct DynamicMap2DElement *element)
{
   switch (group_type)
   {
      case CCE_MOVE_GROUP:
         return (uint8_t*) &(element->visibleMoveGroups);
      case CCE_EXTENSION_GROUP:
         return (uint8_t*) &(element->visibleExtensionGroups);
      default:
         return NULL;
   }
}

static inline uint16_t** getElementGroupPointersDynamicMap2D (cce_enum group_type, struct DynamicMap2DElement *element, uint16_t **groupQuantity)
{
   switch (group_type)
   {
      case CCE_MOVE_GROUP:
      {
         if (groupQuantity)
            *groupQuantity = &element->moveGroupsQuantity;
         return &element->moveGroups;
      }
      case CCE_EXTENSION_GROUP:
      {
         if (groupQuantity)
            *groupQuantity = &element->extensionGroupsQuantity;
         return &element->extensionGroups;
      }
      case CCE_COLLISION_GROUP:
      {
         if (groupQuantity)
            *groupQuantity = &element->collisionGroupsQuantity;
         return &element->collisionGroups;
      }
      default:
      {
         if (groupQuantity)
            *groupQuantity = 0u;
         return NULL;
      }
   }
}

static uint8_t getGroupPointersDynamicMap2D (cce_enum group_type, struct DynamicElementGroup ***groups, uint16_t **groupsQuantity, uint16_t **groupsQuantityAllocated)
{
   switch (group_type)
   {
      case CCE_MOVE_GROUP:
      {
         if (groups)
            *groups = &(g_dynamicMap->moveGroups);
         if (groupsQuantity)
            *groupsQuantity = &(g_dynamicMap->moveGroupsQuantity);
         if (groupsQuantityAllocated)
            *groupsQuantityAllocated = &(g_dynamicMap->moveGroupsQuantityAllocated);
         break;
      }
      case CCE_EXTENSION_GROUP:
      {
         if (groups)
            *groups = &(g_dynamicMap->extensionGroups);
         if (groupsQuantity)
            *groupsQuantity = &(g_dynamicMap->extensionGroupsQuantity);
         if (groupsQuantityAllocated)
            *groupsQuantityAllocated = &(g_dynamicMap->extensionGroupsQuantityAllocated);
         break;
      }
      case CCE_COLLISION_GROUP:
      {
         if (groups)
            *groups = &(g_dynamicMap->collisionGroups);
         if (groupsQuantity)
            *groupsQuantity = &(g_dynamicMap->collisionGroupsQuantity);
         if (groupsQuantityAllocated)
            *groupsQuantityAllocated = &(g_dynamicMap->collisionGroupsQuantityAllocated);
         break;
      }
      default:
      {
         if (groups)
            *groups = NULL;
         if (groupsQuantity)
            *groupsQuantity = NULL;
         if (groupsQuantityAllocated)
            *groupsQuantityAllocated = NULL;
         return CCE_INCORRECT_ENUM;
      }
   }
   return 0u;
}

static struct DynamicElementGroup* getGroupDynamicMap2D (cce_enum group_type, uint16_t ID)
{
   uint16_t *groupsQuantity, *groupsQuantityAllocated;
   struct DynamicElementGroup **groups;
   if (getGroupPointersDynamicMap2D(group_type, &groups, &groupsQuantity, &groupsQuantityAllocated))
   {
      return NULL;
   }
   if ((*groupsQuantityAllocated) <= ID)
   {
      uint16_t lastGroupsQuantityAllocated = (*groupsQuantityAllocated);
      CCE_REALLOC_ARRAY_ZEROED(*groups, ID);
      switch (group_type)
      {
         case CCE_MOVE_GROUP:
         {
            usedUBO->moveGroupValues     = realloc(usedUBO->moveGroupValues,       *groupsQuantityAllocated * sizeof(struct cce_i32vec2));
            memset(usedUBO->moveGroupValues + lastGroupsQuantityAllocated, 0, ((*groupsQuantityAllocated) - lastGroupsQuantityAllocated) * sizeof(struct cce_i32vec2));
            usedUBO->moveGroupValuesQuantity = *groupsQuantityAllocated;
            break;
         }
         case CCE_EXTENSION_GROUP:
         {
            usedUBO->extensionGroupValues = realloc(usedUBO->extensionGroupValues, *groupsQuantityAllocated * sizeof(*(usedUBO->extensionGroupValues)));
            memset(usedUBO->extensionGroupValues + lastGroupsQuantityAllocated, 0, ((*groupsQuantityAllocated) - lastGroupsQuantityAllocated) * sizeof(struct cce_i16vec2));
            usedUBO->extensionGroupValuesQuantity = *groupsQuantityAllocated;
            break;
         }
      }
   }
   if ((*groupsQuantity) <= ID)
   {
      (*groupsQuantity) = ID + 1u;
   }
   return ((*groups) + ID);
}

CCE_PUBLIC_OPTIONS struct ElementGroup cceGetGroupDataDynamicMap2D (cce_enum group_type, uint16_t ID)
{
   struct DynamicElementGroup *group = getGroupDynamicMap2D(group_type, ID);
   if (!group)
   {
      return (struct ElementGroup) {NULL, 0u};
   }

   return (struct ElementGroup) {group->elements, group->elementsQuantity};
}

CCE_PUBLIC_OPTIONS struct Map2DCollider cceGetColliderDataDynamicMap2D (uint32_t ID)
{
   if (ID < g_dynamicMap->elementsQuantity)
   {
      return (struct Map2DCollider) {0, 0, 0u, 0u};
   }
   struct DynamicMap2DElement *element = g_dynamicMap->elements + ID;
   struct cce_i32vec2 coordOffset = {0, 0}, sizeOffset = {0, 0}, tmp;
   if ((element->flags & 0x2) && !(element->flags & 0x4))
   {
      for (uint16_t *iterator = (g_dynamicMap->elements + ID)->moveGroups, *end = (g_dynamicMap->elements + ID)->moveGroups + (g_dynamicMap->elements + ID)->moveGroupsQuantity;
      iterator < end; ++iterator)
      {
         cceGetGroupValueDynamicMap2D(CCE_MOVE_GROUP, (g_dynamicMap->elements + ID)->moveGroups - iterator, &tmp);
         coordOffset.x += tmp.x;
         coordOffset.y += tmp.y;
      }
      for (uint16_t *iterator = (g_dynamicMap->elements + ID)->extensionGroups, *end = (g_dynamicMap->elements + ID)->extensionGroups + (g_dynamicMap->elements + ID)->extensionGroupsQuantity;
      iterator < end; ++iterator)
      {
         cceGetGroupValueDynamicMap2D(CCE_EXTENSION_GROUP, (g_dynamicMap->elements + ID)->extensionGroups - iterator, &tmp);
         sizeOffset.x += tmp.x;
         sizeOffset.y += tmp.y;
      }
   }
   return (struct Map2DCollider) {element->x - coordOffset.x, element->y - coordOffset.y, element->width - sizeOffset.x, element->height - sizeOffset.y};
}

uint8_t cce__getDynamicElementFlags (uint16_t ID)
{
   return (g_dynamicMap->elements + ID)->flags;
}

CCE_PUBLIC_OPTIONS struct CollisionGroup cceGetCollisionDataDynamicMap2D (uint16_t ID)
{
   if (ID < g_dynamicMap->collisionQuantity)
   {
      return (struct CollisionGroup) {UINT16_MAX, UINT16_MAX};
   }
   struct DynamicCollisionGroup *group = (g_dynamicMap->collision + ID);
   return (struct CollisionGroup) {group->group1, group->group2};
}

CCE_PUBLIC_OPTIONS uint16_t cceCreateGroupDynamicMap2D (cce_enum group_type, uint16_t elementsQuantity, uint32_t *elements)
{
   struct DynamicElementGroup *group;
   struct DynamicElementGroup **groups;
   {
      uint16_t *groupsQuantity;
      if (getGroupPointersDynamicMap2D(group_type, &groups, &groupsQuantity, NULL) == CCE_INCORRECT_ENUM)
      {
         ptrdiff_t error = group_type;  
         cce__errorPrint("ENGINE::GET_EMPTY_GROUP_FROM_DYNAMIC_MAP2D::INCORRECT_ENUM:\ngroup_type: %i passed to the function is not valid", (void*) error);
         return 0u;
      }
      for (struct DynamicElementGroup *iterator = (*groups), *end = (*groups + *groupsQuantity);; ++iterator)
      {
         if (iterator >= end)
         {
            group = getGroupDynamicMap2D(group_type, *groupsQuantity);
            break;
         }
         if (!iterator->elementsQuantity)
         {
            group = iterator;
            break;
         }
      }
   }
   CCE_REALLOC_ARRAY(group->elements, elementsQuantity);
   if (elements)
   {
      group->elementsQuantity = elementsQuantity;
      memcpy(group->elements, elements, elementsQuantity * sizeof(uint32_t));
   }
   return group - (*groups);
}

CCE_PUBLIC_OPTIONS uint8_t cceAddElementInGroupDynamicMap2D (cce_enum group_type, uint16_t ID, uint32_t elementID)
{
   // All group IDs should begin with 1. Special case is moveGroup because 0 means globalOffset
   if (group_type != CCE_MOVE_GROUP)
   {
      if (ID == 0)
         return 1u;

      --ID;
   }
   struct DynamicElementGroup *group = getGroupDynamicMap2D(group_type, ID);
   if (!group)
   {
      ptrdiff_t error = group_type;  
      cce__errorPrint("ENGINE::ADD_ELEMENT_IN_GROUP_FROM_DYNAMIC_MAP2D::INCORRECT_ENUM:\ngroup_type: %i passed to the function is not valid", (void*) error);
      return CCE_INCORRECT_ENUM;
   }
   for (uint32_t *iterator = group->elements, *end = group->elements + group->elementsQuantity; iterator < end; ++iterator)
   {
      if ((*iterator) == elementID)
      {
         return 0u;
      }
   }
   ++(group->elementsQuantity);
   CCE_FIT_ARRAY_TO_SIZE(group->elements);
   *(group->elements + group->elementsQuantity - 1) = elementID;
   
   uint16_t  *elementGroupLength;
   uint16_t **elementGroup = getElementGroupPointersDynamicMap2D(group_type, g_dynamicMap->elements + elementID, &elementGroupLength);
   ++(*elementGroupLength);
   *elementGroup = realloc(*elementGroup, *elementGroupLength * sizeof(uint16_t));
   *((*elementGroup) + (*elementGroupLength) - 1u) = ID;
   return 0u;
}

CCE_PUBLIC_OPTIONS uint8_t cceAddElementInGroupVisibleDynamicMap2D (cce_enum group_type, uint16_t ID, uint32_t elementID)
{
   if (group_type != CCE_MOVE_GROUP && group_type != CCE_EXTENSION_GROUP)
      return CCE_INCORRECT_ENUM;

   if (ID == 0)
      return 1;
   cceAddElementInGroupDynamicMap2D(group_type, ID, elementID);

   uint8_t *groups = getElementGroupVisiblePointersDynamicMap2D(group_type, g_dynamicMap->elements + elementID);
   for (uint8_t *end = groups + 4; groups < end; ++groups)
   {
      if (*groups == 0)
      {
         *groups = ID;
         return 0;
      }

      if (*groups == ID)
         return 0;
   }
   return CCE_OUT_OF_BOUNDS;
}

static void updateMap2DElementDynamicMap2D (struct Map2DElementDev *element, uint32_t ID, cce_enum elementType, uint8_t flags)
{
   struct DynamicMap2DElement *dynamicElement = g_dynamicMap->elements + ID;
   dynamicElement->x = element->x;
   dynamicElement->y = element->y;
   dynamicElement->width = element->width;
   dynamicElement->height = element->height;
   dynamicElement->textureInfo = element->textureInfo;
   if (flags & CCE_TEXTUREID_IS_NOT_IMAGEID)
   {
      dynamicElement->textureInfo.ID = UINT32_MAX;
      dynamicElement->textureElementReliesOn = element->textureInfo.ID;
   }
   memcpy(dynamicElement->textureOffsetGroups, element->textureOffsetGroups, 4);
   memcpy(dynamicElement->colorGroups, element->colorGroups, 4);
   dynamicElement->rotateGroup = element->rotateGroup;
   if (elementType & CCE_COLLIDER)
   {
      if (!(element->isGlobalOffset))
         cceAddElementInGroupDynamicMap2D(CCE_MOVE_GROUP, 0u, ID);

      for (uint8_t *iterator = element->moveGroups, *end = element->moveGroups + 4; iterator < end; ++iterator)
      {
         if (*iterator > 0)
            cceAddElementInGroupVisibleDynamicMap2D(CCE_MOVE_GROUP, *iterator, ID);
      }

      for (uint8_t *iterator = element->extensionGroups, *end = element->extensionGroups + 4; iterator < end; ++iterator)
      {
         if (*iterator > 0)
            cceAddElementInGroupVisibleDynamicMap2D(CCE_EXTENSION_GROUP, *iterator, ID);
      }
   }
   else
   {
      memcpy(dynamicElement->visibleMoveGroups, element->moveGroups, 4 * sizeof(uint8_t));
      memcpy(dynamicElement->visibleExtensionGroups, element->extensionGroups, 4 * sizeof(uint8_t));
   }
   // elementType is a bitfield, not just enum (intentional)
   dynamicElement->flags = 0x1 | (((elementType & CCE_COLLIDER) > 0) << 1) | 0x4 | (((elementType & CCE_ELEMENT_WITHOUT_COLLIDER) > 0) << 3) | 
                           ((element->isGlobalOffset) << 4) | (!(flags & CCE_POSITION_IS_NOT_CURRENT) << 5);
   
   g_flags |= CCE_DYNAMIC_MAP2D_TO_BE_PROCESSED;
}

#define SET_MAP2DELEMENTGROUPS_TO_NULL_DYNAMICMAP2D(element) \
(element)->moveGroupsQuantity = 0u;      \
(element)->moveGroups = NULL;            \
(element)->extensionGroupsQuantity = 0u; \
(element)->extensionGroups = NULL;       \
(element)->collisionGroupsQuantity = 0u; \
(element)->collisionGroups = NULL;       \
(element)->visibleMoveGroups[0] = 0; \
(element)->visibleMoveGroups[1] = 0; \
(element)->visibleMoveGroups[2] = 0; \
(element)->visibleMoveGroups[3] = 0; \
(element)->visibleExtensionGroups[0] = 0; \
(element)->visibleExtensionGroups[1] = 0; \
(element)->visibleExtensionGroups[2] = 0; \
(element)->visibleExtensionGroups[3] = 0; \
(element)->textureElementReliesOn = 0;

CCE_PUBLIC_OPTIONS uint32_t cceCreateMap2DElementDynamicMap2D (struct Map2DElementDev *element, cce_enum elementType, uint8_t flags)
{
   for (struct DynamicMap2DElement *iterator = g_dynamicMap->elements, *end = g_dynamicMap->elements + g_dynamicMap->elementsQuantity; iterator < end; ++iterator)
   {
      if (!(iterator->flags & 0x1))
      {
         SET_MAP2DELEMENTGROUPS_TO_NULL_DYNAMICMAP2D(iterator);
         updateMap2DElementDynamicMap2D(element, (uint32_t) (iterator - g_dynamicMap->elements), elementType, flags);
         return (uint32_t) (iterator - g_dynamicMap->elements);
      }
   }
   ++(g_dynamicMap->elementsQuantity);
   if (g_dynamicMap->elementsQuantity > g_dynamicMap->elementsQuantityAllocated)
   {
      uint32_t prevQuantityAllocated = g_dynamicMap->elementsQuantityAllocated;
      g_dynamicMap->elementsQuantityAllocated += CCE_ALLOCATION_STEP;
      g_dynamicMap->elements = realloc(g_dynamicMap->elements, g_dynamicMap->elementsQuantityAllocated * sizeof(struct DynamicMap2DElement));
      for (struct DynamicMap2DElement *iterator = g_dynamicMap->elements + prevQuantityAllocated, *end = g_dynamicMap->elements + g_dynamicMap->elementsQuantityAllocated; iterator < end; ++iterator)
      {
         iterator->flags = 0x0;
      }
   }
   SET_MAP2DELEMENTGROUPS_TO_NULL_DYNAMICMAP2D(g_dynamicMap->elements + g_dynamicMap->elementsQuantity - 1u);
   updateMap2DElementDynamicMap2D(element, g_dynamicMap->elementsQuantity - 1u, elementType, flags);
   return g_dynamicMap->elementsQuantity - 1u;
}

CCE_PUBLIC_OPTIONS uint32_t* cceCreateMap2DElementsDynamicMap2D (struct Map2DElementDev *elements, uint32_t elementsQuantity, cce_enum elementType, uint8_t flags)
{
   uint32_t *elementIDs = malloc(elementsQuantity * sizeof(uint32_t));
   uint32_t *IDiterator = elementIDs;
   struct Map2DElementDev *elementsEnd = elements + elementsQuantity;
   uint32_t elementsLeft = elementsQuantity;
   for (struct DynamicMap2DElement *iterator = g_dynamicMap->elements, *end = g_dynamicMap->elements + g_dynamicMap->elementsQuantity; iterator < end; ++iterator)
   {
      if (!(iterator->flags & 0x1))
      {
         SET_MAP2DELEMENTGROUPS_TO_NULL_DYNAMICMAP2D(iterator);
         *IDiterator = (uint32_t) (iterator - g_dynamicMap->elements);
         updateMap2DElementDynamicMap2D(elements, *IDiterator, elementType, flags);
         ++elements, ++IDiterator, --elementsLeft;
         if (elements >= elementsEnd)
            return elementIDs;
      }
   }
   uint32_t dynamicElementID = g_dynamicMap->elementsQuantity;
   g_dynamicMap->elementsQuantity += elementsLeft;
   if (g_dynamicMap->elementsQuantity > g_dynamicMap->elementsQuantityAllocated)
   {
      uint32_t prevQuantityAllocated = g_dynamicMap->elementsQuantityAllocated;
      CCE_FIT_ARRAY_TO_SIZE(g_dynamicMap->elements);
      for (struct DynamicMap2DElement *iterator = g_dynamicMap->elements + prevQuantityAllocated, *end = g_dynamicMap->elements + g_dynamicMap->elementsQuantityAllocated;
           iterator < end; ++iterator)
      {
         iterator->flags = 0x0;
      }
   }
   while (elements < elementsEnd)
   {
      SET_MAP2DELEMENTGROUPS_TO_NULL_DYNAMICMAP2D(g_dynamicMap->elements + dynamicElementID);
      *IDiterator = dynamicElementID;
      updateMap2DElementDynamicMap2D(elements, dynamicElementID, elementType, flags);
      ++elements, ++IDiterator, ++dynamicElementID;
   }
   return elementIDs;
}

CCE_PUBLIC_OPTIONS uint8_t cceDeleteGroupVisibilityFromElementDynamicMap2D (cce_enum group_type, uint16_t ID, uint32_t elementID)
{
   uint8_t *groups = getElementGroupVisiblePointersDynamicMap2D(group_type, g_dynamicMap->elements + elementID);
   if (groups == NULL)
      return CCE_INCORRECT_ENUM;
   for (uint8_t *end = groups + 4; groups < end; ++groups)
   {
      if (*groups == ID)
      {
         *groups = 0;
         break;
      }
   }
   return 0;
}

CCE_PUBLIC_OPTIONS uint8_t cceDeleteElementFromGroupDynamicMap2D (cce_enum group_type, uint16_t ID, uint32_t elementID)
{
   struct DynamicElementGroup *group;
   {
      struct DynamicElementGroup **groups;
      uint16_t *groupsQuantity;
      uint16_t *groupsQuantityAllocated;
      if ((getGroupPointersDynamicMap2D(group_type, &groups, &groupsQuantity, &groupsQuantityAllocated)) == CCE_INCORRECT_ENUM)
      {
         ptrdiff_t error = group_type;
         cce__errorPrint("ENGINE::DELETE_ELEMENT_FROM_GROUP_IN_DYNAMIC_MAP2D::INCORRECT_ENUM:\ngroup_type: %i passed to the function is not valid", (void*) error);
         return CCE_INCORRECT_ENUM;
      }
      if (ID >= (*groupsQuantity))
      {
         ptrdiff_t error = ID;
         cce__errorPrint("ENGINE::DELETE_ELEMENT_FROM_GROUP_IN_DYNAMIC_MAP2D::GROUP_DOES_NOT_EXIST:\ngroup ID: %i passed to the function does not exist", (void*) error);
         return CCE_ELEMENT_DOES_NOT_EXIST;
      }
      group = ((*groups) + ID);
   }
   for (uint32_t *iterator = group->elements, *end = group->elements + group->elementsQuantity; iterator < end; ++iterator)
   {
      if ((*iterator) == elementID)
      {
         ++iterator;
         --(group->elementsQuantity);
         while (iterator < end)
         {
            *(iterator - 1u) = *iterator;
            ++iterator;
         }
         break;
      }
   }
   
   uint16_t  *elementGroupLength;
   uint16_t **elementGroup = getElementGroupPointersDynamicMap2D(group_type, g_dynamicMap->elements + elementID, &elementGroupLength);
   for (uint16_t *iterator = (*elementGroup), *end = (*elementGroup) + (*elementGroupLength); iterator < end; ++iterator)
   {
      if ((*iterator) == ID)
      {
         ++iterator;
         --(*elementGroupLength);
         while (iterator < end)
         {
            *(iterator - 1u) = *iterator;
            ++iterator;
         }
         *elementGroup = realloc(*elementGroup, (*elementGroupLength) * sizeof(uint16_t));
         break;
      }
   }
   cceDeleteGroupVisibilityFromElementDynamicMap2D(group_type, ID, elementID);
   return 0u;
}

static void deleteGroupsFromElementDynamicMap2D (struct DynamicMap2DElement *element)
{
   if (element >= g_dynamicMap->elements + g_dynamicMap->elementsQuantity)
      return;
   if ((element->flags) & 0x2u)
   {
      if (element->moveGroupsQuantity)
      {
         for (uint16_t *iterator = element->moveGroups, *end = element->moveGroups + element->moveGroupsQuantity;
         iterator < end; ++iterator)
         {
            cceDeleteElementFromGroupDynamicMap2D(CCE_MOVE_GROUP, *iterator, (uint32_t) (element - g_dynamicMap->elements));
         }
      }
      
      if (element->extensionGroupsQuantity)
      {
         for (uint16_t *iterator = element->extensionGroups, *end = element->extensionGroups + element->extensionGroupsQuantity;
         iterator < end; ++iterator)
         {
            cceDeleteElementFromGroupDynamicMap2D(CCE_EXTENSION_GROUP, *iterator, (uint32_t) (element - g_dynamicMap->elements));
         }
      }
      
      if (element->collisionGroupsQuantity)
      {
         for (uint16_t *iterator = element->collisionGroups, *end = element->collisionGroups + element->collisionGroupsQuantity;
         iterator < end; ++iterator)
         {
            cceDeleteElementFromGroupDynamicMap2D(CCE_COLLISION_GROUP, *iterator, (uint32_t) (element - g_dynamicMap->elements));
         }
      }
   }
}

CCE_PUBLIC_OPTIONS void cceReplaceMap2DElementDynamicMap2D (struct Map2DElementDev *element, uint32_t ID, uint8_t hasCollider, uint8_t isCurrentPosition)
{
   deleteGroupsFromElementDynamicMap2D((g_dynamicMap->elements + ID));
   updateMap2DElementDynamicMap2D(element, ID, hasCollider, isCurrentPosition);
}

CCE_PUBLIC_OPTIONS void cceDeleteMap2DElementDynamicMap2D (uint32_t ID)
{
   deleteGroupsFromElementDynamicMap2D((g_dynamicMap->elements + ID));
   cce__releaseTexture((g_dynamicMap->elements + ID)->textureElementReliesOn);
   memcpy((g_dynamicMap->elements + ID), &nullElement, sizeof(struct DynamicMap2DElement));
   (g_dynamicMap->elements + ID)->flags = 0x4;
   g_flags |= CCE_DYNAMIC_MAP2D_TO_BE_PROCESSED;
   return;
}

CCE_PUBLIC_OPTIONS uint8_t cceDeleteGroupDynamicMap2D (cce_enum group_type, uint16_t ID)
{
   struct DynamicElementGroup **groups;
   uint16_t *groupsQuantity;
   uint16_t *groupsQuantityAllocated;
   if (!(getGroupPointersDynamicMap2D(group_type, &groups, &groupsQuantity, &groupsQuantityAllocated)))
   {
      ptrdiff_t error = group_type;
      cce__errorPrint("ENGINE::DELETE_ELEMENT_FROM_GROUP_IN_DYNAMIC_MAP2D::INCORRECT_ENUM:\ngroup_type: %i passed to the function is not valid", (void*) error);
      return CCE_INCORRECT_ENUM;
   }
   if (ID >= (*groupsQuantity))
   {
      ptrdiff_t error = ID;
      cce__errorPrint("ENGINE::DELETE_ELEMENT_FROM_GROUP_IN_DYNAMIC_MAP2D::GROUP_DOES_NOT_EXIST:\ngroup ID: %i passed to the function does not exist", (void*) error);
      return CCE_ELEMENT_DOES_NOT_EXIST;
   }
   uint16_t elementsQuantity = ((*groups) + ID)->elementsQuantity;
   ((*groups) + ID)->elementsQuantity = 0;
   for (uint32_t *iterator = ((*groups) + ID)->elements, *end = ((*groups) + ID)->elements + elementsQuantity; iterator < end; ++iterator)
   {
      cceDeleteElementFromGroupDynamicMap2D(group_type, ID, *iterator);
   }
   return 0u;
}

void cce__processDynamicMap2DElements (void)
{
   if (!(g_flags & CCE_DYNAMIC_MAP2D_TO_BE_PROCESSED))
   {
      return;
   }
   glBindVertexArray(g_dynamicMap->VAO);
   GL_CHECK_ERRORS;
   
   if (g_dynamicMap->objectBufferAllocatedSpace < g_dynamicMap->elementsQuantityAllocated)
   {
      uint32_t newBuffer;
      glGenBuffers(1, &newBuffer);
      GL_CHECK_ERRORS;
      glBindBuffer(GL_COPY_WRITE_BUFFER, newBuffer);
      GL_CHECK_ERRORS;
      glBufferData(GL_COPY_WRITE_BUFFER, g_dynamicMap->elementsQuantityAllocated * sizeof(struct Map2DElementVertices) * 4, NULL, GL_DYNAMIC_DRAW);
      GL_CHECK_ERRORS;
      glBindBuffer(GL_COPY_READ_BUFFER, g_dynamicMap->VBO);
      GL_CHECK_ERRORS;
      glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_COPY_WRITE_BUFFER, 0u, 0u, (g_dynamicMap->objectBufferAllocatedSpace) * sizeof(struct Map2DElementVertices) * 4);
      GL_CHECK_ERRORS;
      glDeleteBuffers(1u, &g_dynamicMap->VBO);
      GL_CHECK_ERRORS;
      g_dynamicMap->VBO = newBuffer;
      bindVBOtoVAO(g_dynamicMap->VBO, g_dynamicMap->VAO);
      g_dynamicMap->objectBufferAllocatedSpace = g_dynamicMap->elementsQuantityAllocated;
      cce__extendElementBufferIfNecessary(g_dynamicMap->elementsQuantityAllocated);
   }
   glBindBuffer(GL_ARRAY_BUFFER, g_dynamicMap->VBO);
   GL_CHECK_ERRORS;
   struct Map2DElementVertices *bufferPtr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
   GL_CHECK_ERRORS;
   
   for (struct DynamicMap2DElement *iterator = g_dynamicMap->elements, *end = (g_dynamicMap->elements) + (g_dynamicMap->elementsQuantity); iterator < end; ++iterator)
   {
      if (iterator->flags & 0x4u)
      {
         struct cce_i32vec2 moveGroupOffset = {0, 0}, extensionGroupOffset = {0, 0};
         if (iterator->flags & 0x22)
         {
            struct cce_i32vec2 tmp;
            for (uint16_t *jiterator = iterator->moveGroups, *jend = iterator->moveGroups + iterator->moveGroupsQuantity;
            jiterator < jend; ++jiterator)
            {
               cceGetGroupValueDynamicMap2D(CCE_MOVE_GROUP, (iterator->moveGroups - jiterator), &tmp);
               moveGroupOffset.x += tmp.x;
               moveGroupOffset.y += tmp.y;
            }
            for (uint16_t *jiterator = iterator->extensionGroups, *jend = iterator->extensionGroups + iterator->extensionGroupsQuantity;
            jiterator < jend; ++jiterator)
            {
               cceGetGroupValueDynamicMap2D(CCE_EXTENSION_GROUP, (iterator->extensionGroups - jiterator), &tmp);
               extensionGroupOffset.x += tmp.x;
               extensionGroupOffset.y += tmp.y;
            }
         }
         if (iterator->textureElementReliesOn == 0)
         {
            iterator->textureElementReliesOn = cce__loadTexture(iterator->textureInfo.ID);
         }
         if (iterator->flags & 0x20)
         {
            iterator->x -= moveGroupOffset.x;
            iterator->y -= moveGroupOffset.y;
            iterator->width  -= extensionGroupOffset.x;
            iterator->height -= extensionGroupOffset.y;
         }
         if (iterator->flags & 0x8)
            cce__dynamicMap2DElementToMap2DElementVertices(bufferPtr + (iterator - g_dynamicMap->elements) * 4, iterator);
         else
            cce__dynamicMap2DElementToMap2DElementVertices(bufferPtr + (iterator - g_dynamicMap->elements) * 4, &nullElement);
         
         iterator->flags &= ~0x4u;
         if (iterator->flags & 0x2)
         {
            iterator->x += moveGroupOffset.x;
            iterator->y += moveGroupOffset.y;
            iterator->width  += extensionGroupOffset.x;
            iterator->height += extensionGroupOffset.y;
            iterator->flags |= 0x20;
         }
         else
         {
            iterator->flags &= ~0x20;
         }
      }
   }
   glUnmapBuffer(GL_ARRAY_BUFFER);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);
}

/*
void cceUpdateColliderDynamicMap2D (struct Map2DCollider *collider, uint32_t ID)
{
   struct cce_uvec2 groupValueOffset = {0, 0}, tmp;
   for (uint16_t *iterator = (g_dynamicMap->elements + ID)->moveGroups, *end = (g_dynamicMap->elements + ID)->moveGroups + (g_dynamicMap->elements + ID)->moveGroupsQuantity;
   iterator < end; ++iterator)
   {
      cceGetGroupValueDynamicMap2D(CCE_MOVE_GROUP, *iterator, &tmp);
      groupValueOffset.x += tmp.x;
      groupValueOffset.y += tmp.y;
   }
   (g_dynamicMap->elements + ID)->x = collider->x + groupValueOffset.x;
   (g_dynamicMap->elements + ID)->y = collider->y + groupValueOffset.y;
   for (uint16_t *iterator = (g_dynamicMap->elements + ID)->extensionGroups, *end = (g_dynamicMap->elements + ID)->extensionGroups + (g_dynamicMap->elements + ID)->extensionGroupsQuantity;
   iterator < end; ++iterator)
   {
      cceGetGroupValueDynamicMap2D(CCE_EXTENSION_GROUP, *iterator, &tmp);
      groupValueOffset.x += tmp.x;
      groupValueOffset.y += tmp.y;
   }
   (g_dynamicMap->elements + ID)->width = collider->width + groupValueOffset.x;
   (g_dynamicMap->elements + ID)->height = collider->height + groupValueOffset.y;
}
*/

CCE_PUBLIC_OPTIONS void cceReplaceColliderDynamicMap2D (struct Map2DCollider *collider, uint32_t ID)
{
   for (uint16_t *iterator = (g_dynamicMap->elements + ID)->moveGroups, *end = (g_dynamicMap->elements + ID)->moveGroups + (g_dynamicMap->elements + ID)->moveGroupsQuantity;
   iterator < end; ++iterator)
   {
      cceDeleteElementFromGroupDynamicMap2D(CCE_MOVE_GROUP, *iterator, ID);
   }
   for (uint16_t *iterator = (g_dynamicMap->elements + ID)->extensionGroups, *end = (g_dynamicMap->elements + ID)->extensionGroups + (g_dynamicMap->elements + ID)->extensionGroupsQuantity;
   iterator < end; ++iterator)
   {
      cceDeleteElementFromGroupDynamicMap2D(CCE_EXTENSION_GROUP, *iterator, ID);
   }
   for (uint16_t *iterator = (g_dynamicMap->elements + ID)->collisionGroups, *end = (g_dynamicMap->elements + ID)->collisionGroups + (g_dynamicMap->elements + ID)->collisionGroupsQuantity;
   iterator < end; ++iterator)
   {
      cceDeleteElementFromGroupDynamicMap2D(CCE_COLLISION_GROUP, *iterator, ID);
   }
   if ((g_dynamicMap->elements + ID)->flags & CCE_GLOBAL_OFFSET_MASK)
   {
      cceDeleteElementFromGroupDynamicMap2D(CCE_MOVE_GROUP, 0u, ID);
      (g_dynamicMap->elements + ID)->flags ^= CCE_GLOBAL_OFFSET_MASK;
   }
   (g_dynamicMap->elements + ID)->x = collider->x;
   (g_dynamicMap->elements + ID)->y = collider->y;
   (g_dynamicMap->elements + ID)->width = collider->width;
   (g_dynamicMap->elements + ID)->height = collider->height;
}

cce_ubyte cce__fourthLogicTypeFuncDynamicMap2D (uint16_t ID, va_list argp)
{
   struct Map2D *map = (struct Map2D*) va_arg(argp, struct Map2D*);
   uint32_t *group1firstID, *group2firstID;
   cce_void *elements1, *elements2;
   uint16_t groups1Quantity, groups2Quantity;
   uint8_t element1Size, element2Size;
   if ((g_dynamicMap->collision + ID)->flags & 0x2)
   {
      if ((g_dynamicMap->collision + ID)->group1 >= map->collisionGroupsQuantity)
         return 0;
      group1firstID = (map->collisionGroups + (g_dynamicMap->collision + ID)->group1)->elements;
      groups1Quantity = (map->collisionGroups + (g_dynamicMap->collision + ID)->group1)->elementsQuantity;
      elements1 = (cce_void*) map->colliders;
      element1Size = sizeof(struct Map2DCollider);
   }
   else
   {
      if ((g_dynamicMap->collision + ID)->group1 >= g_dynamicMap->collisionGroupsQuantity)
         return 0;
      group1firstID = (g_dynamicMap->collisionGroups + (g_dynamicMap->collision + ID)->group1)->elements;
      groups1Quantity = (g_dynamicMap->collisionGroups + (g_dynamicMap->collision + ID)->group1)->elementsQuantity;
      elements1 = (cce_void*) g_dynamicMap->elements;
      element1Size = sizeof(struct DynamicMap2DElement);
   }
   if ((g_dynamicMap->collision + ID)->flags & 0x4)
   {
      if ((g_dynamicMap->collision + ID)->group2 >= map->collisionGroupsQuantity)
         return 0;
      group2firstID = ((map->collisionGroups + (g_dynamicMap->collision + ID)->group2)->elements);
      groups2Quantity = (map->collisionGroups + (g_dynamicMap->collision + ID)->group2)->elementsQuantity;
      elements2 = (cce_void*) map->colliders;
      element2Size = sizeof(struct Map2DCollider);
   }
   else
   {
      if ((g_dynamicMap->collision + ID)->group2 >= g_dynamicMap->collisionGroupsQuantity)
         return 0;
      group2firstID = ((g_dynamicMap->collisionGroups + (g_dynamicMap->collision + ID)->group2)->elements);
      groups2Quantity = (g_dynamicMap->collisionGroups + (g_dynamicMap->collision + ID)->group2)->elementsQuantity;
      elements2 = (cce_void*) g_dynamicMap->elements;
      element2Size = sizeof(struct DynamicMap2DElement);
   }
   return cce__checkCollision(group1firstID, groups1Quantity, group2firstID, groups2Quantity, elements1, element1Size, elements2, element2Size);
}

#define cce__checkCollisionBetweenMaps(collision, collisionGroups1, collisionGroups2, elements1, elements2) \
cce__checkCollision(((collisionGroups1) + (collision)->group1)->elements, ((collisionGroups1) + (collision)->group1)->elementsQuantity, \
                    ((collisionGroups2) + (collision)->group2)->elements, ((collisionGroups2) + (collision)->group2)->elementsQuantity, \
                    (cce_void*) elements1, sizeof(*(elements1)), (cce_void*) elements2, sizeof(*(elements2)))

#define cce__checkCollisionBetweenMapsWithOffset(collision, collisionGroups1, collisionGroups2, elements1, elements2, offset1, offset2) \
cce__checkCollisionWithOffset(((collisionGroups1) + (collision)->group1)->elements, ((collisionGroups1) + (collision)->group1)->elementsQuantity, \
                              ((collisionGroups2) + (collision)->group2)->elements, ((collisionGroups2) + (collision)->group2)->elementsQuantity, \
                              (cce_void*) elements1, sizeof(*(elements1)), offset1, (cce_void*) elements2, sizeof(*(elements2)), offset2)

cce_ubyte cce__checkCollisionDynamicMap2DmultipleMaps (uint16_t ID, struct Map2D *map, struct Map2D **maps, size_t mapsQuantity, const struct cce_i32vec2 *mapOffsets, size_t mapOffsetsSize)
{
   const struct DynamicCollisionGroup *collision = (g_dynamicMap->collision + ID);
   const struct cce_i32vec2 *offsets = mapOffsets;
   struct cce_i32vec2 zero = {0, 0};
   switch (collision->flags & 0x6)
   {
      case 0x0:
         return cce__checkCollisionBetweenMaps(collision, g_dynamicMap->collisionGroups, g_dynamicMap->collisionGroups, g_dynamicMap->elements, g_dynamicMap->elements);
      case 0x2:
         if (cce__checkCollisionBetweenMaps(collision, map->collisionGroups, g_dynamicMap->collisionGroups, map->colliders, g_dynamicMap->elements))
            return 1;
         for (struct Map2D **iterator = maps, **end = maps + mapsQuantity; iterator < end; ++iterator, offsets = (const struct cce_i32vec2*) (((cce_void*) offsets) + mapOffsetsSize))
         {
            if (cce__checkCollisionBetweenMapsWithOffset(collision, (*iterator)->collisionGroups, g_dynamicMap->collisionGroups, (*iterator)->colliders, g_dynamicMap->elements, offsets, &zero))
               return 1;
         }
         break;
      case 0x4:
         if (cce__checkCollisionBetweenMaps(collision, g_dynamicMap->collisionGroups, map->collisionGroups, g_dynamicMap->elements, map->colliders))
            return 1;
         for (struct Map2D **iterator = maps, **end = maps + mapsQuantity; iterator < end; ++iterator, offsets = (const struct cce_i32vec2*) (((cce_void*) offsets) + mapOffsetsSize))
         {
            if (cce__checkCollisionBetweenMapsWithOffset(collision, g_dynamicMap->collisionGroups, (*iterator)->collisionGroups, g_dynamicMap->elements, (*iterator)->colliders, &zero, offsets))
               return 1;
         }
         break;
      case 0x6: // Both collisions are not with dynamicMap
      {
         if (cce__checkCollisionBetweenMaps(collision, map->collisionGroups, map->collisionGroups, map->colliders, map->colliders))
            return 1;
         offsets = mapOffsets;
         // Check collision between currentMap and all nearest maps
         for (struct Map2D **iterator = maps, **end = maps + mapsQuantity; iterator < end; ++iterator, offsets = (const struct cce_i32vec2*) (((cce_void*) offsets) + mapOffsetsSize))
         {
            if (cce__checkCollisionBetweenMapsWithOffset(collision, map->collisionGroups, (*iterator)->collisionGroups, map->colliders, (*iterator)->colliders, &zero, offsets))
               return 1;
         }
         offsets = mapOffsets;
         // Check collision between all nearest maps
         for (struct Map2D **iterator = maps, **end = maps + mapsQuantity; iterator < end; ++iterator, offsets = (const struct cce_i32vec2*) (((cce_void*) offsets) + mapOffsetsSize))
         {
            const struct cce_i32vec2 *offsets2 = offsets;
            for (struct Map2D **jiterator = iterator, **jend = maps + mapsQuantity; jiterator < jend; ++jiterator, offsets2 = (const struct cce_i32vec2*) (((cce_void*) offsets2) + mapOffsetsSize))
            {
               if (cce__checkCollisionBetweenMapsWithOffset(collision, (*iterator)->collisionGroups, (*jiterator)->collisionGroups,
                                                            (*iterator)->colliders, (*jiterator)->colliders, offsets, offsets2))
                  return 1;
            }
         }
         break;
      }
   }
   return 0;
}

CCE_PUBLIC_OPTIONS void cceUpdateCollisionDynamicMap2D (uint16_t ID, uint16_t group1ID, cce_ubyte isGroup1BelongsToCurrentMap2D, uint16_t group2ID, cce_ubyte isGroup2BelongsToCurrentMap2D)
{
   (g_dynamicMap->collision + ID)->group1 = group1ID - (isGroup1BelongsToCurrentMap2D == 0u);
   (g_dynamicMap->collision + ID)->group2 = group2ID - (isGroup2BelongsToCurrentMap2D == 0u);
   (g_dynamicMap->collision + ID)->flags = 0x1 + ((isGroup1BelongsToCurrentMap2D > 0u) << 1u) + ((isGroup2BelongsToCurrentMap2D > 0u) << 2u);
}

CCE_PUBLIC_OPTIONS uint16_t cceCreateCollisionDynamicMap2D (uint16_t group1ID, cce_ubyte isGroup1BelongsToCurrentMap2D, uint16_t group2ID, cce_ubyte isGroup2BelongsToCurrentMap2D)
{
   uint16_t collision;
   for (struct DynamicCollisionGroup *iterator = g_dynamicMap->collision, *end = g_dynamicMap->collision + g_dynamicMap->collisionQuantity;; ++iterator)
   {
      if (iterator >= end)
      {
         collision = g_dynamicMap->collisionQuantity;
         ++(g_dynamicMap->collisionQuantity);
         break;
      }
      if ((iterator->flags & 0x1) == 0x0)
      {
         collision = iterator - g_dynamicMap->collision;
         break;
      }
   }
   cceUpdateCollisionDynamicMap2D(collision, group1ID, isGroup1BelongsToCurrentMap2D, group2ID, isGroup2BelongsToCurrentMap2D);
   return collision;
}

CCE_PUBLIC_OPTIONS void cceDeleteCollisionDynamicMap2D (uint16_t ID)
{
   (g_dynamicMap->collision + ID)->flags = 0x0;
}

static inline struct Timer* getTimerDynamicMap2D (uint16_t ID)
{
   if (ID > g_dynamicMap->timersQuantityAllocated)
   {
      CCE_REALLOC_ARRAY_ZEROED(g_dynamicMap->timers, ID);
   }
   if (ID > g_dynamicMap->timersQuantity)
   {
      g_dynamicMap->timersQuantity = ID + 1;
   }
   return g_dynamicMap->timers + ID;
}

CCE_PUBLIC_OPTIONS void cceSetTimerDelayDynamicMap2D (uint16_t ID, float delay)
{
   struct Timer *timer = getTimerDynamicMap2D(ID);
   timer->delay = delay;
}

CCE_PUBLIC_OPTIONS void cceStartTimerDynamicMap2D (uint16_t ID)
{
   struct Timer *timer = getTimerDynamicMap2D(ID);
   cceStartTimer(timer);
}

CCE_PUBLIC_OPTIONS uint16_t cceCreateTimerDynamicMap2D (float delay)
{
   uint16_t timerID;
   for (struct Timer *iterator = g_dynamicMap->timers, *end = g_dynamicMap->timers + g_dynamicMap->timersQuantity;; ++iterator)
   {
      if (iterator >= end)
      {
         timerID = g_dynamicMap->timersQuantity;
         ++(g_dynamicMap->timersQuantity);
         break;
      }
      if (iterator->initTime == 0.0)
      {
         timerID = iterator - g_dynamicMap->timers;
         break;
      }
   }
   struct Timer *timer = getTimerDynamicMap2D(timerID);
   timer->initTime = -1.0;
   timer->delay = delay;
   return timerID;
}

CCE_PUBLIC_OPTIONS struct Timer cceGetTimerDynamicMap2D (uint16_t ID)
{
   if (ID > g_dynamicMap->timersQuantity) return (struct Timer) {0.0, 0.0};
   return (struct Timer) {(g_dynamicMap->timers + ID)->initTime, (g_dynamicMap->timers + ID)->delay};
}

static inline void updateLogicElementCommonDynamicMap2D (uint16_t ID, uint8_t logicElementsQuantity, const uint16_t *const logicElements, const cce_enum *const logicElementTypes)
{
   struct ElementLogic *logic = (g_dynamicMap->logic + ID);
   logic->logicElementsQuantity = logicElementsQuantity;
   logic->logicElements = (uint16_t*) realloc(logic->logicElements, logicElementsQuantity * sizeof(uint16_t));
   memcpy(logic->logicElements, logicElements, logicElementsQuantity * sizeof(uint16_t));
   size_t i = 0;
   logic->elementType = 0;
   for (const cce_enum *iterator = logicElementTypes, *end = logicElementTypes + logicElementsQuantity; iterator < end; ++iterator, i += 2)
   {
      switch (*iterator)
      {
         /*case CCE_GLOBAL_BOOL_LOGIC_ELEMENT:
         {
            logic->elementType |= (((uint64_t) 0x0) << i);
            break;
         }*/
         case CCE_PLOT_NUMBER_LOGIC_ELEMENT:
         {
            logic->elementType |= (((uint64_t) 0x1) << i);
            break;
         }
         case CCE_TIMER_LOGIC_ELEMENT:
         {
            logic->elementType |= (((uint64_t) 0x2) << i);
            break;
         }
         case CCE_COLLISION_LOGIC_ELEMENT:
         {
            logic->elementType |= (((uint64_t) 0x3) << i);
            break;
         }
      }
   }
}

CCE_PUBLIC_OPTIONS void cceUpdateLogicElementsByTruthTableDynamicMap2D (const uint16_t ID, const uint8_t logicElementsQuantity, const uint16_t *const logicElements, const cce_enum *const logicElementTypes, const uint_fast16_t *const truthTable)
{
   if (!logicElementsQuantity) return;
   updateLogicElementCommonDynamicMap2D(ID, logicElementsQuantity, logicElements, logicElementTypes);
   struct ElementLogic *logic = (g_dynamicMap->logic + ID);
   uint32_t operationsQuantityInBytes;
   uint8_t isLogicQuantityHigherThan3;
   isLogicQuantityHigherThan3 = logic->logicElementsQuantity > 3;
   operationsQuantityInBytes = ((0x01 << ((logic->logicElementsQuantity) - 3u)) * isLogicQuantityHigherThan3) + !isLogicQuantityHigherThan3;
   logic->operations = (uint_fast16_t*) realloc(logic->operations, operationsQuantityInBytes);
   memcpy(logic->operations, truthTable, operationsQuantityInBytes);
}

CCE_PUBLIC_OPTIONS uint8_t cceUpdateLogicElementsByBooleanExpressionDynamicMap2D (const uint16_t ID, const uint16_t *const logicElements, const cce_enum *const logicElementTypes, const char *const booleanExpression)
{
   uint8_t logicElementsQuantity;
   free((g_dynamicMap->logic + ID)->operations);
   (g_dynamicMap->logic + ID)->operations = cceParseStringToLogicOperations(booleanExpression, &logicElementsQuantity);
   if (!((g_dynamicMap->logic + ID)->operations))
      return 1u;
   updateLogicElementCommonDynamicMap2D(ID, logicElementsQuantity, logicElements, logicElementTypes);
   return 0u;
}

CCE_PUBLIC_OPTIONS void cceUpdateLogicActionsDynamicMap2D (const uint16_t ID, const uint8_t actionsQuantity, uint32_t *actionIDs, const void **actionArgs, const uint32_t *const actionArgSizes)
{
   struct ElementLogic *logic = (g_dynamicMap->logic + ID);
   logic->actionsQuantity = actionsQuantity;
   logic->actionIDs = realloc(logic->actionIDs, actionsQuantity * sizeof(uint32_t));
   memcpy(logic->actionIDs, actionIDs, actionsQuantity * sizeof(uint32_t));
   logic->actionsArgOffsets = realloc(logic->actionsArgOffsets, (actionsQuantity + 1u) * sizeof(uint32_t));
   *(logic->actionsArgOffsets) = 0u;
   const uint32_t *actionArgSize = actionArgSizes;
   for (uint32_t *iterator = (logic->actionsArgOffsets + 1u), *end = (logic->actionsArgOffsets + actionsQuantity); iterator <= end; ++iterator, ++actionArgSize)
   {
      *iterator = *actionArgSize + *(iterator - 1u);
   }
   if (*(logic->actionsArgOffsets + actionsQuantity) > 0u)
   {
      logic->actionsArg = realloc(logic->actionsArg, *(logic->actionsArgOffsets + actionsQuantity));
   }
   else
   {
      free(logic->actionsArg);
      logic->actionsArg = NULL;
      return;
   }
   actionArgSize = actionArgSizes;
   for (uint32_t *iterator = (logic->actionsArgOffsets), *end = (logic->actionsArgOffsets + actionsQuantity); iterator < end; ++iterator, ++actionArgSize, ++actionArgs)
   {
      memcpy((logic->actionsArg + *iterator), *actionArgs, *actionArgSize);
   }
}

CCE_PUBLIC_OPTIONS uint16_t cceCreateLogicDynamicMap2D (void)
{
   for (struct ElementLogic *iterator = g_dynamicMap->logic, *end = g_dynamicMap->logic + g_dynamicMap->logicQuantity; iterator < end; ++iterator)
   {
      if (iterator->actionsArgOffsets == NULL) // The only member that is guaranteed to be initialized when logic is used
      {
         iterator->actionsArgOffsets = malloc(1 * sizeof(uint32_t));
         *(iterator->actionsArgOffsets) = 0;
         return iterator - g_dynamicMap->logic;
      }
   }
   if (g_dynamicMap->logicQuantity >= g_dynamicMap->logicQuantityAllocated)
   {
      CCE_REALLOC_ARRAY_ZEROED(g_dynamicMap->logic, g_dynamicMap->logicQuantity + 1);
   }
   (g_dynamicMap->logic + g_dynamicMap->logicQuantity)->actionsArgOffsets = malloc(1 * sizeof(uint32_t));
   *((g_dynamicMap->logic + g_dynamicMap->logicQuantity)->actionsArgOffsets) = 0;
   return (g_dynamicMap->logicQuantity)++;
}

void cce__terminateDynamicMap2D (void)
{
   if (!g_dynamicMap)
      return;
   
   cce__releaseUBO(g_dynamicMap->UBO_ID);
   cce__releaseTemporaryBools(g_dynamicMap->temporaryBools);
   glDeleteBuffers(1, &(g_dynamicMap->VBO));
   glDeleteVertexArrays(1, &(g_dynamicMap->VAO));

   for (struct DynamicMap2DElement *iterator = g_dynamicMap->elements, *end = g_dynamicMap->elements + g_dynamicMap->elementsQuantity; iterator < end; ++iterator)
   {
      for (uint8_t i = CCE_MOVE_GROUP; i <= CCE_COLLISION_GROUP; ++i)
      {
         uint16_t **elementGroup = getElementGroupPointersDynamicMap2D(i, iterator, NULL);
         free(*elementGroup);
      }
   }
   free(g_dynamicMap->elements);
   
   
   for (uint8_t i = CCE_MOVE_GROUP; i <= CCE_COLLISION_GROUP; ++i)
   {
      struct DynamicElementGroup **groups;
      uint16_t *groupsQuantity;
      uint16_t *groupsQuantityAllocated;
      getGroupPointersDynamicMap2D(i, &groups, &groupsQuantity, &groupsQuantityAllocated);
      for (struct DynamicElementGroup *iterator = *groups, *end = *groups + *groupsQuantity; iterator < end; ++iterator)
      {
         free(iterator->elements);
      }
      free(*groups);
   }
   
   free(g_dynamicMap->collision);
   free(g_dynamicMap->timers);
   
   for (struct ElementLogic *iterator = g_dynamicMap->logic, *end = g_dynamicMap->logic + g_dynamicMap->logicQuantity; iterator < end; ++iterator)
   {
      free(iterator->logicElements);
      free(iterator->operations);
      free(iterator->actionIDs);
      free(iterator->actionsArgOffsets);
      free(iterator->actionsArg);
   }
   free(g_dynamicMap->logic);
   
   llrmlist(&g_dynamicMap->delayedActions);
   
   free(g_dynamicMap);
   g_dynamicMap = NULL;
}
