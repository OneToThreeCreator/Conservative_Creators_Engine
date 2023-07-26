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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include "../../../include/cce/engine_common.h"
#include "../../../include/cce/engine_common_IO.h"
#include "../../../include/cce/os_interaction.h"
#include "../../../include/cce/endianess.h"
#include "../../../include/cce/utils.h"
#include "../../../include/cce/plugins/map2D/map2D.h"

// Integration
#include "../../../include/cce/plugins/actions.h"

#include "map2D_internal.h"

static char *mapPath = NULL;
static size_t mapPathLength = 0;
uint16_t cce__staticMapFunctionSet, cce__dynamicMapFunctionSet;
ptrdiff_t cce__resourceLoadersOffset, cce__renderingInfoOffset;

static size_t resourceSpaceToBeAllocated;

CCE_ARRAY(resourceLoadingFunctions, static cce_rloadfun, static uint16_t);
static cce_dataparsefun *resourceUnloadingFunctions;
static cce_dataparsefun *resourceCreatingFunctions;
static cce_rstorefun *resourceStoringFunctions;
static size_t *resourceLoadingFunctionsBufferSizes;

CCE_API void cceSetMap2Dpath (const char *path)
{
   CCE_SET_PATH(mapPath, mapPathLength, path);
}

CCE_API struct cce_buffer* cceCreateMap2Ddynamic (void)
{
   return cceCreateBuffer(-1, cce__dynamicMapFunctionSet);
}

CCE_API uint32_t cceRegisterMapCustomResourceCallback (cce_rloadfun onLoad, cce_dataparsefun onFree, cce_dataparsefun onCreate, cce_rstorefun onStore, size_t bufferSize)
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
   resourceLoadingFunctionsBufferSizes[resourceLoadingFunctionsQuantity + 1] = bufferSize + resourceLoadingFunctionsBufferSizes[resourceLoadingFunctionsQuantity];
   resourceSpaceToBeAllocated += bufferSize;
   return resourceLoadingFunctionsQuantity++ | 0x80000000;
}

#define LOADELEMENTS(buffer, sectionSize, info, file, elementInfoAlloc, elementsQuantityAlloc) \
fread(&elementInfoQuantity, sizeof(uint16_t), sectionSize > 0, file); \
elementInfoQuantity = cceLittleEndianToHostEndianInt16(elementInfoQuantity); \
fread(&elementsTotal,       sizeof(uint32_t), sectionSize > 0, file); \
elementsTotal = cceLittleEndianToHostEndianInt32(elementsTotal); \
struct cce_element          *elementInfo = elementInfoAlloc; \
elementsQuantity                         = elementsQuantityAlloc; \
fread(elementsQuantity,     sizeof(uint32_t), sectionSize,     file); \
fread(elementInfo, sizeof(struct cce_element), elementInfoQuantity, file); \
if (cceEndianess == CCE_BIG_ENDIAN) \
{ \
   cceLittleEndianToBigEndianArrayInt32(elementsQuantity, sectionSize); \
   for (struct cce_element *iterator = elementInfo, *end = elementInfo + elementInfoQuantity; iterator < end; ++iterator) \
   { \
      iterator->textureID = cceLittleEndianToBigEndianInt16(iterator->textureID); \
      cceLittleEndianToBigEndianArrayInt16((uint16_t*)iterator + (iterator->textureID == 0) * 2, 4 - (iterator->textureID == 0) * 2); \
   } \
} \
if (((struct cce_resourceinfo*)((cce_void*) info + cce__resourceLoadersOffset))->resourcesQuantity > 0) \
{ \
   uint16_t *usedTextures = ((struct cce_usedtexinfo*)(((struct cce_resourceinfo*)((cce_void*) info + cce__resourceLoadersOffset))->resourceData))->texturesMapDependsOn; \
   for (struct cce_element *iterator = elementInfo, *end = elementInfo + elementInfoQuantity; iterator < end; ++iterator) \
      if (iterator->textureID > 0) \
         iterator->textureID = usedTextures[iterator->textureID - 1]; \
}

static int loadElements (void *buffer, uint16_t sectionSize, struct cce_buffer *info, FILE *file)
{
   struct cce_renderinginfo *map = buffer;
   uint32_t *elementsQuantity;
   uint32_t elementsTotal;
   uint16_t elementInfoQuantity = 0;
   LOADELEMENTS(buffer, sectionSize, info, file, malloc(elementsTotal * sizeof(struct cce_elementposition) + sectionSize * sizeof(struct cce_elementpositionarray)
                + elementInfoQuantity * sizeof(struct cce_element)), (uint32_t*)(elementInfo + elementInfoQuantity))
   map->positions = (struct cce_elementpositionarray*)elementsQuantity;
   map->positions[sectionSize - 1].dataAllocated = map->positions[sectionSize - 1].dataQuantity = elementsQuantity[sectionSize - 1];
   map->positions[sectionSize - 1].data = (struct cce_elementposition*)(map->positions + sectionSize) + elementsTotal - elementsQuantity[sectionSize - 1];
   uint32_t *qiterator = elementsQuantity + sectionSize - 2;
   for (struct cce_elementpositionarray *iterator = map->positions + sectionSize - 1, *end = map->positions; iterator > end; --iterator, --qiterator)
   {
      iterator[-1].dataQuantity = *qiterator;
      iterator[-1].dataAllocated = *qiterator;
      iterator[-1].data = iterator->data - *qiterator;
   }
   fread(map->positions->data, sizeof(struct cce_elementposition), elementsTotal, file);
   map->elements = elementInfo;
   map->elementsQuantity = elementInfoQuantity;
   map->layersQuantity = sectionSize;
   map->data = cce__map2DElementsToRenderingBuffer(map->positions, sectionSize, elementInfo, elementInfoQuantity, elementInfoQuantity);
   return 0;
}

static int loadElementsDynamic (void *buffer, uint16_t sectionSize, struct cce_buffer *info, FILE *file)
{
   struct cce_dynamicrenderinginfo *map = buffer;
   uint32_t *elementsQuantity;
   uint32_t elementsTotal;
   uint16_t elementInfoQuantity = 0;
   LOADELEMENTS(buffer, sectionSize, info, file, malloc(CCE_CEIL_TO_POWER_OF_TWO(elementInfoQuantity, map->elementsAllocated) * sizeof(struct cce_element)),
                malloc(sectionSize * sizeof(struct cce_elementpositionarray)))
   map->positions = (struct cce_elementpositionarray*) elementsQuantity;
   uint32_t *qiterator = elementsQuantity + sectionSize;
   for (struct cce_elementpositionarray *iterator = map->positions + sectionSize, *end = map->positions; iterator > end;)
   {
      --iterator, --qiterator;
      iterator->dataQuantity = *qiterator;
      iterator->data = malloc(CCE_CEIL_TO_POWER_OF_TWO(*qiterator, iterator->dataAllocated) * sizeof(struct cce_elementposition));
      fread(iterator->data, sizeof(struct cce_elementposition), *qiterator, file);
   }
   map->elements = elementInfo;
   map->elementsQuantity  = elementInfoQuantity;
   map->layersQuantity = sectionSize;
   map->data = cce__map2DElementsToRenderingBuffer(map->positions, sectionSize, elementInfo, elementInfoQuantity, map->elementsAllocated);
   return 0;
}

static void createElements (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   struct cce_dynamicrenderinginfo *map = buffer;
   map->positions = NULL;
   map->elements = NULL;
   map->layersQuantity = 0;
   map->elementsQuantity = 0;
   map->elementsAllocated = 0;
   map->data = NULL;
   map->flags = 0;
}

static void freeElements (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   struct cce_renderinginfo *map = buffer;
   cce__deleteMap2DRenderingBuffer(map->data, map->layersQuantity);
   free(map->elements);
}

static void freeElementsDynamic (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   struct cce_dynamicrenderinginfo *map = buffer;
   cce__deleteMap2DRenderingBuffer(map->data, map->layersQuantity);
   for (struct cce_elementpositionarray *iterator = map->positions, *end = map->positions + map->layersQuantity; iterator < end; ++iterator)
   {
      free(iterator->data);
   }
   free(map->positions);
   free(map->elements);
}

static uint16_t storeElements (void *buffer, struct cce_buffer *info, FILE *file)
{
   struct cce_dynamicrenderinginfo *map = buffer;
   
   uint16_t sectionSize = map->layersQuantity;
   if (sectionSize == 0)
      return 0;
   {
      uint16_t tmp = cceHostEndianToLittleEndianInt16(map->elementsQuantity);
      fwrite(&tmp, sizeof(uint16_t), 1, file);
      fseek(file, sizeof(uint32_t), SEEK_CUR);
      uint32_t elementsTotalSize = 0;
      uint32_t tmp2;
      for (struct cce_elementpositionarray *iterator = map->positions, *end = map->positions + map->layersQuantity; iterator < end; ++iterator)
      {
         elementsTotalSize += iterator->dataQuantity;
         tmp2 = cceHostEndianToLittleEndianInt32(iterator->dataQuantity);
         fwrite(&tmp2, sizeof(uint32_t), 1, file);
      }
      fseek(file, -((long)((map->layersQuantity + 1) * sizeof(uint32_t))), SEEK_CUR);
      fwrite(&elementsTotalSize, sizeof(uint32_t), 1, file);
      fseek(file, (long)(map->layersQuantity * sizeof(uint32_t)), SEEK_CUR);
   }
   uint16_t *dependantTextures;
   uint16_t  dependantTexturesQuantity;
   if (((struct cce_resourceinfo*)((cce_void*) info + cce__resourceLoadersOffset))->resourcesQuantity == 0)
   {
      dependantTextures = NULL;
      dependantTexturesQuantity = 0;
   }
   else
   {
      dependantTextures =         ((struct cce_usedtexinfo*)(((struct cce_resourceinfo*)((cce_void*) info + cce__resourceLoadersOffset))->resourceData))->texturesMapDependsOn;
      dependantTexturesQuantity = ((struct cce_usedtexinfo*)(((struct cce_resourceinfo*)((cce_void*) info + cce__resourceLoadersOffset))->resourceData))->texturesMapDependsOnQuantity;
   }
   for (struct cce_element tmp, *iterator = map->elements, *end = map->elements + map->elementsQuantity; iterator < end; ++iterator)
   {
      cceHostEndianToLittleEndianNewArrayInt16(&tmp.position.x, &iterator->position.x, 2);
      tmp.rotation = iterator->rotation;
      tmp.flags = iterator->flags & ~CCE_ELEMENT_UPDATED;
      if (iterator->textureID == 0)
      {
         tmp.textureID = 0;
         tmp.data.rgba = iterator->data.rgba;
         cceHostEndianToLittleEndianNewArrayInt16(&tmp.size.x, &iterator->size.x, 2);
         goto TEXTUREID_SET;
      }
      for (uint16_t *jiterator = dependantTextures, *jend = dependantTextures + dependantTexturesQuantity; jiterator < jend; ++jiterator)
      {
         if (*jiterator == iterator->textureID)
         {
            tmp.textureID = cceHostEndianToLittleEndianInt16(dependantTextures - jiterator + 1);
            cceHostEndianToLittleEndianNewArrayInt16((uint16_t*)&tmp.data.texturePosition, (uint16_t*)&iterator->data.texturePosition, 4);
            goto TEXTUREID_SET;
         }
      }
      fprintf(stderr, "ENGINE::MAP2DLOADERS::RENDERING_DATA_STORE_ERROR:\nTextureID specified in element not found in DependantTextures array.\n");
      tmp.textureID = 0;
      tmp.data.rgba = (struct cce_u8vec4) {255, 255, 255, 255};
      cceHostEndianToLittleEndianNewArrayInt16(&tmp.size.x, &iterator->size.x, 2);
TEXTUREID_SET:
      fwrite(&tmp, sizeof(struct cce_element), 1, file);
   }
   for (struct cce_elementpositionarray *iterator = map->positions, *end = map->positions + map->layersQuantity; iterator < end; ++iterator)
   {
      if (cceEndianess == CCE_BIG_ENDIAN)
      {
         for (struct cce_elementposition tmp, *jiterator = iterator->data, *jend = iterator->data + iterator->dataQuantity; jiterator < jend ;++jiterator)
         {
            cceSwapEndianNewArrayIntN(&tmp, jiterator, 4, 2);
            fwrite(&tmp, sizeof(struct cce_elementposition), 1, file);
         }
      }
      else
      {
         fwrite(iterator->data, sizeof(struct cce_elementposition), iterator->dataQuantity, file);
      }
   }
   return sectionSize;
}

static int loadResourcesSection (void *buffer, uint16_t sectionSize, struct cce_buffer *info, FILE *file)
{
   struct cce_resourceinfo *map = buffer;
   uint32_t resourceSizes[256]; // VLA support across C compilers is still bad (and most likely wont improve)
   fread(resourceSizes + 1, sizeof(uint32_t), sectionSize, file);
   cceLittleEndianToHostEndianArrayInt32(resourceSizes + 1, sectionSize);
   resourceSizes[0] = 1;
   if (sectionSize > resourceLoadingFunctionsQuantity)
   {
      for (uint32_t *iterator = resourceSizes + 1 + resourceLoadingFunctionsQuantity, *end = resourceSizes + 1 + sectionSize; iterator < end; ++iterator)
      {
         if (*iterator != 0)
         {
            fprintf(stderr, "ENGINE::MAP2D_LOADING::NONEMPTY_RESOURCE_WITHOUT_LOADER:\nMap2D cannot be loaded because some resource required for map to function does not have corresponding loader\n");
            return -1;
         }
      }
   }
   uint32_t maxSize = 0;
   for (uint32_t *iterator = resourceSizes + 1, *end = iterator + sectionSize; iterator < end; ++iterator)
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
      while (*iterator == 0)
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
   for (uint32_t *iterator = resourceSizes + 1, *end = iterator + sectionSize; iterator < end; ++iterator, ++fun, ++initFun, jiterator = (cce_void*)map->resourceData + *bufferSizes++)
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
         if (namesQuantity >= namesAllocated)
         {
            CCE_REALLOC_ARRAY(names, (uint32_t)(namesQuantity + 1));
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
   struct cce_resourceinfo *map = buffer;
   map->resourceData = malloc(resourceSpaceToBeAllocated);
   map->resourcesQuantity = resourceLoadingFunctionsQuantity;
   size_t *sizes = resourceLoadingFunctionsBufferSizes;
   cce_void *data = map->resourceData;
   for (cce_dataparsefun *fun = resourceCreatingFunctions, *end = resourceCreatingFunctions + resourceLoadingFunctionsQuantity;
        fun < end; ++fun, data = (cce_void*)map->resourceData + *sizes++)
   {
      (*fun)(data, info);
   }
}

static void freeResourcesSection (void *buffer, struct cce_buffer *info)
{
   struct cce_resourceinfo *map = buffer;
   size_t *sizes = resourceLoadingFunctionsBufferSizes;
   cce_void *data = map->resourceData;
   for (cce_dataparsefun *fun = resourceUnloadingFunctions, *end = resourceUnloadingFunctions + map->resourcesQuantity;
        fun < end; ++fun, data = (cce_void*)map->resourceData + *sizes++)
   {
      (*fun)(data, info);
   }
   free(map->resourceData);
}

static uint16_t storeResourcesSection (void *buffer, struct cce_buffer *info, FILE *file)
{
   struct cce_resourceinfo *map = buffer;
   uint32_t resourceSizes[256] = {0};
   uint16_t sectionSize = map->resourcesQuantity;
   size_t *sizes = resourceLoadingFunctionsBufferSizes;
   size_t bytesWritten = 0, size;
   cce_void *data = (cce_void*) map->resourceData;
   cce_rstorefun *fun = resourceStoringFunctions;
   resourceSizes[0] = 1;
   long beginOffset = ftell(file);
   fseek(file, sectionSize * sizeof(uint32_t), SEEK_CUR);
   for (uint32_t *iterator = resourceSizes + 1, *end = iterator + sectionSize; iterator < end; ++iterator, ++fun, data = (cce_void*)map->resourceData + *sizes++)
   {
      char **names = (*fun)(data, info);
      if (names == NULL)
      {
         continue;
      }
      for (char **it = names; *it != NULL; ++it)
      {
         size = strlen(*it) + 1;
         fwrite(*it, sizeof(char), size, file);
         bytesWritten += size;
         *iterator += size;
      }
      free(names);
   }
   long endOffset = ftell(file);
   {
      uint32_t *iterator = resourceSizes + sectionSize;
      while (*iterator == 0)
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
   fseek(file, beginOffset, SEEK_SET);
   if (sectionSize > 0)
   {
      fwrite(resourceSizes + 1, sizeof(uint32_t), sectionSize, file);
      fseek(file, endOffset, SEEK_SET);
   }
   return sectionSize;
}

void cce__initMap2DLoaders (void)
{
   if (mapPath == NULL)
      cceSetMap2Dpath("./maps"); // Default
   resourceSpaceToBeAllocated = 0;
   CCE_ALLOC_ARRAY(resourceLoadingFunctions, 1);
   resourceUnloadingFunctions = malloc(resourceLoadingFunctionsAllocated * sizeof(cce_dataparsefun*));
   resourceCreatingFunctions  = malloc(resourceLoadingFunctionsAllocated * sizeof(cce_dataparsefun*));
   resourceStoringFunctions   = malloc(resourceLoadingFunctionsAllocated * sizeof(cce_rstorefun*));
   resourceLoadingFunctionsBufferSizes = malloc((resourceLoadingFunctionsAllocated + 1) * sizeof(size_t));
   resourceLoadingFunctionsBufferSizes[0] = 0;
   cce__staticMapFunctionSet = cceGetFileIOfunctionSet();
   cceRegisterFileIOcallbacks(cce__staticMapFunctionSet, cceNameToUID("m2Dres"),  loadResourcesSection, freeResourcesSection, NULL,                   NULL,                  sizeof(struct cce_resourceinfo));
   cceRegisterFileIOcallbacks(cce__staticMapFunctionSet, cceNameToUID("m2Drend"), loadElements,         freeElements,         NULL,                   NULL,                  sizeof(struct cce_renderinginfo));
   
   cce__resourceLoadersOffset = cceGetFunctionBufferOffset(cceNameToUID("m2Dres"), cce__staticMapFunctionSet);
   cce__renderingInfoOffset   = cceGetFunctionBufferOffset(cceNameToUID("m2Drend"), cce__staticMapFunctionSet);
   cce__dynamicMapFunctionSet = cceGetFileIOfunctionSet();
   cceRegisterFileIOcallbacks(cce__dynamicMapFunctionSet, cceNameToUID("m2Dres"),  loadResourcesSection, freeResourcesSection, createResourcesSection, storeResourcesSection, sizeof(struct cce_resourceinfo));
   cceRegisterFileIOcallbacks(cce__dynamicMapFunctionSet, cceNameToUID("m2Drend"), loadElementsDynamic,  freeElementsDynamic,  createElements,         storeElements,         sizeof(struct cce_dynamicrenderinginfo));
   if (cceIsPluginLoading(cceaPluginUID))
   {
      cceaRegisterActionsFileIOFunctions(cce__staticMapFunctionSet);
      cceaRegisterActionsFileIOFunctions(cce__dynamicMapFunctionSet);
   }
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
   createResourcesSection((cce_void*) result + cce__resourceLoadersOffset, result);
   struct cce_renderinginfo *info = (struct cce_renderinginfo*)((cce_void*) result + cce__renderingInfoOffset);
   struct cce_elementposition elements[21] = 
   {
      {{-8, -2}, 3, 0, 0},
      {{-8, -1}, 2, 0, 0},
      {{-8,  2}, 3, 0, 0},
      {{-7,  0}, 1, 0, 0},
      {{-5, -2}, 1, 0, 0},
      {{-5, -1}, 2, 0, 0},
      {{-5,  2}, 1, 0, 0},
      {{-3, -1}, 2, 0, 0},
      {{-2, -2}, 1, 0, 0},
      {{-2,  2}, 1, 0, 0},
      {{-1, -1}, 2, 0, 0},
      {{ 1, -1}, 2, 0, 0},
      {{ 1,  2}, 1, 0, 0},
      {{ 2, -2}, 1, 0, 0},
      {{ 2,  0}, 1, 0, 0},
      {{ 3, -1}, 2, 0, 0},
      {{ 3,  2}, 1, 0, 0},
      {{ 5,  2}, 3, 0, 0},
      {{ 5, -1}, 2, 0, 0},
      {{ 5, -2}, 3, 0, 0},
      {{ 7, -1}, 2, 0, 0},
   }; // ELOAD (should be visible under any game settings)
   
   struct cce_element elementInfo[3] = {{{0, 0}, {.rgba = {255, 255, 255, 255}}, {1, 1}, 0, 0, 0},
                                        {{0, 0}, {.rgba = {255, 255, 255, 255}}, {1, 3}, 0, 0, 0},
                                        {{0, 0}, {.rgba = {255, 255, 255, 255}}, {2, 1}, 0, 0, 0}}; 
   struct cce_elementpositionarray elementsArray = {elements, 21, 32};
   info->layersQuantity = 1;
   info->elementsQuantity = 3;
   if (functionSetID == cce__dynamicMapFunctionSet)
   {
      struct cce_dynamicrenderinginfo *dynamicinfo = (struct cce_dynamicrenderinginfo*)((cce_void*) result + cce__renderingInfoOffset);
      dynamicinfo->positions = malloc(dynamicinfo->layersQuantity * sizeof(struct cce_elementpositionarray));
      dynamicinfo->positions[0].data = malloc(elementsArray.dataAllocated * sizeof(struct cce_elementposition));
      dynamicinfo->elements = malloc(dynamicinfo->elementsQuantity * sizeof(struct cce_element));
      dynamicinfo->elementsAllocated = cceCeilToPowerOfTwoInt16(dynamicinfo->elementsQuantity);
      dynamicinfo->data = cce__map2DElementsToRenderingBuffer(&elementsArray, dynamicinfo->layersQuantity, elementInfo, dynamicinfo->elementsQuantity, dynamicinfo->elementsAllocated);
   }
   else
   {
      info->elements = malloc(elementsArray.dataQuantity * sizeof(struct cce_elementposition) + info->layersQuantity * sizeof(struct cce_elementpositionarray) +
                              info->elementsQuantity * sizeof(struct cce_element));
      info->positions = (struct cce_elementpositionarray*)(info->elements + info->elementsQuantity);
      info->positions[0].data = (struct cce_elementposition*)(info->positions + info->layersQuantity);
      elementsArray.dataAllocated = 0; // Workaround in map2D_modification.c
      info->data = cce__map2DElementsToRenderingBuffer(&elementsArray, info->layersQuantity, elementInfo, info->elementsQuantity, info->elementsQuantity);
   }
   info->positions[0].dataQuantity = elementsArray.dataQuantity;
   info->positions[0].dataAllocated = elementsArray.dataAllocated;
   memcpy(info->positions[0].data, elements, info->positions[0].dataQuantity * sizeof(struct cce_elementposition));
   memcpy(info->elements, &elementInfo, info->elementsQuantity * sizeof(struct cce_element));
   info->flags = 0;
   return result;
}

#define CCE_EXPAND_PATH(path, function) \
if (!cceIsPathAbsolute(path)) \
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

CCE_API struct cce_buffer* cceLoadMap2D (char *path)
{
   struct cce_buffer *result;
   CCE_EXPAND_PATH(path, result = cceLoadBinaryCCF(path, cce__staticMapFunctionSet));
   if (result == NULL && ((cce__map2Dflags & (CCE_RETURN_NULL_ON_MAP_LOADING_FAILURE | CCE_RETURN_FALLBACK_ON_MAP_LOADING_FAILURE)) == CCE_RETURN_FALLBACK_ON_MAP_LOADING_FAILURE))
   {
      result = createFailMap(cce__staticMapFunctionSet);
   }
   return result;
}

CCE_API struct cce_buffer* cceLoadMap2Ddynamic(char *path)
{
   struct cce_buffer *result;
   CCE_EXPAND_PATH(path, result = cceLoadBinaryCCF(path, cce__dynamicMapFunctionSet));
   if (result == NULL && ((cce__map2Dflags & (CCE_RETURN_NULL_ON_MAP_LOADING_FAILURE | CCE_RETURN_FALLBACK_ON_MAP_LOADING_FAILURE)) == CCE_RETURN_FALLBACK_ON_MAP_LOADING_FAILURE))
   {
      result = createFailMap(cce__dynamicMapFunctionSet);
   }
   return result;
}

CCE_API int cceWriteMap2Ddynamic (struct cce_buffer *map, char *path)
{
   int result;
   CCE_EXPAND_PATH(path, result = cceWriteBinaryCCF(map, path));
   return result;
}

CCE_API void* cceGetResource (uint8_t resource, struct cce_buffer *map)
{
   assert(map != NULL);
   assert(map->loadingFunctionBlockID == cce__staticMapFunctionSet || map->loadingFunctionBlockID == cce__dynamicMapFunctionSet);
   if (resource >= resourceLoadingFunctionsQuantity)
      return NULL;
   struct cce_resourceinfo *resources = (struct cce_resourceinfo*)((uint8_t*)map + cce__resourceLoadersOffset);
   if (resource >= resources->resourcesQuantity)
   {
      resources->resourceData = realloc(resources->resourceData, resource + 1);
      resources->resourcesQuantity = resource + 1;
   }
   return (cce_void*)resources->resourceData + resourceLoadingFunctionsBufferSizes[resource];
}
