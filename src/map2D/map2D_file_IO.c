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

#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "../../include/cce/engine_common.h"
#include "../../include/cce/engine_common_IO.h"
#include "../../include/cce/os_interaction.h"
#include "../../include/cce/endianess.h"
#include "../../include/cce/utils.h"
#include "../../include/cce/map2D/map2D.h"

#include "../engine_common_internal.h"
#include "map2D_internal.h"
#include "../platform/platforms.h"

static char *mapPath = NULL;
static size_t mapPathLength = 0;
uint16_t cce__staticMapFunctionSet, cce__dynamicMapFunctionSet;
static ptrdiff_t g_resourceLoadersOffset, g_renderingInfoOffset;

static size_t resourceSpaceToBeAllocated;

CCE_ARRAY(resourceLoadingFunctions, static cce_rloadfun, static uint16_t);
static cce_dataparsefun *resourceUnloadingFunctions;
static cce_dataparsefun *resourceCreatingFunctions;
static cce_rstorefun *resourceStoringFunctions;
static size_t *resourceLoadingFunctionsBufferSizes;

CCE_PUBLIC_OPTIONS void cceSetMap2Dpath (const char *path)
{
   CCE_SET_PATH(mapPath, mapPathLength, path);
}

CCE_PUBLIC_OPTIONS uint32_t cceRegisterMapCustomResourceCallback (cce_rloadfun onLoad, cce_dataparsefun onFree, cce_dataparsefun onCreate, cce_rstorefun onStore, size_t bufferSize)
{
   if (resourceLoadingFunctionsQuantity >= resourceLoadingFunctionsAllocated)
   {
      CCE_REALLOC_ARRAY(resourceLoadingFunctions, resourceLoadingFunctionsQuantity + 1);
      resourceUnloadingFunctions = realloc(resourceUnloadingFunctions, resourceLoadingFunctionsAllocated * sizeof(cce_dataparsefun*));
      resourceCreatingFunctions  = realloc(resourceCreatingFunctions,  resourceLoadingFunctionsAllocated * sizeof(cce_dataparsefun*));
      resourceStoringFunctions   = realloc(resourceStoringFunctions,   resourceLoadingFunctionsAllocated * sizeof(cce_rstorefun*));
      resourceLoadingFunctionsBufferSizes = realloc(resourceLoadingFunctionsBufferSizes, (resourceLoadingFunctionsAllocated + 1) * sizeof(size_t));
   }
   resourceLoadingFunctions[resourceLoadingFunctionsQuantity] = onLoad;
   resourceUnloadingFunctions[resourceLoadingFunctionsQuantity] = onFree;
   resourceCreatingFunctions[resourceLoadingFunctionsQuantity] = onCreate;
   resourceStoringFunctions[resourceLoadingFunctionsQuantity] = onStore;
   resourceLoadingFunctionsBufferSizes[resourceLoadingFunctionsQuantity] = bufferSize;
   resourceSpaceToBeAllocated += bufferSize;
   return resourceLoadingFunctionsQuantity++ | 0x80000000;
}

static struct ElementGroup* cce__loadGroups (uint16_t groupsQuantity, struct ElementGroup *buffer, FILE *map_f)
{
   if (!groupsQuantity)
   {
      return NULL;
   }
   for (struct ElementGroup *iterator = buffer, *end = (buffer + groupsQuantity); iterator < end; ++iterator)
   {
      fread(&(iterator->elementsQuantity), 2u/*uint16_t*/, 1u, map_f);
      iterator->elementsQuantity = cceLittleEndianToHostEndianInt16(iterator->elementsQuantity);
      if (iterator->elementsQuantity)
      {
         iterator->elements = (uint32_t*) malloc(iterator->elementsQuantity * sizeof(uint32_t));
         fread(iterator->elements, 4u/*uint32_t*/, iterator->elementsQuantity, map_f);
         cceLittleEndianToHostEndianArrayInt32(iterator->elements, iterator->elementsQuantity);
      }
      else
      {
         iterator->elements = NULL;
      }
   }
   return buffer;
}

void cce__writeGroups (uint16_t groupsQuantity, struct ElementGroup *groups, FILE *map_f)
{
   for (struct ElementGroup *iterator = groups, *end = (groups + groupsQuantity); iterator < end; ++iterator)
   {
      uint16_t elementsQuantity = cceHostEndianToLittleEndianInt16(iterator->elementsQuantity);
      fwrite(&elementsQuantity, 2u/*uint16_t*/, 1u, map_f);
      if (!iterator->elementsQuantity)
         continue;

      if (*g_endianess == CCE_BIG_ENDIAN)
      {
         uint32_t elementID;
         for (uint32_t *jiterator = iterator->elements, *jend = iterator->elements + iterator->elementsQuantity; jiterator < jend; ++jiterator)
         {
            elementID = cceBigEndianToLittleEndianInt32(*jiterator);
            fwrite(&elementID, 4u/*uint32_t*/, 1, map_f);
         }
      }
      else
      {
         fwrite(iterator->elements, 4u/*uint32_t*/, iterator->elementsQuantity, map_f);
      }
   }
}

#define LOADELEMENTS(buffer, sectionSize, info, file, elementDataAlloc, textureDataAlloc, elementsQuantityAlloc) \
fread(&elementDataQuantity, sizeof(uint16_t), sectionSize > 0, file); \
elementDataQuantity = cceLittleEndianToHostEndianInt16(elementDataQuantity); \
fread(&textureDataQuantity, sizeof(uint16_t), sectionSize > 0, file); \
textureDataQuantity = cceLittleEndianToHostEndianInt16(textureDataQuantity); \
fread(&elementsTotal,       sizeof(uint32_t), sectionSize > 0, file); \
elementsTotal = cceLittleEndianToHostEndianInt32(elementsTotal); \
struct Map2DElementData     *elementData = elementDataAlloc; \
struct cce_texture2D        *textureData = textureDataAlloc; \
elementsQuantity                         = elementsQuantityAlloc; \
fread(elementsQuantity,     sizeof(uint32_t), sectionSize,     file); \
cceLittleEndianToHostEndianArrayInt32(elementsQuantity, sectionSize); \
fread(elementData, sizeof(struct Map2DElementData), elementDataQuantity, file); \
fread(textureData, sizeof(struct cce_texture2D), textureDataQuantity, file); \
cceLittleEndianToHostEndianArrayInt16(textureData, textureDataQuantity * 5); \
uint16_t *usedTextures = ((struct UsedTexturesInfo*)(((struct ResourceInfo*)((cce_void*) info + g_resourceLoadersOffset))->resourceData))->texturesMapDependsOn; \
for (struct cce_texture2D *iterator = textureData, *end = textureData + textureDataQuantity; iterator < end; ++iterator) \
   if (iterator->ID > 0) \
      iterator->ID = usedTextures[iterator->ID - 1]

static int loadElements (void *buffer, uint8_t sectionSize, struct cce_buffer *info, FILE *file)
{
   struct RenderingInfo *map = buffer;
   uint32_t *elementsQuantity;
   uint32_t elementsTotal;
   uint16_t elementDataQuantity = 0, textureDataQuantity = 0;
   LOADELEMENTS(buffer, sectionSize, info, file, malloc(elementsTotal * sizeof(struct Map2DElementPosition) + sectionSize * sizeof(struct Map2DElementPositionArray) +
                elementDataQuantity * sizeof(struct Map2DElementData) + textureDataQuantity * sizeof(struct cce_texture2D)),
                (struct cce_texture2D*)(elementData + elementDataQuantity), (uint32_t*)(textureData + textureDataQuantity));
   struct Map2DElementPositionArray *elements = (struct Map2DElementPositionArray*)elementsQuantity;
   elements[sectionSize - 1].dataAllocated = elements[sectionSize - 1].dataQuantity = elementsQuantity[sectionSize - 1];
   elements[sectionSize - 1].data = (struct Map2DElementPosition*)(elements + sectionSize) + elementsTotal - elementsQuantity[sectionSize - 1];
   uint32_t *qiterator = elementsQuantity + sectionSize - 2;
   for (struct Map2DElementPositionArray *iterator = elements + sectionSize - 1, *end = elements; iterator > end; --iterator, --qiterator)
   {
      iterator[-1].dataQuantity = *qiterator;
      iterator[-1].dataAllocated = *qiterator;
      iterator[-1].data = iterator->data - *qiterator;
   }
   fread(elements->data, sizeof(struct Map2DElementPosition), elementsTotal, file);
   map->data = cce__map2DElementsToRenderingBuffer(elements, sectionSize, textureData, textureDataQuantity, elementData, elementDataQuantity);
   free(elementData);
   return 0;
}

static int loadElementsDynamic (void *buffer, uint8_t sectionSize, struct cce_buffer *info, FILE *file)
{
   struct DynamicRenderingInfo *map = buffer;
   uint32_t *elementsQuantity;
   uint32_t elementsTotal;
   uint16_t elementDataQuantity = 0, textureDataQuantity = 0;
   LOADELEMENTS(buffer, sectionSize, info, file, malloc(sectionSize * sizeof(struct Map2DElementPositionArray)),
                malloc(elementDataQuantity * sizeof(struct Map2DElementData)), malloc(textureDataQuantity * sizeof(struct cce_texture2D)));
   map->elements = (struct Map2DElementPositionArray*) elementsQuantity;
   uint32_t *qiterator = elementsQuantity + sectionSize;
   for (struct Map2DElementPositionArray *iterator = map->elements + sectionSize, *end = map->elements; iterator > end;)
   {
      --iterator, --qiterator;
      iterator->dataQuantity = *qiterator;
      iterator->dataAllocated = *qiterator;
      iterator->data = malloc(*qiterator * sizeof(struct Map2DElementPosition));
      fread(iterator->data, sizeof(struct Map2DElementPosition), *qiterator, file);
   }
   map->elementData = elementData;
   map->textureInfo = textureData;
   map->elementDataQuantity  = elementDataQuantity;
   map->elementDataAllocated = elementDataQuantity;
   map->textureInfoQuantity  = textureDataQuantity;
   map->textureInfoAllocated = textureDataQuantity;
   map->data = cce__map2DElementsToRenderingBuffer(map->elements, sectionSize, textureData, textureDataQuantity, elementData, elementDataQuantity);
   return 0;
}

#define LOADCOLLIDERS(map, sectionSize, file, transformGroupsAlloc, collisionGroupsAlloc, collisionAlloc, collisionCacheAlloc, collidersAlloc) \
fread(&map->collidersQuantity, sizeof(uint32_t), sectionSize > 0, file); \
map->collidersQuantity = cceLittleEndianToHostEndianInt32(map->collidersQuantity); \
fread(&map->transformGroupsQuantity, sizeof(uint16_t), CCE_MAX(sectionSize - 1, 3), file); \
cceLittleEndianToHostEndianArrayInt16(&map->transformGroupsQuantity, 3); \
map->transformGroups = transformGroupsAlloc; \
map->collisionGroups = collisionGroupsAlloc; \
map->collisionCache  = collisionCacheAlloc; \
map->collision       = collisionAlloc; \
map->colliders       = collidersAlloc; \
fread(map->colliders, sizeof(union Collider2D), map->collidersQuantity, file); \
if (*g_endianess == CCE_BIG_ENDIAN) \
{ \
   for (union Collider2D *iterator = map->colliders, *end = map->colliders + map->collidersQuantity; iterator < end; ++iterator) \
   { \
      cceLittleEndianToBigEndianArrayInt32(&iterator->rectangle.position, 2); \
      if (cce__map2Dflags & CCE_CIRCLE_COLLIDER) \
         iterator->circle.radius = cceLittleEndianToBigEndianInt16(iterator->circle.radius); \
   } \
} \
cce__loadGroups(map->transformGroupsQuantity, map->transformGroups, file); \
cce__loadGroups(map->collisionGroupsQuantity, map->collisionGroups, file); \
fread(map->collision, sizeof(struct CollisionGroup), map->collisionGroupsQuantity, file); \
cceLittleEndianToHostEndianArrayInt16(map->collision, 2 * map->collisionGroupsQuantity); \
memset(map->collisionCache, 0, map->collisionQuantity * sizeof(uint64_t))

static int loadColliders (void *buffer, uint8_t sectionSize, struct cce_buffer *info, FILE *file)
{
   CCE_UNUSED(info);
   struct CollisionInfo *map = buffer;
   LOADCOLLIDERS(map, sectionSize, file, 
                 malloc(map->collidersQuantity       * sizeof(union Collider2D)    + map->transformGroupsQuantity * sizeof(struct ElementGroup) + 
                        map->collisionGroupsQuantity * sizeof(struct ElementGroup) + map->collisionQuantity       * (sizeof(struct CollisionGroup) + sizeof(uint64_t))),
                 (map->transformGroups + map->transformGroupsQuantity), (struct CollisionGroup*)(map->collisionCache + map->collisionGroupsQuantity),
                 (uint64_t*)(map->collisionGroups + map->collisionGroupsQuantity), (union Collider2D*)(map->colliders + map->collidersQuantity));
   return 0;
}

static int loadCollidersDynamic (void *buffer, uint8_t sectionSize, struct cce_buffer *info, FILE *file)
{
   CCE_UNUSED(info);
   struct DynamicCollisionInfo *map = buffer;
   LOADCOLLIDERS(map, sectionSize, file,
                 malloc(map->transformGroupsQuantity * sizeof(struct ElementGroup)),
                 malloc(map->collisionGroupsQuantity * sizeof(struct ElementGroup)),
                 malloc(map->collisionQuantity       * sizeof(struct CollisionGroup)),
                 malloc(map->collisionQuantity       * sizeof(uint64_t)),
                 malloc(map->collidersQuantity       * sizeof(union Collider2D)));
   map->collidersAllocated = map->collidersQuantity;
   map->transformGroupsAllocated = map->transformGroupsQuantity;
   map->collisionGroupsAllocated = map->collisionGroupsQuantity;
   map->collisionAllocated = map->collisionQuantity;
   return 0;
}

static void createElements (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   struct DynamicRenderingInfo *map = buffer;
   map->elements = NULL;
   map->textureInfo = NULL;
   map->elementData = NULL;
   map->layersQuantity = 0;
   map->textureInfoQuantity = 0;
   map->textureInfoAllocated = 0;
   map->elementDataQuantity = 0;
   map->elementDataAllocated = 0;
   map->data = cce__map2DElementsToRenderingBuffer(NULL, 0, NULL, 0, NULL, 0);
}

static void createColliders (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   struct DynamicCollisionInfo *map = buffer;
   map->colliders = NULL;
   map->transformGroups = NULL;
   map->collisionGroups = NULL;
   map->collision = NULL;
   map->collisionCache = NULL;
   map->collidersQuantity = 0;
   map->collidersAllocated = 0;
   map->transformGroupsQuantity = 0;
   map->transformGroupsAllocated = 0;
   map->collisionGroupsQuantity = 0;
   map->collisionGroupsAllocated = 0;
   map->collisionQuantity = 0;
   map->collisionAllocated = 0;
}

static void freeElements (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   struct RenderingInfo *map = buffer;
   cce__deleteMap2DRenderingBuffer(map->data);
}

static void freeElementsDynamic (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   struct DynamicRenderingInfo *map = buffer;
   cce__deleteMap2DRenderingBuffer(map->data);
   for (struct Map2DElementPositionArray *iterator = map->elements, *end = map->elements; iterator < end; ++iterator)
   {
      free(iterator->data);
   }
   free(map->elements);
   free(map->elementData);
   free(map->textureInfo);
}

static void freeColliders (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   struct CollisionInfo *map = buffer;
   free(map->transformGroups);
}

static void freeCollidersDynamic (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   struct DynamicCollisionInfo *map = buffer;
   free(map->colliders);
   free(map->transformGroups);
   free(map->collisionGroups);
   free(map->collision);
   free(map->collisionCache);
}

static uint8_t storeElements (void *buffer, struct cce_buffer *info, FILE *file)
{
   struct DynamicRenderingInfo *map = buffer;
   
   uint8_t sectionSize = map->layersQuantity;
   if (sectionSize == 0)
      return 0;
   {
      uint16_t tmp = cceHostEndianToLittleEndianInt16(map->elementDataQuantity);
      fwrite(&tmp, sizeof(uint16_t), 1, file);
      tmp = cceHostEndianToLittleEndianInt16(map->textureInfoQuantity);
      fwrite(&tmp, sizeof(uint16_t), 1, file);
      fseek(file, sizeof(uint32_t), SEEK_CUR);
      uint32_t elementsTotalSize = 0;
      uint32_t tmp2;
      for (struct Map2DElementPositionArray *iterator = map->elements, *end = map->elements + map->layersQuantity; iterator < end; ++iterator)
      {
         elementsTotalSize += iterator->dataQuantity;
         tmp2 = cceHostEndianToLittleEndianInt32(iterator->dataQuantity);
         fwrite(&tmp2, sizeof(uint32_t), 1, file);
      }
      fseek(file, -((long)((map->layersQuantity + 1) * sizeof(uint32_t))), SEEK_CUR);
      fwrite(&elementsTotalSize, sizeof(uint32_t), 1, file);
      fseek(file, (long)(map->layersQuantity * sizeof(uint32_t)), SEEK_CUR);
   }
   fwrite(map->elementData, sizeof(struct Map2DElementData), map->elementDataQuantity, file); // All fields are 1-byte, no endian swap is performed
   uint16_t *dependantTextures =         (**(struct UsedTexturesInfo**)((cce_void*) info + g_resourceLoadersOffset)).texturesMapDependsOn;
   uint16_t  dependantTexturesQuantity = (**(struct UsedTexturesInfo**)((cce_void*) info + g_resourceLoadersOffset)).texturesMapDependsOnQuantity;
   for (struct cce_texture2D tmp, *iterator = map->textureInfo, *end = map->textureInfo + map->textureInfoQuantity; iterator < end; ++iterator)
   {
      if (iterator->ID == 0)
      {
         tmp.ID = 0;
         goto TEXTUREID_SET;
      }
      for (uint16_t *jiterator = dependantTextures, *jend = dependantTextures + dependantTexturesQuantity; jiterator < jend; ++jiterator)
      {
         if (*jiterator == iterator->ID)
         {
            tmp.ID = dependantTextures - jiterator + 1;
            goto TEXTUREID_SET;
         }
      }
      fprintf(stderr, "ENGINE::MAP2DLOADERS::RENDERING_DATA_STORE_ERROR:\nTextureID specified in element not found in DependantTextures array.\n");
      tmp.ID = 0;
TEXTUREID_SET:
      cceHostEndianToLittleEndianNewArrayInt16(&tmp.position, &iterator->position, 4);
      fwrite(&tmp, sizeof(struct cce_texture2D), 1, file);
   }
   for (struct Map2DElementPositionArray *iterator = map->elements, *end = map->elements + map->layersQuantity; iterator < end; ++iterator)
   {
      if (g_endianess == CCE_BIG_ENDIAN)
      {
         for (struct Map2DElementPosition tmp, *jiterator = iterator->data, *jend = iterator->data + iterator->dataQuantity; jiterator < jend ;++jiterator)
         {
            cceSwapEndianNewArrayIntN(&tmp, jiterator, 4, 2);
            fwrite(&tmp, sizeof(struct Map2DElementPosition), 1, file);
         }
      }
      else
      {
         fwrite(iterator->data, sizeof(struct Map2DElementPosition), iterator->dataQuantity, file);
      }
   }
   return sectionSize;
}

static uint8_t storeColliders (void *buffer, struct cce_buffer *info, FILE *file)
{
   CCE_UNUSED(info);
   struct CollisionInfo *map = buffer;
   uint8_t sectionSize = 4;
   
   for (uint16_t *iterator = &map->collisionQuantity, *end = &map->transformGroupsQuantity; iterator >= end && *iterator == 0; --iterator, --sectionSize){}
   
   if (sectionSize == 1 && map->collidersQuantity == 0)
      return 0;
   
   {
      uint32_t tmp = cceHostEndianToLittleEndianInt32(map->collidersQuantity);
      fwrite(&tmp, sizeof(uint32_t),  sectionSize > 0, file);
      uint16_t tmp2 = cceHostEndianToLittleEndianInt16(map->transformGroupsQuantity);
      fwrite(&tmp2, sizeof(uint16_t), sectionSize > 1, file);
      tmp2 = cceHostEndianToLittleEndianInt16(map->collisionGroupsQuantity);
      fwrite(&tmp2, sizeof(uint16_t), sectionSize > 2, file);
      tmp2 = cceHostEndianToLittleEndianInt16(map->collisionQuantity);
      fwrite(&tmp2, sizeof(uint16_t), sectionSize > 3, file);
   }
   
   if (*g_endianess == CCE_BIG_ENDIAN)
   {
      union Collider2D tmp;
      for (union Collider2D *iterator = map->colliders, *end = map->colliders + map->collidersQuantity; iterator < end; ++iterator)
      {
         cceBigEndianToLittleEndianNewArrayInt32(&tmp, &iterator->rectangle.position, 2);
         if (cce__map2Dflags & CCE_CIRCLE_COLLIDER)
            tmp.circle.radius = cceBigEndianToLittleEndianInt16(iterator->circle.radius);
         fwrite(&tmp, sizeof(union Collider2D), 1, file);
      }
   }
   else
   {
      fwrite(map->colliders, sizeof(union Collider2D), map->collidersQuantity, file);
   }
   cce__writeGroups(map->transformGroupsQuantity, map->transformGroups, file);
   cce__writeGroups(map->collisionGroupsQuantity, map->collisionGroups, file);
   
   if (*g_endianess == CCE_BIG_ENDIAN)
   {
      struct CollisionGroup tmp;
      for (struct CollisionGroup *iterator = map->collision, *end = map->collision + map->collisionGroupsQuantity; iterator < end; ++iterator)
      {
         cceBigEndianToLittleEndianNewArrayInt16(&tmp, iterator, 2);
         fwrite(&tmp, sizeof(struct CollisionGroup), 1, file);
      }
   }
   else
   {
      fwrite(map->collision, sizeof(struct CollisionGroup), map->collisionGroupsQuantity, file);
   }
   return sectionSize;
}

static int loadResourcesSection (void *buffer, uint8_t sectionSize, struct cce_buffer *info, FILE *file)
{
   struct ResourceInfo *map = buffer;
   uint32_t resourceSizes[256]; // VLA support across C compilers is still bad (and most likely wont improve)
   fread(resourceSizes + 1, sizeof(uint32_t), sectionSize, file);
   cceLittleEndianToHostEndianArrayInt32(resourceSizes + 1, sectionSize);
   if (sectionSize > resourceLoadingFunctionsQuantity)
   {
      for (uint32_t *iterator = resourceSizes + resourceLoadingFunctionsQuantity, *end = resourceSizes + sectionSize; iterator < end; ++iterator)
      {
         if (*iterator != 0)
         {
            fprintf(stderr, "ENGINE::MAP2D_LOADING::NONEMPTY_RESOURCE_WITHOUT_LOADER:\nMap2D cannot be loaded because some resource required for map to function does not have corresponding loader");
            return -1;
         }
      }
   }
   uint32_t maxSize = 0;
   for (uint32_t *iterator = resourceSizes + 1, *end = resourceSizes + 1 + sectionSize; iterator < end; ++iterator)
   {
      maxSize = (*iterator > maxSize) ? *iterator : maxSize;
   }
   char *buf, **names;
   buf = malloc(maxSize * sizeof(char));
   uint32_t namesAllocated = maxSize / 16; // Approximation
   names = malloc(namesAllocated * sizeof(char*));
   cce_rloadfun *fun = resourceLoadingFunctions;
   cce_dataparsefun *initFun = resourceCreatingFunctions;
   size_t dataBufferSize;
   {
      uint32_t *iterator = resourceSizes + sectionSize;
      size_t *bufferSizes = resourceLoadingFunctionsBufferSizes + sectionSize - 1;
      resourceSizes[0] = 1; // Workaround to avoid out-of-bounds check
      dataBufferSize = resourceSpaceToBeAllocated;
      while (*iterator != 0)
      {
         dataBufferSize -= *bufferSizes;
         --iterator;
         --bufferSizes;
      }
      sectionSize = (iterator - resourceSizes);
   }
   map->resourceData = malloc(dataBufferSize);
   map->resourcesQuantity = sectionSize;
   cce_void *jiterator = map->resourceData;
   size_t *bufferSizes = resourceLoadingFunctionsBufferSizes;
   for (uint32_t *iterator = resourceSizes + 1, *end = iterator + sectionSize; iterator < end; ++iterator, ++fun, ++initFun, jiterator += *bufferSizes++)
   {
      if (*iterator == 0)
      {
         if (*initFun != NULL)
            (*initFun)(jiterator, info);
         continue;
      }
      
      fread(buf, sizeof(char), *iterator, file);
      for (char *it = buf, **jit = names, *iend = buf + *iterator;; ++jit, it += strlen(it) + 1)
      {
         ptrdiff_t namesQuantity = jit - names;
         if (namesQuantity > namesAllocated)
         {
            CCE_REALLOC_ARRAY(names, namesAllocated + 1);
            jit = names + namesQuantity;
         }
         if (it >= iend)
         {
            *jit = NULL;
            break;
         }
         *jit = it;
      }
      (*fun)(jiterator, info, names);
   }
   free(buf);
   free(names);
   return 0;
}

static void createResourcesSection (void *buffer, struct cce_buffer *info)
{
   struct ResourceInfo *map = buffer;
   map->resourceData = malloc(resourceSpaceToBeAllocated);
   map->resourcesQuantity = resourceLoadingFunctionsQuantity;
   size_t *sizes = resourceLoadingFunctionsBufferSizes;
   cce_void *data = map->resourceData;
   for (cce_dataparsefun *fun = resourceCreatingFunctions, *end = resourceCreatingFunctions + resourceLoadingFunctionsQuantity;
        fun < end; ++fun, data += *sizes, ++sizes)
   {
      (*fun)(data, info);
   }
}

static void freeResourcesSection (void *buffer, struct cce_buffer *info)
{
   struct ResourceInfo *map = buffer;
   size_t *sizes = resourceLoadingFunctionsBufferSizes;
   cce_void *data = map->resourceData;
   for (cce_dataparsefun *fun = resourceUnloadingFunctions, *end = resourceUnloadingFunctions + map->resourcesQuantity;
        fun < end; ++fun, data += *sizes, ++sizes)
   {
      (*fun)(data, info);
   }
}

static uint8_t storeResourcesSection (void *buffer, struct cce_buffer *info, FILE *file)
{
   struct ResourceInfo *map = buffer;
   uint32_t resourceSizes[256];
   uint8_t sectionSize = map->resourcesQuantity;
   size_t *sizes = resourceLoadingFunctionsBufferSizes;
   size_t bytesWritten = 0, size;
   cce_void *data = (cce_void*) map->resourceData;
   cce_rstorefun *fun = resourceStoringFunctions;
   resourceSizes[0] = 1;
   long beginOffset = ftell(file);
   fseek(file, sectionSize * sizeof(uint32_t), SEEK_CUR);
   for (uint32_t *iterator = resourceSizes + 1, *end = iterator + sectionSize; iterator < end; ++iterator, ++fun, data += *sizes, ++sizes)
   {
      *iterator = 0;
      
      char **names = (*fun)(data, info);
      if (names == NULL)
      {
         *iterator = 0;
         continue;
      }
      for (char **it = names; *it != NULL; ++it)
      {
         size = strlen(*it);
         fwrite(it, sizeof(char), size + 1, file);
         *iterator = size;
         bytesWritten += size;
      }
      free(names);
   }
   long endOffset = ftell(file);
   fseek(file, beginOffset + sectionSize * sizeof(uint32_t), SEEK_SET);
   {
      uint32_t *iterator = resourceSizes + sectionSize;
      while (*iterator != 0)
      {
         --iterator;
      }
      if (iterator - resourceSizes != sectionSize)
      {
         cceMoveFileContent(file, (iterator - resourceSizes) - sectionSize, SEEK_CUR, bytesWritten);
         sectionSize = (iterator - resourceSizes);
      }
   }
   cceHostEndianToLittleEndianArrayInt32(resourceSizes + 1, 255);
   fwrite(resourceSizes + 1, sizeof(uint32_t), sectionSize, file);
   fseek(file, endOffset, SEEK_SET);
   return sectionSize;
}

void cce__initMap2DLoaders (void)
{
   if (mapPath == NULL)
      cceSetMap2Dpath("./maps"); // Default
   resourceSpaceToBeAllocated = 0;
   CCE_ALLOC_ARRAY(resourceLoadingFunctions);
   resourceUnloadingFunctions = malloc(resourceLoadingFunctionsAllocated * sizeof(cce_dataparsefun*));
   resourceCreatingFunctions  = malloc(resourceLoadingFunctionsAllocated * sizeof(cce_dataparsefun*));
   resourceStoringFunctions   = malloc(resourceLoadingFunctionsAllocated * sizeof(cce_rstorefun*));
   resourceLoadingFunctionsBufferSizes = malloc(resourceLoadingFunctionsAllocated * sizeof(size_t));
   cce__staticMapFunctionSet = cceGetFileIOfunctionSet();
   cceRegisterFileIOcallbacks(cce__staticMapFunctionSet,  loadResourcesSection, freeResourcesSection, NULL,                   NULL,                  sizeof(struct ResourceInfo));
   cceRegisterFileIOcallbacks(cce__staticMapFunctionSet,  loadElements,         freeElements,         NULL,                   NULL,                  sizeof(struct RenderingInfo));
   cceRegisterFileIOcallbacks(cce__staticMapFunctionSet,  loadColliders,        freeColliders,        NULL,                   NULL,                  sizeof(struct CollisionInfo));
   g_resourceLoadersOffset = cceGetFunctionBufferOffset(0, cce__staticMapFunctionSet);
   g_renderingInfoOffset   = cceGetFunctionBufferOffset(1, cce__staticMapFunctionSet);
   cce__dynamicMapFunctionSet  = cceGetFileIOfunctionSet();
   cceRegisterFileIOcallbacks(cce__dynamicMapFunctionSet, loadResourcesSection, freeResourcesSection, createResourcesSection, storeResourcesSection, sizeof(struct ResourceInfo));
   cceRegisterFileIOcallbacks(cce__dynamicMapFunctionSet, loadElementsDynamic,  freeElementsDynamic,  createElements,         storeElements,         sizeof(struct DynamicRenderingInfo));
   cceRegisterFileIOcallbacks(cce__dynamicMapFunctionSet, loadCollidersDynamic, freeCollidersDynamic, createColliders,        storeColliders,        sizeof(struct DynamicCollisionInfo));
}

void cce__terminateMap2DLoaders (void)
{
   free(resourceLoadingFunctions);
   resourceLoadingFunctionsQuantity = 0;
   free(resourceUnloadingFunctions);
   free(resourceCreatingFunctions);
   free(resourceStoringFunctions);
   free(resourceLoadingFunctionsBufferSizes);
   free(mapPath);
   mapPath = NULL;
   mapPathLength = 0;
}

struct cce_buffer* createFailMap (uint16_t functionSetID)
{
   struct cce_buffer *result = cceCreateBuffer(2, functionSetID);
   struct RenderingInfo *info = (struct RenderingInfo*)((cce_void*) result + g_renderingInfoOffset);
   struct Map2DElementPosition elements[42] = 
   {
      {{-8,  2}, 0, 0},
      {{-8,  1}, 0, 0},
      {{-8,  0}, 0, 0},
      {{-8, -1}, 0, 0},
      {{-8, -2}, 0, 0},
      {{-7,  2}, 0, 0},
      {{-7,  0}, 0, 0},
      {{-7, -2}, 0, 0},
      {{-5,  2}, 0, 0},
      {{-5,  1}, 0, 0},
      {{-5,  0}, 0, 0},
      {{-5, -1}, 0, 0},
      {{-5, -2}, 0, 0},
      {{-4, -2}, 0, 0},
      {{-2,  1}, 0, 0},
      {{-2,  0}, 0, 0},
      {{-2, -1}, 0, 0},
      {{-1,  2}, 0, 0},
      {{-1, -2}, 0, 0},
      {{ 0,  1}, 0, 0},
      {{ 0,  0}, 0, 0},
      {{ 0, -1}, 0, 0},
      {{ 2,  1}, 0, 0},
      {{ 2,  0}, 0, 0},
      {{ 2, -1}, 0, 0},
      {{ 2, -2}, 0, 0},
      {{ 3,  2}, 0, 0},
      {{ 3,  0}, 0, 0},
      {{ 4,  1}, 0, 0},
      {{ 4,  0}, 0, 0},
      {{ 4, -1}, 0, 0},
      {{ 4, -2}, 0, 0},
      {{ 6,  2}, 0, 0},
      {{ 6,  1}, 0, 0},
      {{ 6,  0}, 0, 0},
      {{ 6, -1}, 0, 0},
      {{ 6, -2}, 0, 0},
      {{ 7, -2}, 0, 0},
      {{ 7,  2}, 0, 0},
      {{ 8,  1}, 0, 0},
      {{ 8,  0}, 0, 0},
      {{ 8, -1}, 0, 0},
   }; // ELOAD (should be visible under any user settings)
   struct Map2DElementData data = {{1, 1}, {0, 0, 0, 0}, 0, 0};
   struct cce_texture2D textureInfo = {{0, 0}, {0, 0}, 0};
   struct Map2DElementPositionArray elementsArray;
   elementsArray.data = elements;
   elementsArray.dataQuantity = 42;
   elementsArray.dataAllocated = 42;
   info->data = cce__map2DElementsToRenderingBuffer(&elementsArray, 1, &textureInfo, 1, &data, 1);
   if (functionSetID == cce__dynamicMapFunctionSet)
   {
      struct DynamicRenderingInfo *dynamicinfo = (struct DynamicRenderingInfo*)((cce_void*) result + g_renderingInfoOffset);
      dynamicinfo->elements = malloc(1 * sizeof(struct Map2DElementPositionArray));
      dynamicinfo->elements->data = malloc(42 * sizeof(struct Map2DElementPosition));
      memcpy(dynamicinfo->elements->data, elements, 42 * sizeof(struct Map2DElementPosition));
      dynamicinfo->layersQuantity = 1;
      dynamicinfo->elements->dataQuantity = 42;
      dynamicinfo->elements->dataAllocated = 42;
      dynamicinfo->elementData = malloc(1 * sizeof(struct Map2DElementData));
      dynamicinfo->elementDataQuantity = 1;
      dynamicinfo->elementDataAllocated = 1;
      memcpy(dynamicinfo->elementData, &data, 1 * sizeof(struct Map2DElementData));
      dynamicinfo->textureInfo = malloc(1 * sizeof(struct cce_texture2D));
      dynamicinfo->textureInfoQuantity = 1;
      dynamicinfo->textureInfoAllocated = 1;
      memcpy(dynamicinfo->textureInfo, &textureInfo, 1 * sizeof(struct cce_texture2D));
   }
   return result;
}

#ifdef WINDOWS_SYSTEM
#define PATH_PLATFORM_SPECIFIC_CHECKS path[0] != '\\' && path[1] != ':'
#else
#define PATH_PLATFORM_SPECIFIC_CHECKS 1
#endif

#define CCE_EXPAND_PATH(path, function) \
if (path[0] != '/' && PATH_PLATFORM_SPECIFIC_CHECKS) \
{ \
   size_t len = strlen(path); \
   if (len > CCE_PATH_RESERVED) \
   { \
      char *newPath = malloc(len + mapPathLength + 1); \
      memcpy(newPath, mapPath, mapPathLength); \
      memcpy(newPath + mapPathLength, path, len + 1); \
      path = newPath; \
      function; \
      free(newPath); \
   } \
   else \
   { \
      memcpy(mapPath + mapPathLength, path, len + 1); \
      path = mapPath; \
      function; \
      mapPath[mapPathLength] = '\0'; \
   } \
} \
else \
   function

CCE_PUBLIC_OPTIONS struct cce_buffer* cceLoadMap2D(char *path)
{
   struct cce_buffer *result;
   CCE_EXPAND_PATH(path, result = cceLoadBinaryCCF(path, cce__staticMapFunctionSet));
   if (result == NULL && ((cce__map2Dflags & (CCE_RETURN_NULL_ON_MAP_LOADING_FAILURE | CCE_RETURN_FALLBACK_ON_MAP_LOADING_FAILURE)) == CCE_RETURN_FALLBACK_ON_MAP_LOADING_FAILURE))
   {
      result = createFailMap(cce__staticMapFunctionSet);
   }
   return result;
}

CCE_PUBLIC_OPTIONS struct cce_buffer* cceLoadMap2Ddynamic(char *path)
{
   struct cce_buffer *result;
   CCE_EXPAND_PATH(path, result = cceLoadBinaryCCF(path, cce__dynamicMapFunctionSet));
   if (result == NULL && ((cce__map2Dflags & (CCE_RETURN_NULL_ON_MAP_LOADING_FAILURE | CCE_RETURN_FALLBACK_ON_MAP_LOADING_FAILURE)) == CCE_RETURN_FALLBACK_ON_MAP_LOADING_FAILURE))
   {
      result = createFailMap(cce__dynamicMapFunctionSet);
   }
   return result;
}

CCE_PUBLIC_OPTIONS int cceWriteMap2Ddynamic (struct cce_buffer *map, char *path)
{
   int result;
   CCE_EXPAND_PATH(path, result = cceWriteBinaryCCF(map, path));
   return result;
}
