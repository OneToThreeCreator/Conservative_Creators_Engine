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
#include <string.h>

#include "../../../include/cce/engine_common.h"
#include "../../../include/cce/engine_common_IO.h"
#include "../../../include/cce/utils.h"
#include "../../../include/cce/plugins/map2D/map2D.h"

#include "map2D_internal.h"

CCE_API struct cce_elementposition* cceGetElementsPosition (uint8_t layer, uint16_t positionID, uint16_t quantity, struct cce_buffer *map)
{
   if (quantity == 0)
      return NULL;
   assert(map != NULL);
   assert(map->loadingFunctionBlockID == cce__staticMapFunctionSet || map->loadingFunctionBlockID == cce__dynamicMapFunctionSet);
   struct cce_elementposition *elementPosition;
   if (map->loadingFunctionBlockID == cce__dynamicMapFunctionSet)
   {
      if (map->sectionsQuantity < 2)
         cceSetBufferSectionQuantity(map, 2);
      struct cce_dynamicrenderinginfo *renderingInfo = (struct cce_dynamicrenderinginfo*)((uint8_t*)map + cce__renderingInfoOffset);
      if (layer >= renderingInfo->layersQuantity)
      {
         renderingInfo->positions = realloc(renderingInfo->positions, (layer + 1) * sizeof(struct cce_elementpositionarray));
         memset(renderingInfo->positions + renderingInfo->layersQuantity, 0, (layer - renderingInfo->layersQuantity) * sizeof(struct cce_elementpositionarray));
         renderingInfo->layersQuantity = layer + 1;
         CCE_ALLOC_ARRAY_ZEROED(renderingInfo->positions[layer].data, positionID + quantity);
         renderingInfo->positions[layer].dataQuantity = positionID + quantity;
      }
      // Workaround is used to cram one extra bit into elementpositionarray struct (to indicate update request). Extra variable would increase struct size by 50% (+8 bytes) because of alignment rules. Should not happen too often
      else if (positionID >= ((renderingInfo->positions[layer].dataAllocated & ~0x1) | (renderingInfo->positions[layer].dataQuantity == 1 || renderingInfo->positions[layer].dataAllocated > 0x80000000)))
         CCE_REALLOC_ARRAY_ZEROED(renderingInfo->positions[layer].data, positionID + quantity);
      renderingInfo->positions[layer].dataQuantity = CCE_MAX(positionID + quantity, renderingInfo->positions[layer].dataQuantity);
      elementPosition = renderingInfo->positions[layer].data + positionID;
   }
   else
   {
      if (map->sectionsQuantity < 2)
         return NULL;
      if (layer >= ((struct cce_renderinginfo*)((uint8_t*)map + cce__renderingInfoOffset))->layersQuantity)
         return NULL;
      struct cce_elementpositionarray *positionArray = ((struct cce_renderinginfo*)((uint8_t*)map + cce__renderingInfoOffset))->positions + layer;
      if (positionID + quantity >= positionArray->dataQuantity)
         return NULL;
      elementPosition = positionArray->data + positionID;
   }
   return elementPosition;
}

CCE_API int cceSetElementsPositionsUpdated (struct cce_elementpositionarray *elementPositions)
{
   assert(elementPositions != NULL);
   elementPositions->dataAllocated ^= !(elementPositions->dataAllocated & 0x1) ^ (elementPositions->dataQuantity == 1 || elementPositions->dataAllocated > 0x80000000);
   return 0;
}

CCE_API struct cce_elementpositionarray* cceGetElementPositionArray (uint8_t layer, struct cce_buffer *map)
{
   assert(map != NULL);
   assert(map->loadingFunctionBlockID == cce__staticMapFunctionSet || map->loadingFunctionBlockID == cce__dynamicMapFunctionSet);
   if (map->loadingFunctionBlockID == cce__dynamicMapFunctionSet)
   {
      if (map->sectionsQuantity < 2)
         cceSetBufferSectionQuantity(map, 2);
      struct cce_dynamicrenderinginfo *renderingInfo = (struct cce_dynamicrenderinginfo*)((uint8_t*)map + cce__renderingInfoOffset);
      if (layer >= renderingInfo->layersQuantity)
      {
         renderingInfo->positions = realloc(renderingInfo->positions, (layer + 1) * sizeof(struct cce_elementpositionarray));
         memset(renderingInfo->positions + renderingInfo->layersQuantity, 0, (layer - renderingInfo->layersQuantity) * sizeof(struct cce_elementpositionarray));
         renderingInfo->layersQuantity = layer + 1;
         CCE_ALLOC_ARRAY_ZEROED(renderingInfo->positions[layer].data, 1);
         renderingInfo->positions[layer].dataQuantity = 0;
      }
      return renderingInfo->positions + layer;
   }
   else
   {
      if (map->sectionsQuantity < 2)
         return NULL;
      if (layer >= ((struct cce_renderinginfo*)((uint8_t*)map + cce__renderingInfoOffset))->layersQuantity)
         return NULL;
      return ((struct cce_renderinginfo*)((uint8_t*)map + cce__renderingInfoOffset))->positions + layer;
   }
}

CCE_API int cceSetRenderingLayersQuantity (uint8_t layersQuantity, struct cce_buffer *map)
{
   assert(map != NULL);
   if (map->loadingFunctionBlockID != cce__dynamicMapFunctionSet) // Can easily happen at runtime
      return -1;
   struct cce_renderinginfo *renderingData = (struct cce_renderinginfo*)((uint8_t*)map + cce__renderingInfoOffset);
   for (struct cce_elementpositionarray *it = renderingData->positions + layersQuantity, *end = renderingData->positions + renderingData->layersQuantity; it < end; ++it)
   {
      free(it->data);
   }
   renderingData->positions = realloc(renderingData->positions, layersQuantity * sizeof(struct cce_elementpositionarray));
   if (layersQuantity > renderingData->layersQuantity)
      memset(renderingData->positions + renderingData->layersQuantity, 0, layersQuantity - renderingData->layersQuantity);
   renderingData->layersQuantity = layersQuantity;
   return 0;
}

CCE_API struct cce_element* cceGetElements (uint16_t ID, uint16_t quantity, struct cce_buffer *map)
{
   if (quantity == 0)
      return NULL;
   assert(map != NULL);
   assert(map->loadingFunctionBlockID == cce__staticMapFunctionSet || map->loadingFunctionBlockID == cce__dynamicMapFunctionSet);
   struct cce_element *element;
   if (map->loadingFunctionBlockID == cce__dynamicMapFunctionSet)
   {
      if (map->sectionsQuantity < 2)
         cceSetBufferSectionQuantity(map, 2);
      struct cce_dynamicrenderinginfo *renderingInfo = (struct cce_dynamicrenderinginfo*)((uint8_t*)map + cce__renderingInfoOffset);
      if (ID >= renderingInfo->elementsAllocated)
         CCE_REALLOC_ARRAY_ZEROED(renderingInfo->elements, ID + quantity);
      renderingInfo->elementsQuantity = CCE_MAX(ID + quantity, renderingInfo->elementsQuantity);
      element = renderingInfo->elements + ID;
   }
   else
   {
      if (map->sectionsQuantity < 2)
         return NULL;
      if (ID + quantity > ((struct cce_renderinginfo*)((uint8_t*)map + cce__renderingInfoOffset))->elementsQuantity)
         return NULL;
      element = ((struct cce_renderinginfo*)((uint8_t*)map + cce__renderingInfoOffset))->elements + ID;
   }
   return element;
}

CCE_API int cceSetElementsUpdated (struct cce_renderinginfo *info)
{
   assert(info != NULL);
   info->flags |= CCE_ELEMENT_UPDATED;
   return 0;
}

CCE_API struct cce_renderinginfo* cceGetRenderingInfo (struct cce_buffer *map)
{
   assert(map != NULL);
   assert(map->loadingFunctionBlockID == cce__staticMapFunctionSet || map->loadingFunctionBlockID == cce__dynamicMapFunctionSet);
   return (struct cce_renderinginfo*)((uint8_t*)map + cce__renderingInfoOffset);
}

CCE_API struct cce_dynamicrenderinginfo* cceGetDynamicRenderingInfo (struct cce_buffer *map)
{
   assert(map != NULL);
   if (map->loadingFunctionBlockID != cce__dynamicMapFunctionSet) // Can easily happen at runtime
      return NULL;
   return (struct cce_dynamicrenderinginfo*)((uint8_t*)map + cce__renderingInfoOffset);
}


