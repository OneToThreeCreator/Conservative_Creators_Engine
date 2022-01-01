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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

#include "../engine_common.h"
#include "../engine_common_internal.h"
#include "../path_getters.h"
#include "map2D.h"
#include "map2D_internal.h"

#define CCE_DYNAMIC_MAP2D_TO_BE_PROCESSED 0x1

static struct DynamicMap2D *g_dynamicMap = NULL;
static struct DynamicMap2DElement nullElement = {0, 0, 0u, 0u, {0, 0, 0, 0, 0u}, 0u, NULL, 0u, NULL, 0u, NULL, {{0u, 0u}}, 0x4u, 0, 0u, 0u, 0u};
static cce_ubyte flags;

static GLuint makeVAOdynamicMap2D (uint32_t elementsQuantityAllocated, GLuint *VBO)
{
   GLuint VAO;
   glGenVertexArrays(1, &VAO);
   GL_CHECK_ERRORS;
   glGenBuffers(1, VBO);
   GL_CHECK_ERRORS;
   glBindVertexArray(VAO);
   GL_CHECK_ERRORS;
   glBindBuffer(GL_ARRAY_BUFFER, *VBO);
   GL_CHECK_ERRORS;
   
   glBufferData(GL_ARRAY_BUFFER, (sizeof(struct DynamicMap2DElement) * elementsQuantityAllocated), NULL, GL_DYNAMIC_DRAW);
   GL_CHECK_ERRORS;
   
   /* Warning! Members of struct DynamicMap2DElement must follow one by one, otherwise the code below will not work propertly. Probably compiler-dependant. */
   
   /* Pointers */
   glVertexAttribIPointer(0,  2, GL_INT,             sizeof(struct DynamicMap2DElement), (void*)(offsetof(struct DynamicMap2DElement, x)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(1,  2, GL_UNSIGNED_SHORT,  sizeof(struct DynamicMap2DElement), (void*)(offsetof(struct DynamicMap2DElement, width)));
   GL_CHECK_ERRORS;
   glVertexAttribPointer( 2,  1, GL_BYTE,  GL_TRUE,  sizeof(struct DynamicMap2DElement), (void*)(offsetof(struct DynamicMap2DElement, layer)));
   GL_CHECK_ERRORS;
   glVertexAttribPointer( 3,  4, GL_FLOAT, GL_FALSE, sizeof(struct DynamicMap2DElement), (void*)(offsetof(struct DynamicMap2DElement, textureInfo) + offsetof(struct Texture, startX)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(4,  1, GL_UNSIGNED_INT,    sizeof(struct DynamicMap2DElement), (void*)(offsetof(struct DynamicMap2DElement, textureInfo) + offsetof(struct Texture, ID)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(5,  1, GL_UNSIGNED_BYTE,   sizeof(struct DynamicMap2DElement), (void*)(offsetof(struct DynamicMap2DElement, moveGroup)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(6,  1, GL_UNSIGNED_BYTE,   sizeof(struct DynamicMap2DElement), (void*)(offsetof(struct DynamicMap2DElement, flags)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(7,  1, GL_UNSIGNED_BYTE,   sizeof(struct DynamicMap2DElement), (void*)(offsetof(struct DynamicMap2DElement, extensionGroup)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(8,  1, GL_UNSIGNED_BYTE,   sizeof(struct DynamicMap2DElement), (void*)(offsetof(struct DynamicMap2DElement, rotateGroup)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(9,  1, GL_UNSIGNED_BYTE,   sizeof(struct DynamicMap2DElement), (void*)(offsetof(struct DynamicMap2DElement, textureOffsetGroup)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(10, 1, GL_UNSIGNED_BYTE,   sizeof(struct DynamicMap2DElement), (void*)(offsetof(struct DynamicMap2DElement, colorGroup)));
   GL_CHECK_ERRORS;
   
   for (uint8_t i = 0u; i < 11; ++i)
   {
      glEnableVertexAttribArray(i);
      GL_CHECK_ERRORS;
   }
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);
   return VAO;
}

const struct DynamicMap2D* cce__initDynamicMap2D (void)
{
   g_dynamicMap = (struct DynamicMap2D*) malloc(sizeof(struct DynamicMap2D));
   g_dynamicMap->elementsQuantity = 0u;
   g_dynamicMap->elementsAllocatedQuantity = CCE_ALLOCATION_STEP;
   g_dynamicMap->elements = (struct DynamicMap2DElement*) malloc(g_dynamicMap->elementsAllocatedQuantity * sizeof(struct DynamicMap2DElement));
   for (struct DynamicMap2DElement *iterator = g_dynamicMap->elements, *end = g_dynamicMap->elements + g_dynamicMap->elementsAllocatedQuantity; iterator < end; ++iterator)
   {
      iterator->flags = 0x0;
   }
   g_dynamicMap->objectBufferAllocatedSpace = g_dynamicMap->elementsAllocatedQuantity;
   g_dynamicMap->VAO = makeVAOdynamicMap2D(g_dynamicMap->objectBufferAllocatedSpace, &(g_dynamicMap->VBO));
   g_dynamicMap->moveGroupsQuantity = 0u;
   g_dynamicMap->moveGroupsAllocatedQuantity = CCE_ALLOCATION_STEP;
   g_dynamicMap->moveGroups = (struct DynamicElementGroup*) calloc(g_dynamicMap->moveGroupsAllocatedQuantity, sizeof(struct DynamicElementGroup));
   g_dynamicMap->moveGroupValues = (struct cce_ivec2*) calloc(g_dynamicMap->moveGroupsAllocatedQuantity, sizeof(struct cce_uvec2));
   g_dynamicMap->extensionGroupsQuantity = 0u;
   g_dynamicMap->extensionGroupsAllocatedQuantity = CCE_ALLOCATION_STEP;
   g_dynamicMap->extensionGroups = (struct DynamicElementGroup*) calloc(g_dynamicMap->extensionGroupsAllocatedQuantity, sizeof(struct DynamicElementGroup));
   g_dynamicMap->extensionGroupValues = (struct cce_ivec2*) calloc(g_dynamicMap->extensionGroupsAllocatedQuantity, sizeof(struct cce_uvec2));
   g_dynamicMap->collisionGroupsQuantity = 0u;
   g_dynamicMap->collisionGroupsAllocatedQuantity = CCE_ALLOCATION_STEP;
   g_dynamicMap->collisionGroups = (struct DynamicElementGroup*) calloc(g_dynamicMap->collisionGroupsAllocatedQuantity, sizeof(struct DynamicElementGroup));
   g_dynamicMap->collisionQuantity = 0u;
   g_dynamicMap->collisionAllocatedQuantity = CCE_ALLOCATION_STEP;
   g_dynamicMap->collision = (struct DynamicCollisionGroup*) malloc(g_dynamicMap->collisionAllocatedQuantity * sizeof(struct DynamicCollisionGroup));
   g_dynamicMap->timersQuantity = 0u;
   g_dynamicMap->timersAllocatedQuantity = CCE_ALLOCATION_STEP;
   g_dynamicMap->timers = (struct Timer*) malloc(g_dynamicMap->timersAllocatedQuantity * sizeof(struct Timer));
   g_dynamicMap->logicQuantity = 0u;
   g_dynamicMap->logicAllocatedQuantity = CCE_ALLOCATION_STEP;
   g_dynamicMap->logic = (struct ElementLogic*) malloc(g_dynamicMap->logicAllocatedQuantity * sizeof(struct ElementLogic));
   
   g_dynamicMap->temporaryBools = cce__getFreeTemporaryBools();
   
   g_dynamicMap->UBO_ID = cce__getFreeUBO();
   return g_dynamicMap;
}

CCE_PUBLIC_OPTIONS uint8_t cceGetGroupValueDynamicMap2D (cce_enum group_type, uint16_t ID, struct cce_ivec2 *variable)
{
   switch (group_type)
   {
      case CCE_MOVE_GROUP:
      {
         if (ID >= (g_dynamicMap->moveGroupsQuantity))
            return CCE_OUT_OF_BOUNDS;
         variable = (g_dynamicMap->moveGroupValues + ID);
         break;
      }
      case CCE_EXTENSION_GROUP:
      {
         if (ID >= (g_dynamicMap->extensionGroupsQuantity))
            return CCE_OUT_OF_BOUNDS;
         variable = (g_dynamicMap->extensionGroupValues + ID);
         break;
      }
      default:
      {
         return CCE_INCORRECT_ENUM;
      }
   }
   return 0u;
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

static uint8_t getGroupPointersDynamicMap2D (cce_enum group_type, struct DynamicElementGroup ***groups, uint16_t **groupsQuantity, uint16_t **groupsAllocatedQuantity, struct cce_ivec2 ***groupValues)
{
   switch (group_type)
   {
      case CCE_MOVE_GROUP:
      {
         if (groups)
            *groups = &(g_dynamicMap->moveGroups);
         if (groupsQuantity)
            *groupsQuantity = &(g_dynamicMap->moveGroupsQuantity);
         if (groupsAllocatedQuantity)
            *groupsAllocatedQuantity = &(g_dynamicMap->moveGroupsAllocatedQuantity);
         if (groupValues)
            *groupValues = &(g_dynamicMap->moveGroupValues);
         break;
      }
      case CCE_EXTENSION_GROUP:
      {
         if (groups)
            *groups = &(g_dynamicMap->extensionGroups);
         if (groupsQuantity)
            *groupsQuantity = &(g_dynamicMap->extensionGroupsQuantity);
         if (groupsAllocatedQuantity)
            *groupsAllocatedQuantity = &(g_dynamicMap->extensionGroupsAllocatedQuantity);
         if (groupValues)
            *groupValues = &(g_dynamicMap->extensionGroupValues);
         break;
      }
      case CCE_COLLISION_GROUP:
      {
         if (groups)
            *groups = &(g_dynamicMap->collisionGroups);
         if (groupsQuantity)
            *groupsQuantity = &(g_dynamicMap->collisionGroupsQuantity);
         if (groupsAllocatedQuantity)
            *groupsAllocatedQuantity = &(g_dynamicMap->collisionGroupsAllocatedQuantity);
         if (groupValues)
            *groupValues = NULL;
         break;
      }
      default:
      {
         if (groups)
            *groups = NULL;
         if (groupsQuantity)
            *groupsQuantity = NULL;
         if (groupsAllocatedQuantity)
            *groupsAllocatedQuantity = NULL;
         if (groupValues)
            *groupValues = NULL;
         return CCE_INCORRECT_ENUM;
      }
   }
   return 0u;
}

static struct DynamicElementGroup* getGroupDynamicMap2D (cce_enum group_type, uint16_t ID)
{
   uint16_t *groupsQuantity, *groupsAllocatedQuantity;
   struct DynamicElementGroup **groups;
   struct cce_ivec2 **groupValues;
   if (getGroupPointersDynamicMap2D(group_type, &groups, &groupsQuantity, &groupsAllocatedQuantity, &groupValues))
   {
      return NULL;
   }
   if ((*groupsAllocatedQuantity) <= ID)
   {
      uint16_t lastGroupsAllocatedQuantity = (*groupsAllocatedQuantity);
      (*groupsAllocatedQuantity) = (ID & ~(CCE_ALLOCATION_STEP - 1u)) + CCE_ALLOCATION_STEP;
      if (groupValues)
      {
         (*groupValues) = (struct cce_ivec2*) realloc((*groupValues), (*groupsAllocatedQuantity) * sizeof(struct cce_uvec2));
         memset((*groupValues) + lastGroupsAllocatedQuantity, 0u, ((*groupsAllocatedQuantity) - lastGroupsAllocatedQuantity) * sizeof(struct cce_uvec2));
      }
      (*groups) = (struct DynamicElementGroup*) realloc((*groups), (*groupsAllocatedQuantity) * sizeof(struct DynamicElementGroup));
      memset((*groups) + lastGroupsAllocatedQuantity, 0u, ((*groupsAllocatedQuantity) - lastGroupsAllocatedQuantity) * sizeof(struct DynamicElementGroup));
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

   return (struct ElementGroup) {group->elementIDs, group->elementsQuantity};
}

CCE_PUBLIC_OPTIONS struct Map2DCollider cceGetColliderDataDynamicMap2D (uint32_t ID)
{
   if (ID < g_dynamicMap->elementsQuantity)
   {
      return (struct Map2DCollider) {0, 0, 0u, 0u};
   }
   struct DynamicMap2DElement *element = g_dynamicMap->elements + ID;
   struct cce_ivec2 coordOffset = {0, 0}, sizeOffset = {0, 0}, tmp;
   if ((element->flags & 0x2) && !(element->flags & 0x4))
   {
      for (uint16_t *iterator = (g_dynamicMap->elements + ID)->moveGroups, *end = (g_dynamicMap->elements + ID)->moveGroups + (g_dynamicMap->elements + ID)->moveGroupsQuantity;
      iterator < end; ++iterator)
      {
         cceGetGroupValueDynamicMap2D(CCE_MOVE_GROUP, *iterator, &tmp);
         coordOffset.x += tmp.x;
         coordOffset.y += tmp.y;
      }
      for (uint16_t *iterator = (g_dynamicMap->elements + ID)->extensionGroups, *end = (g_dynamicMap->elements + ID)->extensionGroups + (g_dynamicMap->elements + ID)->extensionGroupsQuantity;
      iterator < end; ++iterator)
      {
         cceGetGroupValueDynamicMap2D(CCE_EXTENSION_GROUP, *iterator, &tmp);
         sizeOffset.x += tmp.x;
         sizeOffset.y += tmp.y;
      }
   }
   return (struct Map2DCollider) {element->x - coordOffset.x, element->y - coordOffset.y, element->width - sizeOffset.x, element->height - sizeOffset.y};
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

CCE_PUBLIC_OPTIONS uint8_t cceCreateGroupDynamicMap2D (cce_enum group_type, uint16_t elementsQuantity, uint32_t *elementIDs, uint16_t *emptyGroupID)
{
   struct DynamicElementGroup *group;
   struct DynamicElementGroup **groups;
   {
      uint16_t *groupsQuantity;
      if (getGroupPointersDynamicMap2D(group_type, &groups, &groupsQuantity, NULL, NULL))
      {
         ptrdiff_t error = group_type;  
         cce__errorPrint("ENGINE::GET_EMPTY_GROUP_FROM_DYNAMIC_MAP2D::INCORRECT_ENUM:\ngroup_type: %i passed to the function is not valid", (void*) error);
         return CCE_INCORRECT_ENUM;
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
   if (elementsQuantity > group->elementsAllocatedQuantity)
   {
      group->elementsAllocatedQuantity += CCE_ALLOCATION_STEP;
      group->elementIDs = realloc(group->elementIDs, group->elementsAllocatedQuantity * sizeof(uint32_t));
   }
   if (elementIDs)
   {
      group->elementsQuantity = elementsQuantity;
      memcpy(group->elementIDs, elementIDs, elementsQuantity * sizeof(uint32_t));
   }
   *emptyGroupID = group - (*groups);
   return 0u;
}

CCE_PUBLIC_OPTIONS uint8_t cceAddElementInGroupDynamicMap2D (cce_enum group_type, uint16_t ID, uint32_t elementID)
{
   struct DynamicElementGroup *group = getGroupDynamicMap2D(group_type, ID);
   if (!group)
   {
      ptrdiff_t error = group_type;  
      cce__errorPrint("ENGINE::ADD_ELEMENT_IN_GROUP_FROM_DYNAMIC_MAP2D::INCORRECT_ENUM:\ngroup_type: %i passed to the function is not valid", (void*) error);
      return CCE_INCORRECT_ENUM;
   }
   for (uint32_t *iterator = group->elementIDs, *end = group->elementIDs + group->elementsQuantity; iterator < end; ++iterator)
   {
      if ((*iterator) == elementID)
      {
         return 0u;
      }
   }
   ++(group->elementsQuantity);
   if (group->elementsQuantity > group->elementsAllocatedQuantity)
   {
      group->elementsAllocatedQuantity += CCE_ALLOCATION_STEP;
      group->elementIDs = realloc(group->elementIDs, (group->elementsAllocatedQuantity) * sizeof(uint32_t));
   }
   *(group->elementIDs) = elementID;
   
   uint16_t  *elementGroupLength;
   uint16_t **elementGroup = getElementGroupPointersDynamicMap2D(group_type, g_dynamicMap->elements + elementID, &elementGroupLength);
   ++(*elementGroupLength);
   *elementGroup = realloc(*elementGroup, *elementGroupLength * sizeof(uint16_t));
   *((*elementGroup) + (*elementGroupLength) - 1u) = ID;
   return 0u;
}

static void updateMap2DElementDynamicMap2D (struct Map2DElementDev *element, uint32_t ID, uint8_t hasCollider)
{
   struct DynamicMap2DElement *dynamicElement = g_dynamicMap->elements + ID;
   dynamicElement->x = element->x;
   dynamicElement->y = element->y;
   dynamicElement->width = element->width;
   dynamicElement->height = element->height;
   dynamicElement->textureInfo.startX = element->textureInfo.startX;
   dynamicElement->textureInfo.startY = element->textureInfo.startY;
   dynamicElement->textureInfo.endX = element->textureInfo.endX;
   dynamicElement->textureInfo.endY = element->textureInfo.endY;
   dynamicElement->textureInfo.ID = element->textureInfo.ID;
   dynamicElement->layer = element->layer;
   dynamicElement->textureOffsetGroup = element->textureOffsetGroup;
   dynamicElement->colorGroup = element->colorGroup;
   dynamicElement->rotateGroup = element->rotateGroup;
   dynamicElement->moveGroup = element->moveGroup;
   dynamicElement->extensionGroup = element->extensionGroup;
   if (hasCollider)
   {
      if (element->moveGroup)
         cceAddElementInGroupDynamicMap2D(CCE_MOVE_GROUP, element->moveGroup, ID);

      if (element->extensionGroup)
         cceAddElementInGroupDynamicMap2D(CCE_EXTENSION_GROUP, element->extensionGroup - 1u, ID);

      if (element->isGlobalOffset)
         cceAddElementInGroupDynamicMap2D(CCE_MOVE_GROUP, 0u, ID);
   }
   dynamicElement->flags = 0x1u + (hasCollider << 1u) + 0x4u + 0x8u + ((element->isGlobalOffset) << 4u);
   
   flags |= CCE_DYNAMIC_MAP2D_TO_BE_PROCESSED;
}

#define SET_MAP2DELEMENTGROUPS_TO_NULL_DYNAMICMAP2D(element) \
(element)->moveGroupsQuantity = 0u;      \
(element)->moveGroups = NULL;            \
(element)->extensionGroupsQuantity = 0u; \
(element)->extensionGroups = NULL;       \
(element)->collisionGroupsQuantity = 0u; \
(element)->collisionGroups = NULL

CCE_PUBLIC_OPTIONS uint32_t cceCreateMap2DElementDynamicMap2D (struct Map2DElementDev *element, uint8_t hasCollider)
{
   for (struct DynamicMap2DElement *iterator = g_dynamicMap->elements, *end = g_dynamicMap->elements + g_dynamicMap->elementsQuantity; iterator < end; ++iterator)
   {
      if (!(iterator->flags & 0x1))
      {
         SET_MAP2DELEMENTGROUPS_TO_NULL_DYNAMICMAP2D(iterator);
         updateMap2DElementDynamicMap2D(element, (uint32_t) (iterator - g_dynamicMap->elements), hasCollider);
         return (uint32_t) (iterator - g_dynamicMap->elements);
      }
   }
   ++(g_dynamicMap->elementsQuantity);
   if (g_dynamicMap->elementsQuantity > g_dynamicMap->elementsAllocatedQuantity)
   {
      g_dynamicMap->elementsAllocatedQuantity += CCE_ALLOCATION_STEP;
      g_dynamicMap->elements = realloc(g_dynamicMap->elements, g_dynamicMap->elementsAllocatedQuantity * sizeof(struct DynamicMap2DElement));
      for (struct DynamicMap2DElement *iterator = g_dynamicMap->elements, *end = g_dynamicMap->elements + g_dynamicMap->elementsAllocatedQuantity; iterator < end; ++iterator)
      {
         iterator->flags = 0x0;
      }
   }
   SET_MAP2DELEMENTGROUPS_TO_NULL_DYNAMICMAP2D(g_dynamicMap->elements + g_dynamicMap->elementsQuantity - 1u);
   updateMap2DElementDynamicMap2D(element, g_dynamicMap->elementsQuantity - 1u, hasCollider);
   return g_dynamicMap->elementsQuantity - 1u;
}

CCE_PUBLIC_OPTIONS uint8_t cceDeleteElementFromGroupDynamicMap2D (cce_enum group_type, uint16_t ID, uint32_t elementID)
{
   struct DynamicElementGroup *group;
   {
      struct DynamicElementGroup **groups;
      uint16_t *groupsQuantity;
      uint16_t *groupsAllocatedQuantity;
      if (!(getGroupPointersDynamicMap2D(group_type, &groups, &groupsQuantity, &groupsAllocatedQuantity, NULL)))
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
   for (uint32_t *iterator = group->elementIDs, *end = group->elementIDs + group->elementsQuantity; iterator < end; ++iterator)
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
         *elementGroup = realloc(*elementGroup, *elementGroupLength * sizeof(uint16_t));
         break;
      }
   }
   return 0u;
}

static void deleteGroupsFromElementDynamicMap2D (struct DynamicMap2DElement *element)
{
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

CCE_PUBLIC_OPTIONS void cceReplaceMap2DElementDynamicMap2D (struct Map2DElementDev *element, uint32_t ID, uint8_t hasCollider)
{
   deleteGroupsFromElementDynamicMap2D((g_dynamicMap->elements + ID));
   updateMap2DElementDynamicMap2D(element, ID, hasCollider);
}

CCE_PUBLIC_OPTIONS void cceDeleteMap2DElementDynamicMap2D (uint32_t ID)
{
   deleteGroupsFromElementDynamicMap2D((g_dynamicMap->elements + ID));
   memcpy((g_dynamicMap->elements + ID), &nullElement, sizeof(struct DynamicMap2DElement));
   (g_dynamicMap->elements + ID)->flags = 0x0u;
   flags |= CCE_DYNAMIC_MAP2D_TO_BE_PROCESSED;
   return;
}

CCE_PUBLIC_OPTIONS uint8_t cceDeleteGroupDynamicMap2D (cce_enum group_type, uint16_t ID)
{
   struct DynamicElementGroup **groups;
   uint16_t *groupsQuantity;
   uint16_t *groupsAllocatedQuantity;
   if (!(getGroupPointersDynamicMap2D(group_type, &groups, &groupsQuantity, &groupsAllocatedQuantity, NULL)))
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
   ((*groups) + ID)->elementsQuantity = 0u;
   for (uint32_t *iterator = ((*groups) + ID)->elementIDs, *end = ((*groups) + ID)->elementIDs + elementsQuantity; iterator < end; ++iterator)
   {
      cceDeleteElementFromGroupDynamicMap2D(group_type, ID, *iterator);
   }
   return 0u;
}

void cce__processDynamicMap2DElements (void)
{
   if (!(flags & CCE_DYNAMIC_MAP2D_TO_BE_PROCESSED))
   {
      return;
   }
   glBindVertexArray(g_dynamicMap->VAO);
   
   if (g_dynamicMap->objectBufferAllocatedSpace < g_dynamicMap->elementsAllocatedQuantity)
   {
      uint32_t newBuffer;
      glGenBuffers(1, &newBuffer);
      glBindBuffer(GL_ARRAY_BUFFER, newBuffer);
      glBufferData(GL_ARRAY_BUFFER, g_dynamicMap->elementsAllocatedQuantity * sizeof(struct DynamicMap2DElement), NULL, GL_DYNAMIC_DRAW);
      glBindBuffer(GL_COPY_READ_BUFFER, g_dynamicMap->VBO);
      glCopyBufferSubData(GL_COPY_READ_BUFFER, GL_ARRAY_BUFFER, 0u, 0u, (g_dynamicMap->objectBufferAllocatedSpace) * sizeof(struct DynamicMap2DElement));
      glDeleteBuffers(1u, &g_dynamicMap->VBO);
      g_dynamicMap->VBO = newBuffer;
      g_dynamicMap->objectBufferAllocatedSpace = g_dynamicMap->elementsAllocatedQuantity;
   }
   else
   {
      glBindBuffer(GL_ARRAY_BUFFER, g_dynamicMap->VBO);
   }
   void *bufferPtr = glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY);
   
   for (struct DynamicMap2DElement *iterator = g_dynamicMap->elements, *end = (g_dynamicMap->elements) + (g_dynamicMap->elementsQuantity); iterator < end; ++iterator)
   {
      if (iterator->flags & 0x4u)
      {
         cce__loadTextureDynamicMap2D(iterator);
         memcpy(bufferPtr + (iterator - g_dynamicMap->elements), iterator, sizeof(struct DynamicMap2DElement));
         iterator->textureElementReliesOn = iterator->textureInfo.ID;
         uint8_t isMoveGroup = iterator->moveGroup > 0u, isGlobalOffset = iterator->flags & CCE_GLOBAL_OFFSET_MASK;
         iterator->moveGroupsQuantity += isMoveGroup + isGlobalOffset;
         if (isMoveGroup || isGlobalOffset)
         {
            iterator->moveGroups = realloc(iterator->moveGroups, iterator->moveGroupsQuantity * sizeof(uint16_t));
            uint8_t i = 0u;
            *(iterator->moveGroups) = iterator->moveGroup;
            i += isMoveGroup;
            if (isGlobalOffset)
               *(iterator->moveGroups + i) = 0u;
         }
         iterator->extensionGroupsQuantity += iterator->extensionGroup > 0u;
         if (iterator->extensionGroup)
         {
            iterator->extensionGroups = realloc(iterator->extensionGroups, sizeof(uint16_t));
            *(iterator->extensionGroups) = iterator->extensionGroup - 1u;
         }
         iterator->flags ^= 0x4u;
         struct cce_ivec2 globalGroupOffset = {0, 0}, tmp;
         if (iterator->flags & 0x2)
         {
            for (uint16_t *jiterator = iterator->moveGroups, *end = iterator->moveGroups + iterator->moveGroupsQuantity;
            jiterator < end; ++jiterator)
            {
               cceGetGroupValueDynamicMap2D(CCE_MOVE_GROUP, *jiterator, &tmp);
               globalGroupOffset.x += tmp.x;
               globalGroupOffset.y += tmp.y;
            }
            iterator->x += globalGroupOffset.x;
            iterator->y += globalGroupOffset.y;
            for (uint16_t *jiterator = iterator->extensionGroups, *end = iterator->extensionGroups + iterator->extensionGroupsQuantity;
            jiterator < end; ++jiterator)
            {
               cceGetGroupValueDynamicMap2D(CCE_EXTENSION_GROUP, *jiterator, &tmp);
               globalGroupOffset.x += tmp.x;
               globalGroupOffset.y += tmp.y;
            }
            iterator->width += globalGroupOffset.x;
            iterator->height += globalGroupOffset.y;
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

cce_ubyte cce__fourthLogicTypeFuncDynamicMap2D(uint16_t ID, va_list argp)
{
   struct Map2D *map = (struct Map2D*) va_arg(argp, struct Map2D*);
   uint32_t *group1lastID, *group1IDs, *group2firstID, *group2IDs, *group2lastID;
   cce_void *elements1, *elements2;
   uint8_t element1Size, element2Size, isDifferent;
   if ((g_dynamicMap->collision + ID)->flags & 0x2)
   {
      group1IDs = ((map->collisionGroups + (g_dynamicMap->collision + ID)->group1)->elementIDs);
      group1lastID = (group1IDs + (map->collisionGroups + (g_dynamicMap->collision + ID)->group1)->elementsQuantity - 1u);
      elements1 = (cce_void*) map->colliders;
      element1Size = sizeof(struct Map2DCollider);
   }
   else
   {
      group1IDs = ((map->collisionGroups + (g_dynamicMap->collision + ID)->group1)->elementIDs);
      group1lastID = (group1IDs + (map->collisionGroups + (g_dynamicMap->collision + ID)->group1)->elementsQuantity - 1u);
      elements1 = (cce_void*) g_dynamicMap->elements;
      element1Size = sizeof(struct DynamicMap2DElement);
   }
   if ((g_dynamicMap->collision + ID)->flags & 0x4)
   {
      group2firstID = ((map->collisionGroups + (g_dynamicMap->collision + ID)->group2)->elementIDs);
      group2lastID = (group2firstID + (map->collisionGroups + (g_dynamicMap->collision + ID)->group2)->elementsQuantity - 1u);
      elements2 = (cce_void*) map->colliders;
      element2Size = sizeof(struct Map2DCollider);
   }
   else
   {
      group2firstID = ((map->collisionGroups + (g_dynamicMap->collision + ID)->group2)->elementIDs);
      group2lastID = (group2firstID + (map->collisionGroups + (g_dynamicMap->collision + ID)->group2)->elementsQuantity - 1u);
      elements2 = (cce_void*) g_dynamicMap->elements;
      element2Size = sizeof(struct DynamicMap2DElement);
   }
   isDifferent = elements1 != elements2;
   while (group1IDs <= group1lastID)
   {
      group2IDs = group2firstID;
      while (group2IDs <= group2lastID)
      {
         // Identical iterating with 2 different structs without duplication. Dangerous! Also ignore comparing with itself.
         if ((isDifferent || (*group1IDs != *group2IDs)) &&
         cceCheckCollisionMap2D((struct Map2DCollider*) (elements1 + (*group1IDs * element1Size)), (struct Map2DCollider*) (elements2 + (*group2IDs * element2Size))))
         {
            return 1u;
         }
         ++group2IDs;
      }
      ++group1IDs;
   }
   return 0u;
}

CCE_PUBLIC_OPTIONS void cceUpdateCollisionDynamicMap2D (uint16_t ID, uint16_t group1ID, cce_ubyte isGroup1BelongsToCurrentMap2D, uint16_t group2ID, cce_ubyte isGroup2BelongsToCurrentMap2D)
{
   (g_dynamicMap->collision + ID)->group1 = group1ID;
   (g_dynamicMap->collision + ID)->group2 = group2ID;
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
      if (iterator->flags & 0x1)
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

CCE_PUBLIC_OPTIONS void cceSetTimerDynamicMap2D (uint16_t ID, float delay)
{
   if (ID > g_dynamicMap->timersAllocatedQuantity)
   {
      uint16_t lastTimersAllocatedQuantity = g_dynamicMap->timersAllocatedQuantity;
      g_dynamicMap->timersAllocatedQuantity = (ID & ~(CCE_ALLOCATION_STEP - 1u)) + CCE_ALLOCATION_STEP;
      g_dynamicMap->timers = (struct Timer*) realloc(g_dynamicMap->timers, g_dynamicMap->timersAllocatedQuantity * sizeof(struct Timer));
      memset(g_dynamicMap->timers + lastTimersAllocatedQuantity, 0u, (g_dynamicMap->timersAllocatedQuantity - lastTimersAllocatedQuantity) * sizeof(struct Timer));
   }
   if (ID > g_dynamicMap->timersQuantity)
   {
      ++(g_dynamicMap->timersQuantity);
   }
   (g_dynamicMap->timers + ID)->initTime = *cceCurrentTime;
   (g_dynamicMap->timers + ID)->delay = delay;
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
      if (!(iterator->initTime))
      {
         timerID = iterator - g_dynamicMap->timers;
         break;
      }
   }
   cceSetTimerDynamicMap2D(timerID, delay);
   return timerID;
}

CCE_PUBLIC_OPTIONS void cceResetTimerDynamicMap2D (uint16_t ID)
{
   if (ID > g_dynamicMap->timersQuantity) return;
   (g_dynamicMap->timers + ID)->initTime = *cceCurrentTime;
}

CCE_PUBLIC_OPTIONS int8_t cceGetTimerDynamicMap2D (uint16_t ID)
{
   if (ID > g_dynamicMap->timersQuantity) return -1;
   struct Timer *timer = (g_dynamicMap->timers + ID);
   return ((*cceCurrentTime) >= (timer->initTime + timer->delay));
}

static void updateLogicElementCommonDynamicMap2D (uint16_t ID, uint8_t logicElementsQuantity, const uint16_t *const logicElements, const cce_enum *const logicElementTypes)
{
   struct ElementLogic *logic = (g_dynamicMap->logic + ID);
   logic->logicElementsQuantity = logicElementsQuantity;
   logic->logicElements = (uint16_t*) malloc(logicElementsQuantity * sizeof(uint16_t));
   memcpy(logic->logicElements, logicElements, logicElementsQuantity * sizeof(uint16_t));
}

CCE_PUBLIC_OPTIONS void cceUpdateLogicElementByTruthTableDynamicMap2D (const uint16_t ID, const uint8_t logicElementsQuantity, const uint16_t *const logicElements, const cce_enum *const logicElementTypes, const uint_fast16_t *const truthTable)
{
   if (!logicElementsQuantity) return;
   updateLogicElementCommonDynamicMap2D(ID, logicElementsQuantity, logicElements, logicElementTypes);
   struct ElementLogic *logic = (g_dynamicMap->logic + ID);
   uint32_t operationsQuantityInBytes;
   uint8_t isLogicQuantityHigherThan3;
   isLogicQuantityHigherThan3 = logic->logicElementsQuantity > 3;
   operationsQuantityInBytes = (0x01 << ((logic->logicElementsQuantity) - 3u) * isLogicQuantityHigherThan3) + !isLogicQuantityHigherThan3;
   (logic->operations) = (uint_fast16_t*) malloc(operationsQuantityInBytes);
   memcpy(logic->operations, truthTable, operationsQuantityInBytes);
}

CCE_PUBLIC_OPTIONS uint8_t cceUpdateLogicElementsByBooleanExpressionDynamicMap2D (const uint16_t ID, const uint16_t *const logicElements, const cce_enum *const logicElementTypes, const char *const booleanExpression)
{
   uint8_t logicElementsQuantity;
   ((g_dynamicMap->logic + ID)->operations) = cceParseStringToLogicOperations(booleanExpression, &logicElementsQuantity);
   if (!((g_dynamicMap->logic + ID)->operations))
      return 1u;
   updateLogicElementCommonDynamicMap2D(ID, logicElementsQuantity, logicElements, logicElementTypes);
   return 0u;
}

CCE_PUBLIC_OPTIONS void cceUpdateLogicActionsDynamicMap2D (const uint16_t ID, const uint8_t actionsQuantity, uint32_t *actionIDs, const void **actionArgs, const size_t *const actionArgSizes)
{
   struct ElementLogic *logic = (g_dynamicMap->logic + ID);
   logic->actionsQuantity = actionsQuantity;
   logic->actionIDs = malloc(actionsQuantity * sizeof(uint32_t));
   memcpy(logic->actionIDs, actionIDs, actionsQuantity * sizeof(uint32_t));
   logic->actionsArgOffsets = malloc((actionsQuantity + 1u) * sizeof(uint32_t));
   *(logic->actionsArgOffsets) = 0u;
   const size_t *actionArgSize = actionArgSizes;
   for (uint32_t *iterator = (logic->actionsArgOffsets + 1u), *end = (logic->actionsArgOffsets + actionsQuantity); iterator <= end; ++iterator, ++actionArgSize)
   {
      *iterator = *actionArgSize + *(iterator - 1u);
   }
   logic->actionsArg = malloc(*(logic->actionsArgOffsets + actionsQuantity));
   actionArgSize = actionArgSizes;
   for (uint32_t *iterator = (logic->actionsArgOffsets), *end = (logic->actionsArgOffsets + actionsQuantity); iterator < end; ++iterator, ++actionArgSize, ++actionArgs)
   {
      memcpy((logic->actionsArg + *iterator), *actionArgs, *actionArgSize);
   }
}

CCE_PUBLIC_OPTIONS uint16_t cceFindFreeLogicDynamicMap2D (void)
{
   for (struct ElementLogic *iterator = g_dynamicMap->logic, *end = g_dynamicMap->logic + g_dynamicMap->logicQuantity; iterator < end; ++iterator)
   {
      if (!(iterator->logicElementsQuantity))
      {
         return iterator - g_dynamicMap->logic;
      }
   }
   if (g_dynamicMap->logicQuantity >= g_dynamicMap->logicAllocatedQuantity)
   {
      g_dynamicMap->logicAllocatedQuantity += CCE_ALLOCATION_STEP;
      g_dynamicMap->logic = realloc(g_dynamicMap->logic, g_dynamicMap->logicAllocatedQuantity);
      memset(g_dynamicMap->logic + g_dynamicMap->logicAllocatedQuantity - CCE_ALLOCATION_STEP, 0u, CCE_ALLOCATION_STEP);
   }
   return (g_dynamicMap->logicQuantity)++;
}

void cce__terminateDynamicMap2D (void)
{
   if (!g_dynamicMap)
      return;
   
   cce__releaseUBO(g_dynamicMap->UBO_ID);
   cce__releaseTemporaryBools(g_dynamicMap->temporaryBools);
   glDeleteBuffers(1u, &(g_dynamicMap->VBO));
   glDeleteVertexArrays(1u, &(g_dynamicMap->VAO));

   for (struct DynamicMap2DElement *iterator = g_dynamicMap->elements, *end = g_dynamicMap->elements + g_dynamicMap->elementsQuantity; iterator < end; ++iterator)
   {
      for (uint8_t i = 1u; i <= 3u; ++i)
      {
         uint16_t **elementGroup = getElementGroupPointersDynamicMap2D(i, iterator, NULL);
         free(*elementGroup);
      }
   }
   free(g_dynamicMap->elements);
   
   
   for (uint8_t i = 1u; i <= 3u; ++i)
   {
      struct DynamicElementGroup **groups;
      uint16_t *groupsQuantity;
      uint16_t *groupsAllocatedQuantity;
      struct cce_ivec2 **groupValues;
      getGroupPointersDynamicMap2D(i, &groups, &groupsQuantity, &groupsAllocatedQuantity, &groupValues);
      for (struct DynamicElementGroup *iterator = *groups, *end = *groups + *groupsQuantity; iterator < end; ++iterator)
      {
         free(iterator->elementIDs);
      }
      if (groupValues)
         free(*groupValues);
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
   
   free(g_dynamicMap);
   g_dynamicMap = NULL;
}