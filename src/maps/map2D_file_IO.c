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
#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__TOS_WIN__) || defined(__WINDOWS__) || \
      defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN__)
#include <malloc.h>
#else
#include <alloca.h>
#endif

#include "../engine_common.h"
#include "../engine_common_internal.h"
#include "../path_getters.h"
#include "map2D.h"
#include "map2D_internal.h"
#include "base_actions.h"

static char *mapPath = NULL;
size_t mapPathLength;

static void (*cce_fileParseFunc)(FILE*, uint16_t);
static void (*cce_callbackOnFreeing)(uint16_t);
static void (***cce_actions)(void*);

void cce__initMap2DLoaders (void (***doAction)(void*))
{
   cce_actions = doAction;
}

CCE_PUBLIC_OPTIONS void cceSetMap2Dpath (const char *path)
{
   if (!path || *path == '\0')
      return;
   free(mapPath);
   mapPath = cce__createNewPathFromOldPath(path, "map_", 9u);
   mapPathLength = strlen(mapPath);
}

CCE_PUBLIC_OPTIONS void cceRegisterMapLoadingCallback (void (*fileParseFunc)(FILE*, uint16_t))
{
   cce_fileParseFunc = fileParseFunc;
}

CCE_PUBLIC_OPTIONS void cceRegisterMapFreeingCallback (void (*callbackOnFreeing)(uint16_t))
{
   cce_callbackOnFreeing = callbackOnFreeing;
}

CCE_PUBLIC_OPTIONS void cceFreeMap2D (struct Map2D *map)
{
   if (!map)
      return;
   if (map->VAO)
      glDeleteVertexArrays(1u, &(map->VAO));
   if (map->VBO)
      glDeleteBuffers(1u, &(map->VBO));
   if (map->elementsQuantity)
      cce__releaseUBO(map->UBO_ID);
   if (map->collidersQuantity)
      free(map->colliders);
   if (map->moveGroupsQuantity)
   {
      for (struct ElementGroup *iterator = map->moveGroups, *end = (map->moveGroups + map->moveGroupsQuantity - 1u); iterator <= end; ++iterator)
      {
         free(iterator->elementIDs);
      }
      free(map->moveGroups);
   }
   if (map->extensionGroupsQuantity)
   {
      for (struct ElementGroup *iterator = map->extensionGroups, *end = (map->extensionGroups + map->extensionGroupsQuantity - 1u); iterator <= end; ++iterator)
      {
         free(iterator->elementIDs);
      }
      free(map->extensionGroups);
   }
   if (map->collisionGroupsQuantity)
   {
      for (struct ElementGroup *iterator = map->collisionGroups, *end = (map->collisionGroups + map->collisionGroupsQuantity - 1u); iterator <= end; ++iterator)
      {
         free(iterator->elementIDs);
      }
      free(map->collisionGroups);
   }
   if (map->collisionQuantity)
   {
      free(map->collision);
      free(map->collisionCache);
   }
   if (map->timersQuantity)
      free(map->timers);
   if (map->logicQuantity)
   {
      struct ElementLogic *end = (map->logic + map->logicQuantity);
      for (struct ElementLogic *iterator = (map->logic); iterator < end; ++iterator)
      {
         free(iterator->logicElements);
         free(iterator->operations);
         free(iterator->actionIDs);
         free(iterator->actionsArgOffsets);
         free(iterator->actionsArg);
      }
      free(map->logic);
   }
   if ((map->texturesMapReliesOn))
   {
      cce__releaseTextures(map->texturesMapReliesOn, map->texturesMapReliesOnQuantity);
   }
   if ((map->exitMapsQuantity))
   {
      free(map->exitMaps);
   }
   if (cce_callbackOnFreeing)
   {
      if (map->logicQuantity)
      {
         cce__setCurrentTemporaryBools(map->temporaryBools);
      }
      cce_callbackOnFreeing(map->ID);
   }
   if (map->logicQuantity)
   {
      cce__releaseTemporaryBools(map->temporaryBools);
   }
   free(map);
}

CCE_PUBLIC_OPTIONS void cceFreeMap2Ddev (struct Map2Ddev *map)
{
   if (!map)
      return;
   if ((map->elementsQuantity))
      free(map->elements);
   if ((map->collidersQuantity))
      free(map->colliders);
   if ((map->moveGroupsQuantity))
   {
      for (struct ElementGroup *iterator = map->moveGroups, *end = (map->moveGroups + map->moveGroupsQuantity - 1u); iterator <= end; ++iterator)
      {
         free(iterator->elementIDs);
      }
      free(map->moveGroups);
   }
   if ((map->extensionGroupsQuantity))
   {
      for (struct ElementGroup *iterator = map->extensionGroups, *end = (map->extensionGroups + map->extensionGroupsQuantity - 1u); iterator <= end; ++iterator)
      {
         free(iterator->elementIDs);
      }
      free(map->extensionGroups);
   }
   if ((map->collisionGroupsQuantity))
   {
      for (struct ElementGroup *iterator = map->collisionGroups, *end = (map->collisionGroups + map->collisionGroupsQuantity - 1u); iterator <= end; ++iterator)
      {
         free(iterator->elementIDs);
      }
      free(map->collisionGroups);
   }
   if ((map->collisionQuantity))
      free(map->collision);
   if ((map->timersQuantity))
      free(map->delaysOfTimers);
   if ((map->logicQuantity))
   {
      struct ElementLogic *end = (map->logic + map->logicQuantity - 1u);
      for (struct ElementLogic *iterator = (map->logic); iterator <= end; ++iterator)
      {
         free(iterator->logicElements);
         free(iterator->operations);
         free(iterator->actionIDs);
         free(iterator->actionsArgOffsets);
         free(iterator->actionsArg);
      }
      free(map->logic);
   }
   if ((map->actionsQuantity))
   {
      free(map->actionIDs);
      free(map->actionsArgOffsets);
      free(map->actionsArg);
   }
   if ((map->exitMapsQuantity))
   {
      free(map->exitMaps);
   }
   if (cce_callbackOnFreeing) cce_callbackOnFreeing(map->ID);
   free(map);
}

static GLuint makeVAOmap2D (struct Map2DElement *elements, uint32_t elementsQuantity, GLuint *VBO)
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
   
   glBufferData(GL_ARRAY_BUFFER, ((sizeof(struct Map2DElement) +  3 * sizeof(uint8_t)) * elementsQuantity), elements, GL_STATIC_DRAW);
   GL_CHECK_ERRORS;
   
   /* Pointers */
   glVertexAttribIPointer(0, 2, GL_INT, sizeof(struct Map2DElement), (void*)(offsetof(struct Map2DElement, x)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(1, 2, GL_UNSIGNED_SHORT, sizeof(struct Map2DElement), (void*)(offsetof(struct Map2DElement, width)));
   GL_CHECK_ERRORS;
   glVertexAttribPointer(2, 1, GL_BYTE, GL_TRUE, sizeof(struct Map2DElement), (void*)(offsetof(struct Map2DElement, layer)));
   GL_CHECK_ERRORS;
   glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(struct Map2DElement), (void*)(offsetof(struct Map2DElement, textureInfo) + offsetof(struct Texture, startX)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(4, 1, GL_UNSIGNED_INT, sizeof(struct Map2DElement), (void*)(offsetof(struct Map2DElement, textureInfo) + offsetof(struct Texture, ID)));
   GL_CHECK_ERRORS;
   
   glVertexAttribIPointer(5, 1, GL_UNSIGNED_BYTE, 2u * sizeof(uint8_t), (void*)(elementsQuantity * sizeof(struct Map2DElement)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(6, 1, GL_UNSIGNED_BYTE, 2u * sizeof(uint8_t), (void*)(elementsQuantity * sizeof(struct Map2DElement) + sizeof(uint8_t)));
   GL_CHECK_ERRORS;
   
   glVertexAttribIPointer(7, 1, GL_UNSIGNED_BYTE, sizeof(uint8_t), (void*)(elementsQuantity * (sizeof(struct Map2DElement) + elementsQuantity * 2u * sizeof(uint8_t))));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(8, 1, GL_UNSIGNED_BYTE, sizeof(struct Map2DElement), (void*)(offsetof(struct Map2DElement, rotateGroup)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(9, 1, GL_UNSIGNED_BYTE, sizeof(struct Map2DElement), (void*)(offsetof(struct Map2DElement, textureOffsetGroup)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(10, 1, GL_UNSIGNED_BYTE, sizeof(struct Map2DElement), (void*)(offsetof(struct Map2DElement, colorGroup)));
   GL_CHECK_ERRORS;
   
   /* I'm lazy */
   for (uint8_t i = 0u; i < 11; ++i)
   {
      glEnableVertexAttribArray(i);
      GL_CHECK_ERRORS;
   }
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glBindVertexArray(0);
   return VAO;
}

static void convertCCEgroupsToGLgroups (uint16_t groupsQuantity, struct ElementGroup *groups, uint8_t *glGroups, uint8_t glGroupsStep, uint32_t elementsWithoutColliderQuantity, uint32_t elementsQuantity)
{
   uint8_t i = 1u;
   uint32_t offset = 0u;
   struct ElementGroup *end = (groups + (MIN(256u, groupsQuantity)));
   while (groups < end)
   {
      if (groups->elementsQuantity) for (uint32_t *j = groups->elementIDs, *jend = (groups->elementIDs + groups->elementsQuantity); j < jend; ++j)
      {
         if ((*j) < elementsQuantity)
         {
            (*(glGroups + (*j) * glGroupsStep)) = i;
         }
         if (*j < elementsWithoutColliderQuantity)
         {
            if (offset)
            {
               *(j - offset) = *j;
            }
            (*j) -= elementsWithoutColliderQuantity;
         }
         else
         {
            --(groups->elementsQuantity);
            ++offset;
         }
      }
      if (groups->elementsQuantity)
      {
         groups->elementIDs = realloc(groups->elementIDs, groups->elementsQuantity * sizeof(uint32_t));
      }
      else
      {
         free(groups->elementIDs);
      }
      offset = 0u;
      ++i, ++groups;
   }
}

static void offsetCCEgroupsFromElementsToColliders (uint16_t groupsQuantity, struct ElementGroup *groups, uint32_t elementsWithoutColliderQuantity)
{
   struct ElementGroup *end = (groups + (groupsQuantity));
   uint32_t offset = 0u;
   while (groups < end)
   {
      if (groups->elementsQuantity) for (uint32_t *iterator = groups->elementIDs, *end = (groups->elementIDs + groups->elementsQuantity); iterator < end; ++iterator)
      {
         if (*iterator < elementsWithoutColliderQuantity)
         {
            if (offset)
            {
               *(iterator - offset) = *iterator;
            }
            (*iterator) -= elementsWithoutColliderQuantity;
         }
         else
         {
            --(groups->elementsQuantity);
            ++offset;
         }
      }
      if (groups->elementsQuantity)
      {
         groups->elementIDs = realloc(groups->elementIDs, groups->elementsQuantity * sizeof(uint32_t));
      }
      else
      {
         free(groups->elementIDs);
      }
      offset = 0u;
      ++groups;
   }
}

// Just in case of file corruption to avoid underflow (define because string is too long)
#define ELEMENTSCOLLIDERSQUANTITY(elementsQuantity, elementsWithoutColliderQuantity) (((elementsQuantity) > (elementsWithoutColliderQuantity)) ? \
                                                                                       (elementsQuantity) - (elementsWithoutColliderQuantity)  :  \
                                                                                        0u)

static struct Map2DCollider* elementsToColliders (uint32_t  elementsQuantity, uint32_t elementsWithoutColliderQuantity, struct Map2DElement *elements,
                                                  uint16_t **texturesMapReliesOn, uint16_t *texturesMapReliesOnQuantity,
                                                  uint16_t  moveGroupsQuantity, struct ElementGroup *moveGroups,
                                                  uint16_t  extensionGroupsQuantity, struct ElementGroup *extensionGroups,
                                                  GLuint *VAO, GLuint *VBO)
{
   *texturesMapReliesOn = cce__loadTexturesMap2D(elements, elementsQuantity, texturesMapReliesOnQuantity);
   elements = (struct Map2DElement*) realloc(elements, (sizeof(struct Map2DElement) + 3u * sizeof(uint8_t)) * elementsQuantity);
   uint8_t *glGroups = (uint8_t*) ((void*) (elements + elementsQuantity));
   memset(glGroups, 0, elementsQuantity * (3u * sizeof(uint8_t)));
   uint32_t currentElement = 0u;
   if (moveGroups)
   {
      if (moveGroups->elementsQuantity) for (uint32_t *exclude = moveGroups->elementIDs, *excludeEnd = moveGroups->elementIDs + moveGroups->elementsQuantity;
                                             currentElement < elementsQuantity; ++currentElement)
      {
         if (currentElement == *exclude)
         {
            ++exclude;
            if (exclude >= excludeEnd)
               break;
            continue;
         }
         (*(glGroups + currentElement * 2u + 1u)) = CCE_GLOBAL_OFFSET_MASK;
      }
      convertCCEgroupsToGLgroups(moveGroupsQuantity - 1u, moveGroups + 1u, glGroups, 2u, elementsWithoutColliderQuantity, elementsQuantity);
   }
   while (currentElement < elementsQuantity)
   {
      (*(glGroups + currentElement * 2u + 1u)) = CCE_GLOBAL_OFFSET_MASK;
      ++currentElement;
   }
   if (extensionGroups)
      convertCCEgroupsToGLgroups(extensionGroupsQuantity, extensionGroups, glGroups + (elementsQuantity * (2u * sizeof(uint8_t))), 1u, elementsWithoutColliderQuantity, elementsQuantity);
      
   *VAO = makeVAOmap2D(elements, elementsQuantity, VBO);
   
   if (moveGroups && moveGroupsQuantity > 256u)
      offsetCCEgroupsFromElementsToColliders(moveGroupsQuantity - 256u, moveGroups + 256u, elementsWithoutColliderQuantity);
   if (extensionGroups && extensionGroupsQuantity > 255u)
      offsetCCEgroupsFromElementsToColliders(extensionGroupsQuantity - 255u, extensionGroups + 255u, elementsWithoutColliderQuantity);
   
   // Dangerous memory optimization!
   struct Map2DCollider *colliders = (struct Map2DCollider*) ((void*) elements);
   for (struct Map2DCollider *iterator = colliders, *end = (colliders + ELEMENTSCOLLIDERSQUANTITY(elementsQuantity, elementsWithoutColliderQuantity) - 1); iterator <= end; ++iterator, ++elements)
   {
      iterator->x      = elements->x;
      iterator->y      = elements->y;
      iterator->height = elements->height;
      iterator->width  = elements->width;
   }
   return colliders;
}

struct Map2D* cceLoadMap2D (uint16_t number)
{
   shortToString(mapPath, number, ".c2m");
   FILE *map_f = fopen(mapPath, "rb");
   if (!map_f)
   {
      cce__criticalErrorPrint("ENGINE::MAP2D::FAILED_TO_LOAD:\n%s - no such file or directory", mapPath);
   }
   *(mapPath + mapPathLength) = '\0';
   
   struct Map2D *map = (struct Map2D*) malloc(sizeof(struct Map2D));
   map->ID = number;
   // GL elements
   {
      uint32_t elementsWithoutColliderQuantity, elementsCollidersQuantity;
      fread(&(map->elementsQuantity), 4u/*uint32_t*/, 1u, map_f);
      fread(&elementsWithoutColliderQuantity, 4u/*uint32_t*/, 1u, map_f);
      elementsCollidersQuantity = ELEMENTSCOLLIDERSQUANTITY(map->elementsQuantity, elementsWithoutColliderQuantity);
      struct Map2DCollider *colliders;
      colliders = NULL;
      if (map->elementsQuantity)
      {
         struct Map2DElement *elements;
         elements = (struct Map2DElement*) malloc(map->elementsQuantity * sizeof(struct Map2DElement));
         fread(elements, sizeof(struct Map2DElement), map->elementsQuantity, map_f);
         // Game logic elements
         fread(&(map->moveGroupsQuantity), 2u/*uint16_t*/, 1u, map_f);
         map->moveGroups = cce__loadGroups(map->moveGroupsQuantity, map_f);
         fread(&(map->extensionGroupsQuantity), 2u/*uint16_t*/, 1u, map_f);
         map->extensionGroups = cce__loadGroups(map->extensionGroupsQuantity, map_f);
         colliders = elementsToColliders(map->elementsQuantity, elementsWithoutColliderQuantity, elements, &(map->texturesMapReliesOn), &(map->texturesMapReliesOnQuantity),
                                         map->moveGroupsQuantity, map->moveGroups, map->extensionGroupsQuantity, map->extensionGroups,  &(map->VAO), &(map->VBO));
         
         map->UBO_ID = cce__getFreeUBO();
      }
      else
      {
         fread(&(map->moveGroupsQuantity), 2u/*uint16_t*/, 1u, map_f);
         map->moveGroups = cce__loadGroups(map->moveGroupsQuantity, map_f);
      }
      uint32_t collidersQuantity;
      fread(&collidersQuantity, 4u, 1u, map_f);
      map->collidersQuantity = elementsCollidersQuantity + collidersQuantity;
      map->colliders = (struct Map2DCollider*) realloc(colliders, (map->collidersQuantity) * sizeof(struct Map2DCollider));
      if (collidersQuantity)
      {
         fread((map->colliders + elementsCollidersQuantity), sizeof(struct Map2DCollider), collidersQuantity, map_f);
      }
   }
   fread(&(map->collisionGroupsQuantity), 2u/*uint16_t*/, 1u, map_f);
   map->collisionGroups = cce__loadGroups(map->collisionGroupsQuantity, map_f);
   fread(&(map->collisionQuantity), 2u/*uint16_t*/, 1u, map_f);
   if ((map->collisionQuantity))
   {
      (map->collision) = (struct CollisionGroup*) malloc((map->collisionQuantity) * sizeof(struct CollisionGroup));
      fread((map->collision), sizeof(struct CollisionGroup), (map->collisionQuantity), map_f);
      (map->collisionCache) = (double*) calloc((map->collisionQuantity), sizeof(double));
   }
   else
   {
      (map->collision) = NULL;
      (map->collisionCache) = NULL;
   }
   fread(&(map->timersQuantity), 2u/*uint16_t*/, 1u, map_f);
   if ((map->timersQuantity))
   {
      (map->timers) = (struct Timer*) malloc(map->timersQuantity * sizeof(struct Timer));
      struct Timer *end = (map->timers + map->timersQuantity - 1u);
      for (struct Timer *iterator = (map->timers); iterator <= end; ++iterator)
      {
         fread((&(iterator->delay)), 4u/*float*/, 1u, map_f);
         iterator->initTime = 0.0;
      }
   }
   else
   {
      (map->timers) = NULL;
   }
   fread(&(map->logicQuantity),  4u/*uint32_t*/, 1u, map_f);
   if ((map->logicQuantity))
   {
      map->logic = cce__loadLogic(map->logicQuantity, map_f);
      map->temporaryBools = cce__getFreeTemporaryBools();
   }
   else
   {
      map->logic = NULL;
   }
   
   uint8_t staticActionsQuantity;
   fread(&(staticActionsQuantity), 1u, 1u, map_f);
   if (staticActionsQuantity)
   {
      cce__beginBaseActions(map);
      uint32_t *staticActionsIDs = (uint32_t *) malloc(staticActionsQuantity * sizeof(uint32_t));
      fread(staticActionsIDs, 4u, staticActionsQuantity, map_f);
      uint32_t *staticActionsArgOffsets = (uint32_t *) malloc((staticActionsQuantity + 1u) * sizeof(uint32_t));
      fread(staticActionsArgOffsets + 1, 4u, staticActionsQuantity, map_f);
      *staticActionsArgOffsets = 0u;
      cce_void *staticActionsArgs = (cce_void *) malloc((*(staticActionsArgOffsets + staticActionsQuantity)) * sizeof(uint8_t));
      fread( (staticActionsArgs), 1u, *(staticActionsArgOffsets + staticActionsQuantity), map_f);
      if (map->logicQuantity)
      {
         cce__setCurrentTemporaryBools(map->temporaryBools);
      }
      cce__callActions(*cce_actions, staticActionsQuantity, staticActionsIDs, staticActionsArgOffsets, staticActionsArgs);
      
      free(staticActionsIDs);
      free(staticActionsArgOffsets);
      free(staticActionsArgs);
      cce__endBaseActions();
   }
   fread(&(map->exitMapsQuantity), 1u, 1u, map_f);
   if (map->exitMapsQuantity)
   {
      map->exitMaps = (struct ExitMap2D*) malloc(map->exitMapsQuantity * sizeof(struct ExitMap2D));
      fread(map->exitMaps, sizeof(struct ExitMap2D), map->exitMapsQuantity, map_f);
   }
   
   // I'm lazy
   for (uint8_t smth = 0u, i = 0u; i < 10; ++i)
   {
      fread(&smth, 1u, 1u, map_f);
      if (smth)
      {
         cce__criticalErrorPrint("ENGINE::MAP2D_LOADER::PARSING_ERROR:\nfile of map %u contains fields that isn't implemented in current engine version", number);
      }
   }
   if (cce_fileParseFunc) cce_fileParseFunc(map_f, number);
   if (fclose(map_f) == -1)
   {
      cce__errorPrint("ENGINE::MAP2D_LOADER::FILE_UNEXPECTED_CLOSE:\nmap %u file was unexpectedly closed by external file handler", number);
   }
   return map;
}

/* Do not frees mapdev */
struct Map2D* cceMap2DdevToMap2D (struct Map2Ddev *mapdev)
{
   struct Map2D *map = (struct Map2D*) malloc(sizeof(struct Map2D));
   map->ID = mapdev->ID;
   map->moveGroupsQuantity = mapdev->moveGroupsQuantity;
   if (mapdev->moveGroupsQuantity)
   {
      map->moveGroups = (struct ElementGroup*) malloc(map->moveGroupsQuantity * sizeof(struct ElementGroup));
      for (struct ElementGroup *i = map->moveGroups, *j = mapdev->moveGroups, *jend = mapdev->moveGroups + mapdev->moveGroupsQuantity - 1u;
           j < jend; ++i, ++j)
      {
         i->elementsQuantity = j->elementsQuantity;
         i->elementIDs = (uint32_t*) malloc(j->elementsQuantity * sizeof(uint32_t));
         memcpy(i->elementIDs, j->elementIDs, (j->elementsQuantity * sizeof(uint32_t)));
      }
   }
   else
   {
      map->moveGroups = NULL;
   }
   if (mapdev->extensionGroupsQuantity)
   {
      map->extensionGroups = (struct ElementGroup*) malloc(mapdev->extensionGroupsQuantity * sizeof(struct ElementGroup));
      for (struct ElementGroup *i = map->extensionGroups, *j = mapdev->extensionGroups, *jend = mapdev->extensionGroups + mapdev->extensionGroupsQuantity - 1u;
           j < jend; ++i, ++j)
      {
         i->elementsQuantity = j->elementsQuantity;
         i->elementIDs = (uint32_t*) malloc(j->elementsQuantity * sizeof(uint32_t));
         memcpy(i->elementIDs, j->elementIDs, (j->elementsQuantity * sizeof(uint32_t)));
      }
   }
   else
   {
      map->extensionGroups = NULL;
   }
   {
      map->elementsQuantity = mapdev->elementsQuantity;
      uint32_t elementsCollidersQuantity = ELEMENTSCOLLIDERSQUANTITY(mapdev->elementsQuantity, mapdev->elementsWithoutColliderQuantity);
      struct Map2DElement *elements = (struct Map2DElement*) malloc(mapdev->elementsQuantity * sizeof(struct Map2DElement));
      memcpy(elements, mapdev->elements, (mapdev->elementsQuantity * sizeof(struct Map2DElement)));
      struct Map2DCollider *colliders;
      colliders = NULL;
      if (mapdev->elementsQuantity)
      {
         colliders = elementsToColliders(mapdev->elementsQuantity, mapdev->elementsWithoutColliderQuantity, elements, &(map->texturesMapReliesOn), &(map->texturesMapReliesOnQuantity),
                                         mapdev->moveGroupsQuantity, map->moveGroups, mapdev->extensionGroupsQuantity, map->extensionGroups, &(map->VAO), &(map->VBO));
         
         map->UBO_ID = cce__getFreeUBO();
      }
      map->collidersQuantity = elementsCollidersQuantity + mapdev->collidersQuantity;
      map->colliders = (struct Map2DCollider*) realloc(colliders, (map->collidersQuantity) * sizeof(struct Map2DCollider));
      if (mapdev->collidersQuantity)
      {
         memcpy(map->colliders + elementsCollidersQuantity, mapdev->colliders, mapdev->collidersQuantity * sizeof(struct Map2DCollider));
      }
   }
   map->collisionGroupsQuantity = mapdev->collisionGroupsQuantity;
   if (mapdev->collisionGroupsQuantity)
   {
      map->collisionGroups = (struct ElementGroup*) malloc(map->collisionGroupsQuantity * sizeof(struct ElementGroup));
      for (struct ElementGroup *i = map->collisionGroups, *j = mapdev->collisionGroups, *jend = mapdev->collisionGroups + mapdev->collisionGroupsQuantity - 1u;
           j < jend; ++i, ++j)
      {
         i->elementsQuantity = j->elementsQuantity;
         i->elementIDs = (uint32_t*) malloc(j->elementsQuantity * sizeof(uint32_t));
         memcpy(i->elementIDs, j->elementIDs, (j->elementsQuantity * sizeof(uint32_t)));
      }
   }
   else
   {
      map->collisionGroups = NULL;
   }
   map->collisionQuantity = mapdev->collisionQuantity;
   if (mapdev->collisionQuantity)
   {
      map->collision = (struct CollisionGroup*) malloc(map->collisionQuantity * sizeof(struct CollisionGroup));
      memcpy(map->collision, mapdev->collision, (mapdev->collisionQuantity) * sizeof(struct CollisionGroup));
      map->collisionCache = (double*) calloc(map->collisionQuantity, sizeof(double));
   }
   else
   {
      map->collision = NULL;
      map->collisionCache = NULL;
   }
   map->timersQuantity = mapdev->timersQuantity;
   if (mapdev->timersQuantity)
   {
      map->timers = (struct Timer*) malloc(mapdev->timersQuantity * sizeof(struct Timer));
      float *srcDelays = mapdev->delaysOfTimers;
      for(struct Timer *iterator = map->timers, *end = (map->timers + mapdev->timersQuantity - 1u); iterator <= end; ++iterator, ++srcDelays)
      {
         iterator->delay = *srcDelays;
         iterator->initTime = 0.0;
      }
   }
   else
   {
      map->timers = NULL;
   }
   map->logicQuantity = mapdev->logicQuantity;
   if (mapdev->logicQuantity)
   {
      map->logic = (struct ElementLogic*) malloc(mapdev->logicQuantity * sizeof(struct ElementLogic));
      
      uint_fast32_t operationsQuantityInBytes;
      uint8_t isLogicQuantityHigherThanThree;
      for(struct ElementLogic *src = mapdev->logic, *dest = map->logic, *end = (map->logic + mapdev->logicQuantity - 1u); dest <= end; ++src, ++end)
      {
         dest->logicElementsQuantity = src->logicElementsQuantity;
         dest->logicElements = (uint16_t*) malloc(src->logicElementsQuantity * sizeof(uint16_t));
         memcpy(dest->logicElements, src->logicElements, src->logicElementsQuantity * sizeof(uint16_t));
         
         isLogicQuantityHigherThanThree = src->logicElementsQuantity > 3u;
         operationsQuantityInBytes = (0x01 << ((src->logicElementsQuantity) - 3u) * isLogicQuantityHigherThanThree) + (!isLogicQuantityHigherThanThree);
         (dest->operations) = (uint_fast16_t*) calloc((operationsQuantityInBytes > sizeof(uint_fast16_t)) ? operationsQuantityInBytes : sizeof(uint_fast16_t), 1u);
         memcpy(dest->operations, src->operations, operationsQuantityInBytes);
         
         dest->elementType = src->elementType;
         dest->actionsQuantity = src->actionsQuantity;
         dest->actionIDs = (uint32_t*) malloc(src->actionsQuantity * sizeof(uint32_t));
         memcpy(dest->actionIDs, src->actionIDs, src->actionsQuantity * sizeof(uint32_t));
         dest->actionsArgOffsets = (uint32_t*) malloc((src->actionsQuantity + 1u) * sizeof(uint32_t));
         memcpy(dest->actionsArgOffsets, src->actionsArgOffsets, src->actionsQuantity + 1u);
         dest->actionsArg = (cce_void *) malloc((*(src->actionsArgOffsets + src->actionsQuantity) - 1u)/* sizeof(cce_void)*/);
         memcpy(dest->actionsArg, src->actionsArg, (*(src->actionsArgOffsets + src->actionsQuantity) - 1u)/* sizeof(cce_void)*/);
      }
      map->temporaryBools = cce__getFreeTemporaryBools();
   }
   else
   {
      map->logic = NULL;
   }
   if (mapdev->actionsQuantity)
   {
      if (mapdev->logicQuantity)
      {
         cce__setCurrentTemporaryBools(map->temporaryBools);
      }
      cce__beginBaseActions(map);
      cce__callActions(*cce_actions, mapdev->actionsQuantity, mapdev->actionIDs, mapdev->actionsArgOffsets, mapdev->actionsArg);
      cce__endBaseActions();
   }
   map->exitMapsQuantity = mapdev->exitMapsQuantity;
   if (mapdev->exitMapsQuantity)
   {
      map->exitMaps = (struct ExitMap2D*) malloc(mapdev->exitMapsQuantity * sizeof(struct ExitMap2D));
      memcpy(map->exitMaps, mapdev->exitMaps, (mapdev->exitMapsQuantity * sizeof(struct ExitMap2D)));
   }
   return map;
}

struct Map2Ddev* cceLoadMap2Ddev (uint16_t number)
{
   shortToString(mapPath, number, ".c2m");
   FILE *map_f = fopen(mapPath, "rb");
   if (!map_f)
   {
      cce__criticalErrorPrint("ENGINE::MAP2Ddev::FAILED_TO_LOAD:\n%s - no such file or directory", mapPath);
   }
   *(mapPath + mapPathLength) = '\0';
   struct Map2Ddev *map = (struct Map2Ddev*) calloc(1u, sizeof(struct Map2Ddev));
   map->ID = number;
   fread(&(map->elementsQuantity), 4u/*uint32_t*/, 1u, map_f);
   fread(&(map->elementsWithoutColliderQuantity), 4u/*uint32_t*/, 1u, map_f);
   if ((map->elementsQuantity))
   {
      (map->elements) = (struct Map2DElement*) malloc((map->elementsQuantity) * sizeof(struct Map2DElement));
      fread((map->elements), sizeof(struct Map2DElement), (map->elementsQuantity), map_f);
   }
   fread(&(map->moveGroupsQuantity), 2u/*uint16_t*/, 1u, map_f);
   map->moveGroups = cce__loadGroups(map->moveGroupsQuantity, map_f);
   fread(&(map->extensionGroupsQuantity), 2u/*uint16_t*/, 1u, map_f);
   map->extensionGroups = cce__loadGroups(map->extensionGroupsQuantity, map_f);
   fread(&(map->collidersQuantity), 4u/*uint16_t*/, 1u, map_f);
   if ((map->collidersQuantity))
   {
      (map->colliders) = (struct Map2DCollider*) malloc((map->collidersQuantity) * sizeof(struct Map2DCollider));
      fread((map->colliders), sizeof(struct Map2DCollider), (map->collidersQuantity), map_f);
   }
   fread(&(map->collisionGroupsQuantity), 2u/*uint16_t*/, 1u, map_f);
   map->collisionGroups = cce__loadGroups(map->collisionGroupsQuantity, map_f);
   fread(&(map->collisionQuantity), 2u/*uint16_t*/, 1u, map_f);
   if ((map->collisionQuantity))
   {
      (map->collision) = (struct CollisionGroup*) malloc((map->collisionQuantity) * sizeof(struct CollisionGroup));
      fread((map->collision), sizeof(struct CollisionGroup), (map->collisionQuantity), map_f);
   }
   fread(&(map->timersQuantity), 2u/*uint16_t*/, 1u, map_f);
   if ((map->timersQuantity))
   {
      (map->delaysOfTimers) = (float*) malloc((map->timersQuantity) * sizeof(float));
      fread((map->delaysOfTimers), 4u/*float*/, (map->timersQuantity), map_f);
   }
   fread(&(map->logicQuantity), 4u/*uint32_t*/, 1u, map_f);
   if (map->logicQuantity)
   {
      map->logic = cce__loadLogic(map->logicQuantity, map_f);
   }
   fread(&(map->actionsQuantity), 1u/*uint8_t*/, 1u, map_f);
   if (map->actionsQuantity)
   {
      map->actionIDs = (uint32_t *) malloc((map->actionsQuantity) * sizeof(uint32_t));
      fread( (map->actionIDs),             4u/*uint32_t*/,   (map->actionsQuantity),                                           map_f);
      map->actionsArgOffsets = (uint32_t*) malloc((map->actionsQuantity + 1u) * sizeof(uint32_t));
      *(map->actionsArgOffsets) = 0u;
      fread( (map->actionsArgOffsets + 1), 4u/*uint32_t*/, (map->actionsQuantity),                                           map_f);
      map->actionsArg = (cce_void *) malloc((*(map->actionsArgOffsets + map->actionsQuantity) - 1u)/* sizeof(cce_void)*/);
      fread( (map->actionsArg),            1u/*cce_void*/,         *(map->actionsArgOffsets + map->actionsQuantity),          map_f);
   }
   fread(&(map->exitMapsQuantity), 1u, 1u, map_f);
   if (map->exitMapsQuantity)
   {
      map->exitMaps = (struct ExitMap2D*) malloc(map->exitMapsQuantity * sizeof(struct ExitMap2D));
      fread(map->exitMaps, sizeof(struct ExitMap2D), map->exitMapsQuantity, map_f);
   }
   
   // I'm lazy, again
   for (uint8_t smth = 0u, i = 0u; i < 10; ++i)
   {
      fread(&smth, 1u, 1u, map_f);
      if (smth)
      {
         cce__criticalErrorPrint("ENGINE::MAP2Ddev_LOADER::PARSING_ERROR:\nfile of map %u contains fields that isn't implemented in current engine version", number);
      }
   }
   if (cce_fileParseFunc) cce_fileParseFunc(map_f, number);
   if (fclose(map_f) == -1)
   {
      cce__criticalErrorPrint("ENGINE::MAP2Ddev_LOADER::FILE_UNEXPECTED_CLOSE:\nmap %u file was unexpectedly close by external file handler", number);
   }
   return map;
}

int cceWriteMap2Ddev (struct Map2Ddev *map, void (*writeFunc)(FILE*))
{
   shortToString(mapPath, map->ID, ".c2m");
   FILE *map_f = fopen(mapPath, "wb");
   if (!map_f)
   {
      cce__infoPrint("ENGINE::MAP2Ddev::FAILED_TO_OPEN_FILE:\n%s - cannot open file. Are you haven't enough free space left? Are you have a directory with same name? Does directory really exist?\n", mapPath);
      return -1;
   }
   *(mapPath + mapPathLength) = '\0';
   fwrite(&(map->elementsQuantity), 4u/*uint32_t*/, 1u, map_f);
   fwrite(&(map->elementsWithoutColliderQuantity), 4u/*uint32_t*/, 1u, map_f);
   if ((map->elementsQuantity))
   {
      fwrite((map->elements), sizeof(struct Map2DElement), (map->elementsQuantity), map_f);
   }
   fwrite(&(map->moveGroupsQuantity), 2u/*uint16_t*/, 1u, map_f);
   if ((map->moveGroupsQuantity))
   {
      cce__writeGroups(map->moveGroupsQuantity, map->moveGroups, map_f);
   }
   fwrite(&(map->extensionGroupsQuantity), 2u/*uint16_t*/, 1u, map_f);
   if ((map->extensionGroupsQuantity))
   {
      cce__writeGroups(map->extensionGroupsQuantity, map->extensionGroups, map_f);
   }
   fwrite(&(map->collidersQuantity), 4u/*uint32_t*/, 1u, map_f);
   if ((map->collidersQuantity))
   {
      fwrite((map->colliders), sizeof(struct Map2DCollider), (map->collidersQuantity), map_f);
   }
   fwrite(&(map->collisionGroupsQuantity), 2u/*uint16_t*/, 1u, map_f);
   if ((map->collisionGroupsQuantity))
   {
      cce__writeGroups(map->collisionGroupsQuantity, map->collisionGroups, map_f);
   }
   fwrite(&(map->collisionQuantity), 2u/*uint16_t*/, 1u, map_f);
   if ((map->collisionQuantity))
   {
      fwrite((map->collision), sizeof(struct CollisionGroup), (map->collisionQuantity), map_f);
   }
   fwrite(&(map->timersQuantity), 2u/*uint16_t*/, 1u, map_f);
   if ((map->timersQuantity))
   {
      fwrite((map->delaysOfTimers), 4u/*float*/, (map->timersQuantity), map_f);
   }
   fwrite(&(map->logicQuantity), 4u/*uint32_t*/, 1u, map_f);
   if (map->logicQuantity)
   {
      cce__writeLogic(map->logicQuantity, map->logic, map_f);
   }
   fwrite(&(map->actionsQuantity), 1u/*uint8_t*/, 1u, map_f);
   if (map->actionsQuantity)
   {
      fwrite( (map->actionIDs),             4u/*uint32_t*/,  (map->actionsQuantity),                          map_f);
      fwrite( (map->actionsArgOffsets + 1), 4u/*uint32_t*/,  (map->actionsQuantity),                          map_f);
      fwrite( (map->actionsArg),            1u/*cce_void*/, *(map->actionsArgOffsets + map->actionsQuantity), map_f);
   }
   fwrite(&(map->exitMapsQuantity), 1u/*uint8_t*/, 1u, map_f);
   if (map->exitMapsQuantity)
   {
      fwrite(map->exitMaps, sizeof(struct ExitMap2D), map->exitMapsQuantity, map_f);
   }
   
   // I'm lazy, again
   uint8_t null = 0u;
   for (uint8_t i = 0u; i < 10; ++i)
   {
      fwrite(&null, 1u/*uint8_t*/, 1u, map_f);
   }
   if (writeFunc) writeFunc(map_f);
   if (fclose(map_f) == -1)
   {
      cce__errorPrint("ENGINE::MAP2Ddev_MAPWRITER::FILE_UNEXPECTED_CLOSE:\nmap %u file was unexpectedly closed by external file handler", map->ID);
   }
   return 0;
}
