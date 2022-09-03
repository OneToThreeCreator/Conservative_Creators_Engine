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

#include "../../include/coffeechain/engine_common.h"
#include "../../include/coffeechain/os_interaction.h"
#include "../../include/coffeechain/endianess.h"
#include "../../include/coffeechain/utils.h"
#include "../../include/coffeechain/map2D/map2D.h"
#include "../../include/coffeechain/map2D/base_actions.h"

#include "../engine_common_internal.h"
#include "map2D_internal.h"

static char *mapPath = NULL;
size_t mapPathLength;
const cce_flag *map2Dflags;

static void (*cce_fileParseFunc)(FILE*, uint16_t);
static void (*cce_callbackOnFreeing)(uint16_t);

void cce__initMap2DLoaders (const cce_flag *flagsPointer)
{
   map2Dflags = flagsPointer;
}

CCE_PUBLIC_OPTIONS void cceSetMap2Dpath (const char *path)
{
   if (!path || *path == '\0')
      return;
   free(mapPath);
   mapPath = cceCreateNewPathFromOldPath(path, "map_", 9u);
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
   if (map->collidersQuantity)
      free(map->colliders);
   if (map->moveGroupsQuantity)
   {
      for (struct ElementGroup *iterator = map->moveGroups, *end = map->moveGroups + map->moveGroupsQuantity; iterator < end; ++iterator)
      {
         free(iterator->elements);
      }
      free(map->moveGroups);
   }
   if (map->extensionGroupsQuantity)
   {
      for (struct ElementGroup *iterator = map->extensionGroups, *end = map->extensionGroups + map->extensionGroupsQuantity; iterator < end; ++iterator)
      {
         free(iterator->elements);
      }
      free(map->extensionGroups);
   }
   if (map->collisionGroupsQuantity)
   {
      for (struct ElementGroup *iterator = map->collisionGroups, *end = map->collisionGroups + map->collisionGroupsQuantity; iterator < end; ++iterator)
      {
         free(iterator->elements);
      }
      free(map->collisionGroups);
   }
   if (map->collisionQuantity)
      free(map->collision);
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
      cce__releaseTextures(map->texturesMapReliesOn, map->texturesMapReliesOnQuantity);
   if ((map->exitMapsQuantity))
      free(map->exitMaps);
   if (cce_callbackOnFreeing)
   {
      if (map->logicQuantity)
      {
         cce__setCurrentTemporaryBools(map->temporaryBools);
      }
      cce_callbackOnFreeing(map->ID);
   }
   if (*map2Dflags & (CCE_PROCESS_LOGIC_FOR_VISIBLE_MAPS | CCE_PROCESS_LOGIC_FOR_ALL_MAPS | CCE_FORCE_INITIALIZE_MAP_ONLOAD))
   {
      cce__releaseTemporaryBools(map->temporaryBools);
   }
   llrmlist(&map->delayedActions);
   free(map);
}

CCE_PUBLIC_OPTIONS void cceFreeMap2Ddev (struct Map2Ddev *map)
{
   if (!map)
      return;
   if ((map->elementsQuantity))
      free(map->elements);
   if ((map->transformGroupsQuantity))
   {
      for (struct DynamicElementGroup *iterator = map->transformGroups, *end = map->transformGroups + map->transformGroupsQuantity; iterator < end; ++iterator)
      {
         free(iterator->elements);
      }
      free(map->transformGroups);
   }
   if ((map->collisionGroupsQuantity))
   {
      for (struct DynamicElementGroup *iterator = map->collisionGroups, *end = map->collisionGroups + map->collisionGroupsQuantity; iterator < end; ++iterator)
      {
         free(iterator->elements);
      }
      free(map->collisionGroups);
   }
   if ((map->collisionQuantity))
      free(map->collision);
   if ((map->timersQuantity))
      free(map->timers);
   if ((map->logicQuantity))
   {
      for (struct ElementLogic *iterator = map->logic, *end = map->logic + map->logicQuantity; iterator < end; ++iterator)
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

//#define ADD_TO_2BIT_ARRAY(array, i, number) ((array)[(i) >> (SHIFT_OF_FAST_SIZE - 1)] += ((number) << ((i) & ((1 << (SHIFT_OF_FAST_SIZE - 1)) - 1))))
//#define GET_VALUE_FROM_2BIT_ARRAY(array, i)(((array)[(i) >> (SHIFT_OF_FAST_SIZE - 1)] >> ((i) & ((1 << (SHIFT_OF_FAST_SIZE - 1)) - 1))) & 3)
static void convertCCEgroupsToRenderablegroups (uint16_t groupsQuantity, struct ElementGroup *groups, uint8_t *glGroups, uint8_t glGroupsStep, uint32_t elementsWithoutColliderQuantity, uint32_t elementsQuantity)
{
   uint8_t i = 1u;
   uint32_t offset = 0u;
   struct ElementGroup *end = (groups + (CCE_MIN(256u, groupsQuantity)));
   while (groups < end)
   {
      if (groups->elementsQuantity) for (uint32_t *j = groups->elements, *jend = (groups->elements + groups->elementsQuantity); j < jend; ++j)
      {
         if ((*j) < elementsQuantity)
         {
            for (uint8_t *iterator = glGroups + (*j) * glGroupsStep * 4, *iend = glGroups + (*j) * glGroupsStep * 4 + 4; iterator < iend; ++iterator)
            {
               if (*iterator == 0)
               {
                  *iterator = i;
                  break;
               }
            }
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
         groups->elements = realloc(groups->elements, groups->elementsQuantity * sizeof(uint32_t));
      }
      else
      {
         free(groups->elements);
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
      if (groups->elementsQuantity) for (uint32_t *iterator = groups->elements, *iend = (groups->elements + groups->elementsQuantity); iterator < iend; ++iterator)
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
         groups->elements = realloc(groups->elements, groups->elementsQuantity * sizeof(uint32_t));
      }
      else
      {
         free(groups->elements);
      }
      offset = 0u;
      ++groups;
   }
}

// Just in case of file corruption to avoid underflow (define because string is too long)
#define ELEMENTSCOLLIDERSQUANTITY(elementsQuantity, elementsWithoutColliderQuantity) (((elementsQuantity) > (elementsWithoutColliderQuantity)) ? \
                                                                                       (elementsQuantity) - (elementsWithoutColliderQuantity)  : \
                                                                                        0u)

static struct Map2DCollider* elementsToColliders (uint32_t  elementsQuantity, uint32_t elementsWithoutColliderQuantity, struct Map2DElement *elements,
                                                  uint16_t **texturesMapReliesOn, uint16_t *texturesMapReliesOnQuantity,
                                                  uint16_t  transformGroupsQuantity, struct ElementGroup *transformGroups)
{
   *texturesMapReliesOn = cce__loadTexturesMap2D(elements, elementsQuantity, texturesMapReliesOnQuantity);
   elements = (struct Map2DElement*) realloc(elements, (sizeof(struct Map2DElement) + (2 * 4 + 1) * sizeof(uint8_t)) * elementsQuantity);
   uint8_t *glGroups = (uint8_t*) ((void*) (elements + elementsQuantity));
   memset(glGroups, 0, elementsQuantity * ((2 * 4 + 1) * sizeof(uint8_t)));
   uint32_t currentElement = 0;
   if (transformGroups)
   {
      if (transformGroups->elementsQuantity) for (uint32_t *exclude = transformGroups->elements, *excludeEnd = transformGroups->elements + transformGroups->elementsQuantity;
                                             currentElement < elementsQuantity; ++currentElement)
      {
         if (currentElement == *exclude)
         {
            ++exclude;
            if (exclude >= excludeEnd)
               break;
            continue;
         }
         (*(glGroups + currentElement)) = 1;
      }
      convertCCEgroupsToRenderablegroups(transformGroupsQuantity - 1,  transformGroups + 1,  glGroups + (elementsQuantity * (1 * sizeof(uint8_t))), 1, elementsWithoutColliderQuantity, elementsQuantity);
   }
   while (currentElement < elementsQuantity)
   {
      (*(glGroups + currentElement)) = 1;
      ++currentElement;
   }
   
   if (transformGroups && transformGroupsQuantity > 256u)
      offsetCCEgroupsFromElementsToColliders(transformGroupsQuantity - 256u, transformGroups + 256u, elementsWithoutColliderQuantity);
   
   // Dangerous memory optimization!
   struct Map2DCollider *colliders = (struct Map2DCollider*) ((void*) elements);
   for (struct Map2DCollider *iterator = colliders, *end = colliders + ELEMENTSCOLLIDERSQUANTITY(elementsQuantity, elementsWithoutColliderQuantity); iterator < end; ++iterator, ++elements)
   {
      *iterator = elements->collider;
   }
   return colliders;
}

static struct Map2DElement* cce__loadMap2DElements (uint32_t elementsQuantity, FILE *file)
{
   struct Map2DElement *elements = malloc(elementsQuantity * sizeof(struct Map2DElement));
   for (struct Map2DElement *iterator = elements, *end = elements + elementsQuantity; iterator < end; ++iterator)
   {
      fread(&(iterator->position), sizeof(int32_t), 2, file); // x and y at the same time
      cceLittleEndianToHostEndianArrayInt32(&(iterator->position), 2);
      fread(&(iterator->size), sizeof(uint16_t), 2, file);
      cceLittleEndianToHostEndianArrayInt16(&(elements->size), 2);
      fread(&(iterator->textureInfo), sizeof(struct Texture), 1, file);
      cceLittleEndianToHostEndianArrayInt16(&(iterator->textureInfo), 4);
      iterator->textureInfo.ID = cceLittleEndianToHostEndianInt32(iterator->textureInfo.ID);
      fread(iterator->textureOffsetGroups, sizeof(uint8_t), 9, file);
   }
   return elements;
}

static void cce__writeMap2DElements (struct Map2DElement* elements, uint32_t elementsQuantity, FILE *file)
{
   union
   {
      struct
      {
         int32_t x;
         int32_t y;
      } i32;
      struct
      {
         uint16_t x;
         uint16_t y;
      } u16;
      struct Texture tInfo;
   }
   temporary;
   for (struct Map2DElement *iterator = elements, *end = elements + elementsQuantity; iterator < end; ++iterator)
   {
      cceHostEndianToLittleEndianNewArrayInt32(&(temporary.i32), &(iterator->position), 2);
      fwrite(&(temporary.i32.x), sizeof(int32_t), 2, file);
      cceHostEndianToLittleEndianNewArrayInt16(&(temporary.u16), &(iterator->size), 2);
      fwrite(&(temporary.u16.x), sizeof(uint16_t), 2, file);
      cceHostEndianToLittleEndianNewArrayInt16(&(temporary.tInfo), &(iterator->textureInfo), 4);
      temporary.tInfo.ID = cceHostEndianToLittleEndianInt32(iterator->textureInfo.ID);
      fwrite(&(temporary.tInfo), sizeof(struct Texture), 1, file);
      fwrite(iterator->textureOffsetGroups, sizeof(uint8_t), 9, file);
   }
}

static struct ExitMap2D* cce__loadExitMap2Ds (uint8_t exitMapsQuantity, FILE *file)
{
   struct ExitMap2D *exitMaps = malloc(exitMapsQuantity * sizeof(struct ExitMap2D));
   for (struct ExitMap2D *iterator = exitMaps, *end = exitMaps + exitMapsQuantity; iterator < end; ++iterator)
   {
      fread(iterator, 4, 6, file);
      fread(((int32_t*) iterator) + 6, 1, 1, file);
      cceLittleEndianToHostEndianArrayInt32(iterator, 6);
   }
   return exitMaps;
}

static void cce__writeExitMap2Ds (struct ExitMap2D *exitMaps, uint8_t exitMapsQuantity, FILE *file)
{
   uint32_t temporary[6];
   for (struct ExitMap2D *iterator = exitMaps, *end = exitMaps + exitMapsQuantity; iterator < end; ++iterator)
   {
      cceLittleEndianToHostEndianNewArrayInt32(temporary, iterator, 6);
      fwrite(temporary, 4, 6, file);
      fwrite(((int32_t*) iterator) + 6, 1, 1, file);
   }
}

struct Map2D* cceLoadMap2D (uint16_t number)
{
   cce__shortToString(mapPath, number, ".c2m");
   FILE *mapFile = fopen(mapPath, "rb");
   if (!mapFile)
   {
      cce__criticalErrorPrint("ENGINE::MAP2D::FAILED_TO_LOAD:\n%s - no such file or directory", mapPath);
   }
   *(mapPath + mapPathLength) = '\0';
   
   struct Map2D *map = (struct Map2D*) malloc(sizeof(struct Map2D));
   map->ID = number;
   map->delayedActions = LL_LIST_INIT(LL_SINGLELINKED);
   // GL elements
   {
      struct Map2DElement *elements;
      uint32_t elementsWithoutColliderQuantity, elementsCollidersQuantity;
      fread(&(map->elementsQuantity), 4u/*uint32_t*/, 1u, mapFile);
      map->elementsQuantity = cceLittleEndianToHostEndianInt32(map->elementsQuantity);
      fread(&elementsWithoutColliderQuantity, 4u/*uint32_t*/, 1u, mapFile);
      elementsWithoutColliderQuantity = cceLittleEndianToHostEndianInt32(elementsWithoutColliderQuantity);
      elementsCollidersQuantity = ELEMENTSCOLLIDERSQUANTITY(map->elementsQuantity, elementsWithoutColliderQuantity);
      struct Map2DCollider *colliders;
      colliders = NULL;
      if (map->elementsQuantity)
         elements = cce__loadMap2DElements(map->elementsQuantity, mapFile);
      fread(&(map->transformGroupsQuantity), 2u/*uint16_t*/, 1u, mapFile);
      map->transformGroupsQuantity = cceLittleEndianToHostEndianInt16(map->transformGroupsQuantity);
      map->transformGroups = cce__loadGroups(map->transformGroupsQuantity, mapFile);
      if (map->elementsQuantity)
      {
         colliders = elementsToColliders(map->elementsQuantity, elementsWithoutColliderQuantity, elements, &(map->texturesMapReliesOn), &(map->texturesMapReliesOnQuantity),
                                         map->transformGroupsQuantity, map->transformGroups);
      }
      uint32_t collidersQuantity;
      fread(&collidersQuantity, 4u, 1u, mapFile);
      collidersQuantity = cceLittleEndianToHostEndianInt32(collidersQuantity);
      map->collidersQuantity = elementsCollidersQuantity + collidersQuantity;
      map->colliders = (struct Map2DCollider*) realloc(colliders, (map->collidersQuantity) * sizeof(struct Map2DCollider));
      if (collidersQuantity)
      {
         fread((map->colliders + elementsCollidersQuantity), sizeof(struct Map2DCollider), collidersQuantity, mapFile);
         if (g_endianess == CCE_BIG_ENDIAN)
            for (struct Map2DCollider *iterator = map->colliders + elementsCollidersQuantity, *end = map->colliders + map->collidersQuantity; iterator < end; ++iterator)
            {
               cceLittleEndianToBigEndianArrayInt32(&(iterator->position), 2);
               cceLittleEndianToBigEndianArrayInt16(&(iterator->size), 2);
            }
      }
   }
   fread(&(map->collisionGroupsQuantity), 2u/*uint16_t*/, 1u, mapFile);
   map->collisionGroupsQuantity = cceLittleEndianToHostEndianInt16(map->collisionGroupsQuantity);
   map->collisionGroups = cce__loadGroups(map->collisionGroupsQuantity, mapFile);
   fread(&(map->collisionQuantity), 2u/*uint16_t*/, 1u, mapFile);
   map->collisionQuantity = cceLittleEndianToHostEndianInt16(map->collisionQuantity);
   if ((map->collisionQuantity))
   {
      (map->collision) = (struct CollisionGroup*) malloc((map->collisionQuantity) * sizeof(struct CollisionGroup));
      fread((map->collision), sizeof(struct CollisionGroup), (map->collisionQuantity), mapFile);
      cceLittleEndianToHostEndianArrayInt16(map->collision, map->collisionQuantity * 2);
   }
   else
   {
      (map->collision) = NULL;
   }
   fread(&(map->timersQuantity), 2u/*uint16_t*/, 1u, mapFile);
   map->timersQuantity = cceLittleEndianToHostEndianInt16(map->timersQuantity);
   if ((map->timersQuantity))
   {
      (map->timers) = (struct Timer*) malloc(map->timersQuantity * sizeof(struct Timer));
      struct Timer *end = (map->timers + map->timersQuantity - 1u);
      uint32_t tmp;
      for (struct Timer *iterator = (map->timers); iterator <= end; ++iterator)
      {
         fread((&(iterator->delay)), 4u/*float*/, 1u, mapFile);
         tmp = cceLittleEndianToHostEndianInt32(*((uint32_t*) &iterator->delay));
         iterator->delay = *((float*) &tmp);
         iterator->initTime = 0.0;
      }
   }
   else
   {
      (map->timers) = NULL;
   }
   fread(&(map->logicQuantity),  4u/*uint32_t*/, 1u, mapFile);
   map->logicQuantity = cceLittleEndianToHostEndianInt32(map->logicQuantity);
   if ((map->logicQuantity))
   {
      map->logic = cce__loadLogic(map->logicQuantity, mapFile, cce_endianSwapActions);
   }
   else
   {
      map->logic = NULL;
   }
   
   fread(&(map->staticActionsQuantity), 1u, 1u, mapFile);
   if (map->staticActionsQuantity)
   {
      map->staticActionIDs = (uint32_t *) malloc(map->staticActionsQuantity * sizeof(uint32_t));
      fread(map->staticActionIDs, 4u, map->staticActionsQuantity, mapFile);
      cceLittleEndianToHostEndianArrayInt32(map->staticActionIDs, map->staticActionsQuantity);
      map->staticActionArgOffsets = (uint32_t *) malloc((map->staticActionsQuantity + 1u) * sizeof(uint32_t));
      fread(map->staticActionArgOffsets + 1, 4u, map->staticActionsQuantity, mapFile);
      cceLittleEndianToHostEndianArrayInt32(map->staticActionArgOffsets + 1, map->staticActionsQuantity);
      *(map->staticActionArgOffsets) = 0u;
      map->staticActionArgs = (cce_void *) malloc((*(map->staticActionArgOffsets + map->staticActionsQuantity)) * sizeof(uint8_t));
      fread( (map->staticActionArgs), 1u, *(map->staticActionArgOffsets + map->staticActionsQuantity), mapFile);

      if (*map2Dflags & (CCE_PROCESS_LOGIC_FOR_VISIBLE_MAPS | CCE_PROCESS_LOGIC_FOR_ALL_MAPS | CCE_FORCE_INITIALIZE_MAP_ONLOAD))
      {
         cce__initLogicMap2D(map);
      }
   }
   fread(&(map->exitMapsQuantity), 1u, 1u, mapFile);
   if (map->exitMapsQuantity)
   {
      map->exitMaps = cce__loadExitMap2Ds(map->exitMapsQuantity, mapFile);
   }
   
   // I'm lazy
   for (uint8_t smth = 0u, i = 0u; i < 10; ++i)
   {
      fread(&smth, 1u, 1u, mapFile);
      if (smth)
      {
         cce__criticalErrorPrint("ENGINE::MAP2D_LOADER::PARSING_ERROR:\nfile of map %u contains fields that isn't implemented in current engine version", number);
      }
   }
   if (cce_fileParseFunc) cce_fileParseFunc(mapFile, number);
   if (fclose(mapFile) == -1)
   {
      cce__errorPrint("ENGINE::MAP2D_LOADER::FILE_UNEXPECTED_CLOSE:\nmap %u file was unexpectedly closed by external file handler", number);
   }
   return map;
}

#define CONVERT_ELEMENTGROUP(dest, src) \
do \
{ \
   dest = (struct ElementGroup*) malloc(src ## Quantity * sizeof(struct ElementGroup)); \
   struct ElementGroup *d = dest; \
   for (struct DynamicElementGroup *i = src, *jend = src + src ## Quantity; \
        i < jend; ++i, ++d) \
   { \
      d->elementsQuantity = i->elementsQuantity; \
      d->elements = (uint32_t*) malloc(i->elementsQuantity * sizeof(uint32_t)); \
      memcpy(d->elements, i->elements, (i->elementsQuantity * sizeof(uint32_t))); \
   } \
} \
while (0)

/* Do not frees mapdev */
struct Map2D* cceMap2DdevToMap2D (struct Map2Ddev *mapdev)
{
   struct Map2D *map = (struct Map2D*) malloc(sizeof(struct Map2D));
   map->ID = mapdev->ID;
   map->delayedActions = LL_LIST_INIT(LL_SINGLELINKED);
   map->transformGroupsQuantity = mapdev->transformGroupsQuantity;
   if (mapdev->moveGroupsQuantity)
   {
      CONVERT_ELEMENTGROUP(map->transformGroups, mapdev->transformGroups);
   }
   else
   {
      map->transformGroups = NULL;
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
      }
      map->collidersQuantity = elementsCollidersQuantity + mapdev->collidersQuantity;
      if (mapdev->collidersQuantity)
      {
         map->colliders = (struct Map2DCollider*) realloc(colliders, (map->collidersQuantity) * sizeof(struct Map2DCollider));
         memcpy(map->colliders + elementsCollidersQuantity, mapdev->colliders, mapdev->collidersQuantity * sizeof(struct Map2DCollider));
      }
      else
      {
         free(colliders);
      }
   }
   map->collisionGroupsQuantity = mapdev->collisionGroupsQuantity;
   if (mapdev->collisionGroupsQuantity)
   {
      CONVERT_ELEMENTGROUP(map->collisionGroups, mapdev->collisionGroups);
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
   }
   else
   {
      map->collision = NULL;
   }
   map->timersQuantity = mapdev->timersQuantity;
   if (mapdev->timersQuantity)
   {
      map->timers = (struct Timer*) malloc(mapdev->timersQuantity * sizeof(struct Timer));
      float *srcDelays = mapdev->timers;
      for(struct Timer *iterator = map->timers, *end = map->timers + mapdev->timersQuantity; iterator < end; ++iterator, ++srcDelays)
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
   }
   else
   {
      map->logic = NULL;
   }
   if (mapdev->actionsQuantity)
   {
      map->staticActionIDs = malloc(mapdev->actionsQuantity * sizeof(uint32_t));
      memcpy(map->staticActionIDs, mapdev->actionIDs, mapdev->actionsQuantity * sizeof(uint32_t));
      map->staticActionArgOffsets = malloc((mapdev->actionsQuantity + 1) * sizeof(uint32_t));
      memcpy(map->staticActionArgOffsets, mapdev->actionsArgOffsets, (mapdev->actionsQuantity + 1) * sizeof(uint32_t));
      map->staticActionArgs = malloc(*(mapdev->actionsArgOffsets + mapdev->actionsQuantity) * sizeof(cce_void));
      memcpy(map->staticActionArgs, mapdev->actionsArg, *(mapdev->actionsArgOffsets + mapdev->actionsQuantity) * sizeof(cce_void));

      if (*map2Dflags & (CCE_PROCESS_LOGIC_FOR_VISIBLE_MAPS | CCE_PROCESS_LOGIC_FOR_ALL_MAPS | CCE_FORCE_INITIALIZE_MAP_ONLOAD))
      {
         cce__initLogicMap2D(map);
      }
   }
   map->exitMapsQuantity = mapdev->exitMapsQuantity;
   if (mapdev->exitMapsQuantity)
   {
      map->exitMaps = (struct ExitMap2D*) malloc(mapdev->exitMapsQuantity * sizeof(struct ExitMap2D));
      memcpy(map->exitMaps, mapdev->exitMaps, (mapdev->exitMapsQuantity * sizeof(struct ExitMap2D)));
   }
   return map;
}

#define ELEMENTGROUP_SET_ALLOCATED(groups) \
do \
{ \
   for (struct DynamicElementGroup *iterator = groups, *end = groups + groups ## Quantity; \
        iterator < end; ++iterator) \
   { \
      iterator->elementsQuantityAllocated = iterator->elementsQuantity; \
   } \
} \
while(0)

struct Map2Ddev* cceLoadMap2Ddev (uint16_t number)
{
   cce__shortToString(mapPath, number, ".c2m");
   FILE *mapFile = fopen(mapPath, "rb");
   if (!mapFile)
   {
      cce__criticalErrorPrint("ENGINE::MAP2Ddev::FAILED_TO_LOAD:\n%s - no such file or directory", mapPath);
   }
   *(mapPath + mapPathLength) = '\0';
   struct Map2Ddev *map = (struct Map2Ddev*) calloc(1u, sizeof(struct Map2Ddev));
   map->ID = number;
   fread(&(map->elementsQuantity), 4u/*uint32_t*/, 1u, mapFile);
   map->elementsQuantity = cceLittleEndianToHostEndianInt32(map->elementsQuantity);
   fread(&(map->elementsWithoutColliderQuantity), 4u/*uint32_t*/, 1u, mapFile);
   map->elementsWithoutColliderQuantity = cceLittleEndianToHostEndianInt32(map->elementsWithoutColliderQuantity);
   if ((map->elementsQuantity))
      map->elements = cce__loadMap2DElements(map->elementsQuantity, mapFile);
   fread(&(map->moveGroupsQuantity), 2u/*uint16_t*/, 1u, mapFile);
   map->moveGroupsQuantity = cceLittleEndianToHostEndianInt16(map->moveGroupsQuantity);
   map->moveGroups = (struct DynamicElementGroup*) cce__loadGroups(map->moveGroupsQuantity, mapFile);
   ELEMENTGROUP_SET_ALLOCATED(map->moveGroups);
   fread(&(map->extensionGroupsQuantity), 2u/*uint16_t*/, 1u, mapFile);
   map->extensionGroupsQuantity = cceLittleEndianToHostEndianInt16(map->extensionGroupsQuantity);
   map->extensionGroups = (struct DynamicElementGroup*) cce__loadGroups(map->extensionGroupsQuantity, mapFile);
   ELEMENTGROUP_SET_ALLOCATED(map->moveGroups);
   fread(&(map->collidersQuantity), 4u/*uint32_t*/, 1u, mapFile);
   map->collidersQuantity = cceLittleEndianToHostEndianInt32(map->collidersQuantity);
   if (map->collidersQuantity)
   {
      map->colliders = (struct Map2DCollider*) malloc(map->collidersQuantity * sizeof(struct Map2DCollider));
      fread(map->colliders, sizeof(struct Map2DCollider), map->collidersQuantity, mapFile);
      if (g_endianess == CCE_BIG_ENDIAN)
         for (struct Map2DCollider *iterator = map->colliders, *end = map->colliders + map->collidersQuantity; iterator < end; ++iterator)
         {
            cceLittleEndianToBigEndianArrayInt32(&(iterator->x), 2);
            cceLittleEndianToBigEndianArrayInt16(&(iterator->width), 2);
         }
   }
   fread(&(map->collisionGroupsQuantity), 2u/*uint16_t*/, 1u, mapFile);
   map->collisionGroupsQuantity = cceLittleEndianToHostEndianInt16(map->collisionGroupsQuantity);
   map->collisionGroups = (struct DynamicElementGroup*) cce__loadGroups(map->collisionGroupsQuantity, mapFile);
   ELEMENTGROUP_SET_ALLOCATED(map->moveGroups);
   fread(&(map->collisionQuantity), 2u/*uint16_t*/, 1u, mapFile);
   map->collisionQuantity = cceLittleEndianToHostEndianInt16(map->collisionQuantity);
   if (map->collisionQuantity)
   {
      map->collision = (struct CollisionGroup*) malloc(map->collisionQuantity * sizeof(struct CollisionGroup));
      fread(map->collision, sizeof(struct CollisionGroup), map->collisionQuantity, mapFile);
      cceLittleEndianToHostEndianArrayInt16(&(map->collision), map->collisionQuantity * 2);
   }
   fread(&(map->timersQuantity), 2u/*uint16_t*/, 1u, mapFile);
   map->timersQuantity = cceLittleEndianToHostEndianInt16(map->timersQuantity);
   if ((map->timersQuantity))
   {
      (map->timers) = (float*) malloc(map->timersQuantity * sizeof(float));
      fread((map->timers), 4u/*float*/, map->timersQuantity, mapFile);
      cceLittleEndianToHostEndianArrayInt32(&(map->timers), map->timersQuantity);
   }
   fread(&(map->logicQuantity), 4u/*uint32_t*/, 1u, mapFile);
   map->logicQuantity = cceLittleEndianToHostEndianInt16(map->logicQuantity);
   if (map->logicQuantity)
   {
      map->logic = cce__loadLogic(map->logicQuantity, mapFile, cce_endianSwapActions);
   }
   fread(&(map->actionsQuantity), 1u/*uint8_t*/, 1u, mapFile);
   if (map->actionsQuantity)
   {
      map->actionIDs = (uint32_t *) malloc((map->actionsQuantity) * sizeof(uint32_t));
      fread( (map->actionIDs),             4u/*uint32_t*/,   (map->actionsQuantity),                                           mapFile);
      cceLittleEndianToHostEndianArrayInt32(map->actionIDs, map->actionsQuantity);
      map->actionsArgOffsets = (uint32_t*) malloc((map->actionsQuantity + 1u) * sizeof(uint32_t));
      *(map->actionsArgOffsets) = 0u;
      fread( (map->actionsArgOffsets + 1), 4u/*uint32_t*/, (map->actionsQuantity),                                           mapFile);
      cceLittleEndianToHostEndianArrayInt32(map->actionsArgOffsets + 1, map->actionsQuantity);
      map->actionsArg = (cce_void *) malloc((*(map->actionsArgOffsets + map->actionsQuantity) - 1u) /*sizeof(cce_void)*/);
      fread( (map->actionsArg),            1u/*cce_void*/,         *(map->actionsArgOffsets + map->actionsQuantity),          mapFile);
   }
   fread(&(map->exitMapsQuantity), 1u/*uint8_t*/, 1u, mapFile);
   if (map->exitMapsQuantity)
   {
      map->exitMaps = cce__loadExitMap2Ds(map->exitMapsQuantity, mapFile);
   }
   
   // I'm lazy, again
   for (uint8_t smth = 0u, i = 0u; i < 10; ++i)
   {
      fread(&smth, 1u, 1u, mapFile);
      if (smth)
      {
         cce__criticalErrorPrint("ENGINE::MAP2Ddev_LOADER::PARSING_ERROR:\nfile of map %u contains fields that isn't implemented in current engine version", number);
      }
   }
   if (cce_fileParseFunc) cce_fileParseFunc(mapFile, number);
   if (fclose(mapFile) == -1)
   {
      cce__errorPrint("ENGINE::MAP2Ddev_LOADER::FILE_UNEXPECTED_CLOSE:\nmap %u file was unexpectedly closed by external file handler", number);
   }
   return map;
}

int cceWriteMap2Ddev (struct Map2Ddev *map, void (*writeFunc)(FILE*))
{
   union
   {
      uint32_t u32;
      uint16_t u16;
      uint16_t arr16[2];
   } temporary;
   cce__shortToString(mapPath, map->ID, ".c2m");
   FILE *mapFile = fopen(mapPath, "wb");
   if (!mapFile)
   {
      cce__errorPrint("ENGINE::MAP2Ddev::FAILED_TO_OPEN_FILE:\n%s - cannot open file. Are you haven't enough free space left? Are you have a directory with same name? Does directory really exist?\n", mapPath);
      return -1;
   }
   *(mapPath + mapPathLength) = '\0';
   temporary.u32 = cceHostEndianToLittleEndianInt32(map->elementsQuantity);
   fwrite(&(temporary.u32), 4/*uint32_t*/, 1, mapFile);
   temporary.u32 = cceHostEndianToLittleEndianInt32(map->elementsWithoutColliderQuantity);
   fwrite(&(temporary.u32), 4/*uint32_t*/, 1, mapFile);
   if ((map->elementsQuantity))
   {
      cce__writeMap2DElements(map->elements, map->elementsQuantity, mapFile);
   }
   temporary.u16 = cceHostEndianToLittleEndianInt16(map->moveGroupsQuantity);
   fwrite(&(temporary.u16), 2/*uint16_t*/, 1, mapFile);
   if ((map->moveGroupsQuantity))
   {
      cce__writeGroups(map->moveGroupsQuantity, (struct ElementGroup*) map->moveGroups, mapFile);
   }
   temporary.u16 = cceHostEndianToLittleEndianInt16(map->extensionGroupsQuantity);
   fwrite(&(temporary.u16), 2/*uint16_t*/, 1, mapFile);
   if ((map->extensionGroupsQuantity))
   {
      cce__writeGroups(map->extensionGroupsQuantity, (struct ElementGroup*) map->extensionGroups, mapFile);
   }
   temporary.u32 = cceHostEndianToLittleEndianInt32(map->collidersQuantity);
   fwrite(&(temporary.u32), 4/*uint32_t*/, 1, mapFile);
   if ((map->collidersQuantity))
   {
      if (*g_endianess == CCE_BIG_ENDIAN)
      {
         struct Map2DCollider collider;
         for (struct Map2DCollider *iterator = map->colliders, *end = map->colliders + map->collidersQuantity; iterator < end; ++iterator)
         {
            collider = *iterator;
            cceBigEndianToLittleEndianArrayInt32(&(collider.x), 2);
            cceBigEndianToLittleEndianArrayInt16(&(collider.width), 2);
            fwrite(&collider, sizeof(struct Map2DCollider), 1, mapFile);
         }
      }
      else
      {
         fwrite(map->colliders, sizeof(struct Map2DCollider), map->collidersQuantity, mapFile);
      }
   }
   temporary.u16 = cceHostEndianToLittleEndianInt16(map->collisionGroupsQuantity);
   fwrite(&(temporary.u16), 2/*uint16_t*/, 1, mapFile);
   if ((map->collisionGroupsQuantity))
   {
      cce__writeGroups(map->collisionGroupsQuantity, (struct ElementGroup*) map->collisionGroups, mapFile);
   }
   temporary.u16 = cceHostEndianToLittleEndianInt16(map->collisionQuantity);
   fwrite(&(temporary.u16), 2/*uint16_t*/, 1, mapFile);
   if ((map->collisionQuantity))
   {
      if (*g_endianess == CCE_BIG_ENDIAN)
      {
         for (struct CollisionGroup *iterator = map->collision, *end = map->collision + map->collisionQuantity; iterator < end; ++iterator)
         {
            cceBigEndianToLittleEndianNewArrayInt16(temporary.arr16, iterator, 2);
            fwrite(temporary.arr16, sizeof(struct CollisionGroup), 1, mapFile);
         }
      }
      else
      {
         fwrite((map->collision), sizeof(struct CollisionGroup), (map->collisionQuantity), mapFile);
      }
   }
   temporary.u16 = cceHostEndianToLittleEndianInt16(map->timersQuantity);
   fwrite(&(temporary.u16), 2/*uint16_t*/, 1, mapFile);
   if ((map->timersQuantity))
   {
      if (*g_endianess == CCE_BIG_ENDIAN)
      {
         for (uint32_t *iterator = (uint32_t*) map->timers, *end = (uint32_t*) map->timers + map->timersQuantity; iterator < end; ++iterator)
         {
            temporary.u32 = cceBigEndianToLittleEndianInt32(*iterator);
            fwrite(&(temporary.u32), 4, 1, mapFile);
         }
      }
      else
      {
         fwrite((map->timers), 4/*float*/, (map->timersQuantity), mapFile);
      }
   }
   temporary.u32 = cceHostEndianToLittleEndianInt32(map->logicQuantity);
   fwrite(&(temporary.u32), 4/*uint32_t*/, 1, mapFile);
   if (map->logicQuantity)
   {
      cce__writeLogic(map->logicQuantity, map->logic, mapFile, cce_endianSwapActions);
   }
   fwrite(&(map->actionsQuantity), 1/*uint8_t*/, 1, mapFile);
   if (map->actionsQuantity)
   {
      if (*g_endianess == CCE_BIG_ENDIAN)
      {
         for (uint32_t *iterator = map->actionIDs, *end = map->actionIDs + map->actionsQuantity; iterator < end; ++iterator)
         {
            temporary.u32 = cceBigEndianToLittleEndianInt32(*iterator);
            fwrite(&(temporary.u32), 4, 1, mapFile);
         }
         for (uint32_t *iterator = map->actionsArgOffsets + 1, *end = map->actionsArgOffsets + 1 + map->actionsQuantity; iterator < end; ++iterator)
         {
            temporary.u32 = cceBigEndianToLittleEndianInt32(*iterator);
            fwrite(&(temporary.u32), 4, 1, mapFile);
         }
      }
      else
      {
         fwrite( (map->actionIDs),             4/*uint32_t*/,  (map->actionsQuantity),                          mapFile);
         fwrite( (map->actionsArgOffsets + 1), 4/*uint32_t*/,  (map->actionsQuantity),                          mapFile);
      }
      fwrite( (map->actionsArg),            1/*cce_void*/, *(map->actionsArgOffsets + map->actionsQuantity), mapFile);
   }
   fwrite(&(map->exitMapsQuantity), 1/*uint8_t*/, 1, mapFile);
   if (map->exitMapsQuantity)
   {
      cce__writeExitMap2Ds(map->exitMaps, map->exitMapsQuantity, mapFile);
   }
   
   // I'm lazy, again
   uint8_t null = 0u;
   for (uint8_t i = 0u; i < 10; ++i)
   {
      fwrite(&null, 1u/*uint8_t*/, 1u, mapFile);
   }
   if (writeFunc) writeFunc(mapFile);
   if (fclose(mapFile) == -1)
   {
      cce__errorPrint("ENGINE::MAP2Ddev_MAPWRITER::FILE_UNEXPECTED_CLOSE:\nmap %u file was unexpectedly closed by external file handler", map->ID);
   }
   return 0;
}
