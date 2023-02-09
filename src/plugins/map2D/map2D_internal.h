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

#ifndef MAP2D_INTERNAL_H
#define MAP2D_INTERNAL_H

#include "../../../include/cce/engine_common.h"
#include "../../../include/cce/utils.h"
#include "../../../include/cce/plugins/map2D/map2D.h"
#include "../../../include/cce/os_interaction.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define CCE_PROCESS_TEXTURES      0x010
#define CCE_PROCESS_NEAREST_MAPS  0x020

#define CCE_BASIC_ACTIONS_NOT_SET 0x100
#define CCE_INIT CCE_BASIC_ACTIONS_NOT_SET

#define CCE_STORE_COLOR_IN (CCE_STORE_COLOR_IN_RGB | CCE_STORE_COLOR_IN_HSV | CCE_STORE_COLOR_IN_HSL | CCE_STORE_COLOR_IN_HCL)

#ifndef CCE_PATH_RESERVED
#define CCE_PATH_RESERVED 16
#endif

#define CCE_LOADEDTEXTURES_TOBELOADED 0x1u

#define CCE_ELEMENT_UPDATED 0x80

struct cce_loadedtextures
{
   char              *path;
   struct cce_u16vec2 size;
   uint8_t            dependantMapsQuantity;
   uint8_t            flags;
};

struct cce_resourceinfo
{
   void *resourceData;
   uint8_t resourcesQuantity;
};

#define CCE_LAYER_STATIC  0x0
#define CCE_LAYER_DYNAMIC 0x1

struct cce_layer
{
   void    *layersData;
   uint16_t layer;
   uint8_t  flags;
};

struct cce_rendereringfuns
{
   void (*drawMap2D)(struct cce_layer *layers, uint32_t layersQuantity);
   struct cce_renderingdata* (*map2DElementsToRenderingBuffer)(const struct cce_elementpositionarray *elements, uint8_t layersQuantity,
                                                               const struct cce_element *elementInfo, uint16_t elementInfoQuantity, uint16_t elementsAllocated);
   struct cce_element* (*renderingBufferToMap2DElements) (struct cce_renderingdata *data);
   size_t (*getRenderingDataSize) (void);
   struct cce_renderingdata* (*createElementsBuffer)(size_t);
   struct cce_renderingdata* (*resizeElementsBuffer) (struct cce_renderingdata*, size_t);
   void (*deleteMap2DRenderingBuffer)(struct cce_renderingdata*, uint8_t);
   void (*loadTexture)(void*, uint16_t, uint16_t, uint16_t);
   void (*reallocateTextureArray)(uint16_t);
   void (*moveTextureFromOldArray)(uint16_t);
   void (*removeOldArray)(void);
   void (*terminateMap2DRenderer)(void);
};

extern struct cce_rendereringfuns            cce__renderingFunctions;
extern struct cce_loadedtextures            *cce__textures;
extern struct cce_i16vec2                    cce__cameraPosition;
extern cce_flag  cce__map2Dflags;
extern ptrdiff_t cce__resourceLoadersOffset, cce__renderingInfoOffset;
extern uint16_t  cce__staticMapFunctionSet,  cce__dynamicMapFunctionSet;
extern uint16_t  cce__texturesQuantity;
extern uint16_t  cce__pixelsPerCoordinate;
extern uint8_t   cce__viewRotationAngle;

#define cce__drawMap2D(maps, mapsQuantity) cce__renderingFunctions.drawMap2D(maps, mapsQuantity)
#define cce__map2DElementsToRenderingBuffer(layers, layersQuantity, elements, elementsQuantity, elementsAllocated) \
cce__renderingFunctions.map2DElementsToRenderingBuffer(layers, layersQuantity, elements, elementsQuantity, elementsAllocated)
#define cce__renderingBufferToMap2DElements(data) cce__renderingFunctions.renderingBufferToMap2DElements(data)
#define cce__getRenderingDataSize() cce__renderingFunctions.getRenderingDataSize()
#define cce__createElementsBuffer(size) cce__renderingFunctions.createElementsBuffer(size)
#define cce__resizeElementsBuffer(data, size) cce__renderingFunctions.resizeElementsBuffer(data, size)
#define cce__deleteMap2DRenderingBuffer(data, layersQuantity) cce__renderingFunctions.deleteMap2DRenderingBuffer(data, layersQuantity)
#define cce__loadTexture(data, width, height, textureID) cce__renderingFunctions.loadTexture(data, width, height, textureID)
#define cce__reallocateTextureArray(size) cce__renderingFunctions.reallocateTextureArray(size)
#define cce__moveTextureFromOldArray(texture) cce__renderingFunctions.moveTextureFromOldArray(texture)
#define cce__removeOldArray() cce__renderingFunctions.removeOldArray()
#define cce__terminateMap2DRenderer() cce__renderingFunctions.terminateMap2DRenderer()

#define CCE_SET_PATH(pathVar, lengthVar, newPath) \
newPath = cceGetAbsolutePath(newPath, CCE_PATH_RESERVED + 1); \
if (newPath == NULL) \
   return; \
free(pathVar); \
pathVar = (char*)newPath; \
lengthVar = strlen(pathVar); \
lengthVar += !cceIsPathDelimiter(pathVar[lengthVar]); \
pathVar[lengthVar - 1] = cceNativePathDelimiter; \
pathVar[lengthVar] = '\0'

void cce__initMap2DLoaders (void);
void cce__terminateMap2DLoaders (void);

void cce__setAttribPointerVAO (void);
void cce__extendElementBufferIfNecessary (uint32_t minimalSize);

void cce__releaseTexture (uint16_t textureID);
uint8_t cce__getDynamicElementFlags (uint16_t ID);
void cce__setToBeProcessedDynamicMap2D (void);

#ifdef __cplusplus
}
#endif // __cplusplus


#endif // MAP2D_INTERNAL_H
