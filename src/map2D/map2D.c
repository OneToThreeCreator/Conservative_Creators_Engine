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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>


#include "../../include/coffeechain/engine_common.h"
#include "../../include/coffeechain/utils.h"
#include "../../include/coffeechain/endianess.h"
#include "../../include/coffeechain/os_interaction.h"

#include "../engine_common_internal.h"
#include "../external/stb_image.h"
#include "../../include/coffeechain/map2D/map2D.h"
#include "map2D_internal.h"

uint16_t                                     cce__loadedMap2Dnumber;
static char                                 *cce__resourcePath;
CCE_PUBLIC_OPTIONS const uint16_t     *const cceLoadedMap2Dnumber = &cce__loadedMap2Dnumber;
static uint8_t                               g_nearestMapsQuantity;
static uint8_t                               g_nearestMaps[16];
static struct cce_u32vec2                    g_textureSize;
CCE_PUBLIC_OPTIONS const struct cce_u32vec2 *cceTextureSize = &g_textureSize;
CCE_ARRAY(g_textures, static struct LoadedTextures, static uint16_t);

static void (*drawMap2Dcommon) (struct Map2Darray*);
static void (*processLogicMap2Dcommon) (struct Map2Darray*);
static void (*drawMap2D) (struct Map2D*);

static struct DynamicMap2D *g_dynamicMap;

static char *texturesPath = NULL;
static size_t texturesPathLength;

static cce_flag map2Dflags;

static void drawMap2Dmain (struct Map2Darray *maps)
{
   drawMap2D(maps->main);
}

static void drawMap2Dnearest (struct Map2Darray *maps)
{
   drawMap2D(maps->main);
   for (uint8_t *iterator = g_nearestMaps, *end = g_nearestMaps + g_nearestMapsQuantity; iterator < end; ++iterator)
   {
      struct ExitMap2D *exitMap = maps->main->exitMaps + *iterator;
      glUniform2i(*(uniformLocations + 2), exitMap->xOffset, exitMap->yOffset);
      drawMap2D(*(maps->dependies + *iterator));
   }
   glUniform2i(uniformLocations[2], 0, 0);
}

static void drawMap2Dall (struct Map2Darray *maps)
{
   drawMap2D(maps->main);
   struct ExitMap2D *exitMap = maps->main->exitMaps;
   for (struct Map2D **iterator = maps->dependies, **end = maps->dependies + maps->main->exitMapsQuantity; iterator < end; ++iterator, ++exitMap)
   {
      glUniform2i(*(uniformLocations + 2), exitMap->xOffset, exitMap->yOffset);
      drawMap2D(*iterator);
   }
   glUniform2i(*(uniformLocations + 2), 0, 0);
}

static cce_ubyte cce__fourthLogicTypeFuncDynamicMap2Dnearest (uint16_t ID, va_list argp)
{
   struct Map2D *map = va_arg(argp, struct Map2D*);
   struct Map2D **nearestMaps = va_arg(argp, struct Map2D**);
   size_t nearestMapsQuantity = va_arg(argp, size_t);
   struct cce_i32vec2 *offsets = va_arg(argp, struct cce_i32vec2*);
   return cce__checkCollisionDynamicMap2DmultipleMaps(ID, map, nearestMaps, nearestMapsQuantity, offsets, sizeof(struct cce_i32vec2));

}

static cce_ubyte cce__fourthLogicTypeFuncDynamicMap2Dall (uint16_t ID, va_list argp)
{
   struct Map2Darray *maps = va_arg(argp, struct Map2Darray*);
   return cce__checkCollisionDynamicMap2DmultipleMaps(ID, maps->main, maps->dependies, maps->main->exitMapsQuantity, (struct cce_i32vec2*) &(maps->main->exitMaps->xOffset), sizeof(struct ExitMap2D));
}

static void dontProcessLogicMap2D (struct Map2Darray *maps)
{
   CCE_UNUSED(maps);
   return;
}

static void processLogicMap2Dmain (struct Map2Darray *maps)
{
   struct Map2D *map = maps->main;
   if ((map)->logicQuantity) 
      cce__setCurrentTemporaryBools(map->temporaryBools);
   cce__beginBaseActions(map);
   cce__processLogic(map->logicQuantity, map->logic, map->timers, cce_actions, cce__fourthLogicTypeFuncMap2D, map);
   cce__setCurrentTemporaryBools(g_dynamicMap->temporaryBools);
   cce__processLogic(g_dynamicMap->logicQuantity, g_dynamicMap->logic, g_dynamicMap->timers, cce_actions, cce__fourthLogicTypeFuncDynamicMap2D, map);
   cce__endBaseActions();
   cce__endBaseActionsDynamicMap2D();
}

static void processLogicMap2Dnearest (struct Map2Darray *maps)
{
   cce__processLogicMap2D(maps->main);
   struct cce_i32vec2 offsets[16];
   struct Map2D *nearestMaps[16];
   {
      uint8_t *iterator = g_nearestMaps, *end = g_nearestMaps + g_nearestMapsQuantity;
      struct cce_i32vec2 *jiterator = offsets;
      struct Map2D **kiterator = nearestMaps;
      while (iterator < end)
      {
         *jiterator = (struct cce_i32vec2) {(maps->main->exitMaps + *iterator)->xOffset, (maps->main->exitMaps + *iterator)->yOffset};
         *kiterator = *(maps->dependies + *iterator);
         cce__processLogicMap2D(*kiterator);
          ++iterator, ++jiterator, ++kiterator;
      }
   }
   cce__processLogicDynamicMap2D(g_dynamicMap, maps->main, cce__fourthLogicTypeFuncDynamicMap2Dnearest, maps->main, nearestMaps, g_nearestMapsQuantity, offsets);
}

static void processLogicMap2Dall (struct Map2Darray *maps)
{
   cce__processLogicMap2D(maps->main);
   for (struct Map2D **iterator = maps->dependies, **end = maps->dependies + maps->main->exitMapsQuantity; iterator < end; ++iterator)
   {
      cce__processLogicMap2D((*iterator));
   }
   cce__processLogicDynamicMap2D(g_dynamicMap, maps->main, cce__fourthLogicTypeFuncDynamicMap2Dall, maps);
}

#define CCE_RENDER_MAP_FLAGS (CCE_RENDER_ONLY_CURRENT_MAP | CCE_RENDER_VISIBLE_MAPS | CCE_RENDER_ALL_LOADED_MAPS)
#define CCE_PROCESS_LOGIC_FLAGS (CCE_PROCESS_LOGIC_ONLY_FOR_CURRENT_MAP | CCE_DONT_PROCESS_LOGIC | \
CCE_PROCESS_LOGIC_FOR_VISIBLE_MAPS | CCE_PROCESS_LOGIC_FOR_ALL_MAPS)
#define CCE_AVAILABLE_FLAGS (CCE_RENDER_MAP_FLAGS | CCE_PROCESS_LOGIC_FLAGS | CCE_FORCE_INITIALIZE_MAP_ONLOAD)

static void cceSetFlags2D (cce_flag flags)
{
   map2Dflags &= ~CCE_AVAILABLE_FLAGS;
   flags &= CCE_AVAILABLE_FLAGS;
   switch (flags & CCE_RENDER_MAP_FLAGS)
   {
      case CCE_RENDER_ONLY_CURRENT_MAP:
      {
         drawMap2Dcommon = drawMap2Dmain;
         break;
      }
      case CCE_RENDER_VISIBLE_MAPS:
      {
         drawMap2Dcommon = drawMap2Dnearest;
         break;
      }
      case CCE_RENDER_ALL_LOADED_MAPS:
      {
         drawMap2Dcommon = drawMap2Dall;
         break;
      }
      default:
      {
         drawMap2Dcommon = drawMap2Dnearest;
         flags |= CCE_RENDER_VISIBLE_MAPS;
      }
   }
   
   switch (flags & CCE_PROCESS_LOGIC_FLAGS)
   {
      case CCE_DONT_PROCESS_LOGIC:
      {
         processLogicMap2Dcommon = dontProcessLogicMap2D;
         break;
      }
      case CCE_PROCESS_LOGIC_ONLY_FOR_CURRENT_MAP:
      {
         processLogicMap2Dcommon = processLogicMap2Dmain;
         break;
      }
      case CCE_PROCESS_LOGIC_FOR_VISIBLE_MAPS:
      {
         processLogicMap2Dcommon = processLogicMap2Dnearest;
         break;
      }
      case CCE_PROCESS_LOGIC_FOR_ALL_MAPS:
      {
         processLogicMap2Dcommon = processLogicMap2Dall;
         break;
      }
      default:
      {
         processLogicMap2Dcommon = processLogicMap2Dmain;
         flags |= CCE_PROCESS_LOGIC_ONLY_FOR_CURRENT_MAP;
      }
   }
   map2Dflags |= flags;
}

CCE_PUBLIC_OPTIONS int cceInitEngine2D (uint16_t globalBoolsQuantity, uint32_t textureMaxWidth, uint32_t textureMaxHeight,
                                        const char *windowLabel, const char *resourcePath, cce_flag flags)
{
   {
      char *path = getenv("CCE_RESOURCE_PATH");
      if (path != NULL && *path != '\0')
         resourcePath = path;
   }
   if (resourcePath == NULL || *resourcePath == '\0')
   {
      fputs("ENGINE::INIT::NO_RESOURCE_PATH:\nEngine could not load the game without knowing where it is", stderr);
      return -1;
   }
   size_t pathLength = strlen(resourcePath) + 1u;
   cce__resourcePath = malloc((pathLength + 11u) * sizeof(char));
   memcpy(cce__resourcePath, resourcePath, pathLength);
   
   map2Dflags = CCE_INIT;
   if (cce__initEngine(windowLabel, globalBoolsQuantity) != 0)
   {
      free(cce__resourcePath);
      return -1;
   }
   
      
   cce__initMap2DLoaders(&map2Dflags);
   cceAppendPath(cce__resourcePath, pathLength + 11, "maps");
   cceSetMap2Dpath(cce__resourcePath);
   *(cce__resourcePath + pathLength) = '\0';
   
   cce__globalOffset = (struct cce_i32vec2) {0, 0};
   g_textureSize.x = textureMaxWidth;
   g_textureSize.y = textureMaxHeight;
   CCE_ALLOC_ARRAY_ZEROED(g_textures);
   glTexturesArray = createTextureArray(CCE_ALLOCATION_STEP);
   glTexturesArraySize = CCE_ALLOCATION_STEP;
   stbi_set_flip_vertically_on_load(1);
   cceAppendPath(cce__resourcePath, pathLength + 11, "textures");
   cceSetTexturesPath(resourcePath);
   *(cce__resourcePath + pathLength) = '\0';
   g_dynamicMap = cce__initDynamicMap2D(g_EBO);
   cce__baseActionsInit(g_dynamicMap, g_UBOs, bufferUniformsOffsets, uniformLocations, shaderProgram, cce_setUniformBufferToDefault, &g_uniformBufferSize, &map2Dflags);
   cceSetFlags2D(flags);
   map2Dflags &= ~CCE_INIT;
   glUseProgram(shaderProgram);
   cceSetGridMultiplierMap2D(1.0f);
   return 0;
}

CCE_PUBLIC_OPTIONS const char* cceGetResourcePath (void)
{
   return cce__resourcePath;
}

CCE_PUBLIC_OPTIONS void cceSetTexturesPath (const char *path)
{
   free(texturesPath);
   texturesPath = cceCreateNewPathFromOldPath(path, "img_", 10u);
   texturesPathLength = strlen(texturesPath);
}

/*
void cce__setAttribPointerVAO (void)
{
   / Pointers /
   glVertexAttribIPointer(0, 2, GL_INT,             sizeof(struct Map2DElementVertices), (void*)(offsetof(struct Map2DElementVertices, vertexCoords)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(1, 2, GL_INT,             sizeof(struct Map2DElementVertices), (void*)(offsetof(struct Map2DElementVertices, position)));
   GL_CHECK_ERRORS;
   glVertexAttribPointer( 2, 2, GL_FLOAT, GL_FALSE, sizeof(struct Map2DElementVertices), (void*)(offsetof(struct Map2DElementVertices, textureCoords)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(3, 4, GL_UNSIGNED_BYTE,   sizeof(struct Map2DElementVertices), (void*)(offsetof(struct Map2DElementVertices, moveIDs)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(4, 4, GL_UNSIGNED_BYTE,   sizeof(struct Map2DElementVertices), (void*)(offsetof(struct Map2DElementVertices, extendIDs)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(5, 2, GL_UNSIGNED_BYTE,   sizeof(struct Map2DElementVertices), (void*)(offsetof(struct Map2DElementVertices, transformGroups)));
   GL_CHECK_ERRORS;
   glVertexAttribPointer( 6, 2, GL_FLOAT, GL_FALSE, sizeof(struct Map2DElementVertices), (void*)(offsetof(struct Map2DElementVertices, textureFragmentSize)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(7, 1, GL_UNSIGNED_SHORT,  sizeof(struct Map2DElementVertices), (void*)(offsetof(struct Map2DElementVertices, textureID)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(8, 4, GL_UNSIGNED_BYTE,   sizeof(struct Map2DElementVertices), (void*)(offsetof(struct Map2DElementVertices, textureOffsetIDs)));
   GL_CHECK_ERRORS;
   glVertexAttribIPointer(9, 4, GL_UNSIGNED_BYTE,   sizeof(struct Map2DElementVertices), (void*)(offsetof(struct Map2DElementVertices, colorIDs)));
   GL_CHECK_ERRORS;
   
   / I'm lazy /
   for (uint8_t i = 0u; i < 10; ++i)
   {
      glEnableVertexAttribArray(i);
      GL_CHECK_ERRORS;
   }
}
*/

// #define GET_SIGN_FROM_BIT(n, bit)(1-((((n)&(1<<(bit)))>>(bit))*2))

/*void cce__elementToMap2DElementVertices (struct Map2DElementVertices *buffer, int32_t x, int32_t y, uint16_t width, uint16_t height,
                                         uint8_t *moveGroups, uint8_t moveGroupsQuantity, uint8_t *extensionGroups, uint8_t extensionGroupsQuantity,
                                         uint8_t globalOffset, uint8_t rotationGroup, struct Texture *textureInfo, uint16_t textureID,
                                         uint8_t *textureOffsetGroups, uint8_t textureOffsetGroupsQuantity, uint8_t *colorGroups, uint8_t colorGroupsQuantity)
{
   uint8_t i = 0;
   float textureInfoX[2] ;//= {0, 1};
   struct cce_u16vec2 textureSize = g_textures[textureID - (textureID > 0)].size;
   textureInfoX[0] = ((float)(textureInfo->position.x + (g_textureSize.x - textureSize.x)))/g_textureSize.x;
   textureInfoX[1] = ((float)(textureInfo->position.x + textureInfo->size.x + (g_textureSize.x - textureSize.x)))/g_textureSize.x;
   float textureInfoY[2] ;//= {0, 1};
   textureInfoY[1] = ((float)(g_textureSize.y - textureInfo->position.y - (g_textureSize.y - textureSize.y)))/g_textureSize.y;
   textureInfoY[0] = ((float)(((int)g_textureSize.y) - ((int)textureInfo->position.y) - ((int)textureInfo->size.y) - ((int)(g_textureSize.y - textureSize.y))))/g_textureSize.y;
   struct cce_f32vec2 textureFragmentSize = {(float) textureInfo->size.x / g_textureSize.x, (float) textureInfo->size.y / g_textureSize.y};
   for (struct Map2DElementVertices *vend = buffer + 4; buffer < vend; ++buffer, ++i)
   {
      buffer->vertexCoords.x  = width  * -GET_SIGN_FROM_BIT(i, 1);
      buffer->vertexCoords.y  = height * -GET_SIGN_FROM_BIT(i, 0);
      buffer->textureCoords.x = textureInfoX[(i & 2) >> 1];
      buffer->textureCoords.y = textureInfoY[i & 1];
      buffer->position.x      = x;
      buffer->position.y      = y;
      buffer->transformGroups.rotateGroupID  = rotationGroup;
      buffer->transformGroups.isGlobalOffset = globalOffset;
      memcpy(buffer->moveIDs, moveGroups, CCE_MIN(moveGroupsQuantity, 4));
      memset(buffer->moveIDs + moveGroupsQuantity, 0, 4 - CCE_MIN(moveGroupsQuantity, 4));
      memcpy(buffer->extendIDs, extensionGroups, CCE_MIN(extensionGroupsQuantity, 4));
      memset(buffer->extendIDs + extensionGroupsQuantity, 0, 4 - CCE_MIN(extensionGroupsQuantity, 4));
      buffer->textureFragmentSize = textureFragmentSize;
      buffer->textureID           = textureID;
      memcpy(buffer->textureOffsetIDs, textureOffsetGroups, CCE_MIN(textureOffsetGroupsQuantity, 4));
      memset(buffer->textureOffsetIDs + textureOffsetGroupsQuantity, 0, 4 - CCE_MIN(textureOffsetGroupsQuantity, 4));
      memcpy(buffer->colorIDs, colorGroups, CCE_MIN(colorGroupsQuantity, 4));
      memset(buffer->colorIDs + colorGroupsQuantity, 0, 4 - CCE_MIN(colorGroupsQuantity, 4));
   }
}
*/

static int loadTexture (char *path, uint16_t position)
{
   unsigned int width, height;
   void *data;
   data = stbi_load(path, (int*) &width, (int*) &height, NULL, 4);
   if (!data)
   {
      fprintf(stderr, "ENGINE::TEXTURE::DECODING_ERROR:\n%s\n", stbi_failure_reason());
      return -1;
   }
   if (width > g_textureSize.x || height > g_textureSize.y)
   {
      fprintf(stderr, "ENGINE::TEXTURE::APPLYING_ERROR:\n%s is bigger than texture buffer allocated for it. Increase textureMaxWidth and textureMaxHeight to fix this error\n", path);
      return -1;
   }
   glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, g_textureSize.x - width, 0, position, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
   GL_CHECK_ERRORS;
   stbi_image_free(data);
   (g_textures + position)->size.x = width;
   (g_textures + position)->size.y = height;
   return 0;
}

static int setTextureAttributes (uint16_t ID)
{
   if (g_textures[ID].size.x != 0 && g_textures[ID].size.y != 0)
      return 0;
   int width = 0, height = 0, channels, result;
   cce__shortToString(texturesPath, g_textures[ID].ID, ".png");
   result = stbi_info(texturesPath, &width, &height, &channels);
   *(texturesPath + texturesPathLength) = '\0';
   g_textures[ID].size = (struct cce_u16vec2){width, height};
   return result;
}

void cce__updateTexturesArray (void)
{
   GLuint freeTexturesQuantityFromEnd = 0;
   for (struct LoadedTextures *iterator = g_textures + g_texturesQuantity - 1;
        (iterator >= g_textures) && (iterator->dependantMapsQuantity == 0u); --iterator, ++freeTexturesQuantityFromEnd);

   g_texturesQuantity -= freeTexturesQuantityFromEnd;
   CCE_FIT_ARRAY_TO_SIZE(g_textures);
   uint8_t arrayResized = 0u;
   GLuint textureArray;

   if (glTexturesArraySize != g_texturesQuantityAllocated)
   {
      textureArray = createTextureArray(g_texturesQuantityAllocated);
      glBindBuffer(GL_READ_BUFFER, glTexturesArray);
      GL_CHECK_ERRORS;
      arrayResized = 1u;
      glBindTexture(GL_TEXTURE_2D_ARRAY, textureArray);
      GL_CHECK_ERRORS;
   }
   else
   {
      textureArray = glTexturesArray;
   }
   for (struct LoadedTextures *iterator = g_textures, *end = g_textures + g_texturesQuantity; iterator < end; ++iterator)
   {
      if (iterator->dependantMapsQuantity > 0u)
      {
         if ((iterator->flags & CCE_LOADEDTEXTURES_TOBELOADED))
         {
            cce__shortToString(texturesPath, iterator->ID, ".png");
            if (loadTexture(texturesPath, iterator - g_textures) != 0)
            {
               memcpy((texturesPath + texturesPathLength), "dummy.png", 10u);
               if (loadTexture(texturesPath, iterator - g_textures) != 0)
               {
                  *(texturesPath + texturesPathLength) = '\0';
                  cce__shortToString(texturesPath, iterator->ID, ".png");
                  cce__criticalErrorPrint("ENGINE::TEXTURE::DUMMY::FAILED_TO_LOAD:\nFailed to load dummy texture requested because %s was not found.", texturesPath);
               }
            }
            *(texturesPath + texturesPathLength) = '\0';
            iterator->flags &= ~CCE_LOADEDTEXTURES_TOBELOADED;
         }
         else if (arrayResized)
         {
            glCopyTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, 0, 0, g_textureSize.x, g_textureSize.y);
            GL_CHECK_ERRORS;
         }
      }
      else 
      {
         iterator->ID = 0u;
      }
   }
   if (arrayResized)
   {
      glDeleteTextures(1, &glTexturesArray);
      GL_CHECK_ERRORS;
      glTexturesArray = textureArray;
   }
   return;
}

// Gets called before processing of map logic begins
void cce__initLogicMap2D (struct Map2D *map)
{
   map->temporaryBools = cce__getFreeTemporaryBools();
   cce__allocateUBObuffers(map->UBO_ID, map->moveGroupsQuantity, map->extensionGroupsQuantity);

   if (*g_endianess == CCE_BIG_ENDIAN)
   {
      cce__callActions(cce_endianSwapActions, map->staticActionsQuantity, map->staticActionIDs, map->staticActionArgOffsets, map->staticActionArgs);
   }

   cce__beginBaseActions(map);
   if (map->logicQuantity)
   {
      cce__setCurrentTemporaryBools(map->temporaryBools);
   }
   cce__callActions(cce_actions, map->staticActionsQuantity, map->staticActionIDs, map->staticActionArgOffsets, map->staticActionArgs);
   cce__endBaseActions();
}

struct UsedUBO* cce__getFreeUBOdata (uint16_t ID)
{
   if (ID >= g_UBOsQuantity)
      return NULL;

   return g_UBOs + ID;
}

uint16_t cce__loadTexture (uint32_t textureID)
{
   if (textureID == 0u)
      return 0u;
   map2Dflags |= CCE_PROCESS_TEXTURES;
   uint16_t current_g_texture = 0u;
   for (;;)
   {
      if (current_g_texture >= g_texturesQuantity)
      {
         current_g_texture = g_texturesQuantity;
         ++g_texturesQuantity;
         if (g_texturesQuantity > g_texturesQuantityAllocated)
         {
            CCE_REALLOC_ARRAY(g_textures, (g_texturesQuantityAllocated + CCE_ALLOCATION_STEP));
         }
         break;
      }
      if ((g_textures + current_g_texture)->ID == (textureID - 1u))
      {
         ++((g_textures + current_g_texture)->dependantMapsQuantity);
         return current_g_texture + 1u;
      }
      if ((g_textures + current_g_texture)->dependantMapsQuantity == 0u)
      {
         uint16_t i = current_g_texture;
         while (i < g_texturesQuantity)
         {
            if ((g_textures + i)->ID == (textureID - 1u))
            {
               ++((g_textures + i)->dependantMapsQuantity);
               return i + 1u;
            }
            ++i;
         }
         break;
      }
      ++current_g_texture;
   }
   (g_textures + current_g_texture)->ID = (textureID - 1u);
   (g_textures + current_g_texture)->flags = CCE_LOADEDTEXTURES_TOBELOADED;
   (g_textures + current_g_texture)->dependantMapsQuantity = 1u;
   setTextureAttributes(current_g_texture);
   return current_g_texture + 1u;
}

CCE_PUBLIC_OPTIONS uint16_t cceLoadTexture (char *path)
{
   uint16_t current_g_texture = 0u;
   for (;;)
   {
      if (current_g_texture >= g_texturesQuantity)
      {
         current_g_texture = g_texturesQuantity;
         ++g_texturesQuantity;
         if (g_texturesQuantity > g_texturesQuantityAllocated)
         {
            CCE_REALLOC_ARRAY(g_textures, (g_texturesQuantityAllocated + CCE_ALLOCATION_STEP));
         }
         break;
      }
      if ((g_textures + current_g_texture)->dependantMapsQuantity == 0u)
      {
         break;
      }
      ++current_g_texture;
   }
   if (loadTexture(path, current_g_texture) != 0)
   {
      --g_texturesQuantity;
      return 0;
   }
   (g_textures + current_g_texture)->ID = UINT32_MAX;
   (g_textures + current_g_texture)->dependantMapsQuantity = 1u;
   return current_g_texture + 1u;
}

uint16_t* cce__loadTexturesMap2D (struct Map2DElement *elements, uint32_t elementsQuantity, uint16_t *texturesLoadedMapReliesOnQuantity)
{
   map2Dflags |= CCE_PROCESS_TEXTURES;
   uint32_t *texturesMapReliesOn = NULL;
   uint16_t  texturesMapReliesOnQuantity = 0u, texturesMapReliesOnAllocated = 0u;
   cce_ubyte isLoaded = 0u;
   struct ElementsArray
   {
      struct Map2DElement **elements;
      uint32_t elementsQuantity;
      uint32_t elementsQuantityAllocated;
   } *elementsWithTexture = NULL;
   #define SET_TEXTUREID_FOR_ELEMENTS_AND_FREE(x, id) \
   for (struct Map2DElement **ITERATOR = (x)->elements, **END = (x)->elements + (x)->elementsQuantity; ITERATOR < END; ++ITERATOR) \
   { \
      (**ITERATOR).textureInfo.ID = (id); \
   } \
   free(x->elements)
   for (struct Map2DElement *iterator = elements, *end = elements + elementsQuantity; iterator < end; ++iterator)
   {
      uint32_t ID = iterator->textureInfo.ID;
      if (ID == 0u) continue;
      for (uint32_t *jiterator = texturesMapReliesOn, *jend = texturesMapReliesOn + texturesMapReliesOnQuantity; jiterator < jend; ++jiterator)
      {
         if ((*jiterator) == (ID - 1u))
         {
            isLoaded = 1u;
            struct ElementsArray *current = (elementsWithTexture + (jiterator - texturesMapReliesOn));
            if (current->elementsQuantity >= current->elementsQuantityAllocated)
            {
               CCE_REALLOC_ARRAY(current->elements, current->elementsQuantity + 1);
            }
            current->elements[current->elementsQuantity] = iterator;
            ++current->elementsQuantity;
            break;
         }
      }
      if (!isLoaded)
      {
         if (texturesMapReliesOnQuantity == texturesMapReliesOnAllocated)
         {
            texturesMapReliesOnAllocated += CCE_ALLOCATION_STEP;
            texturesMapReliesOn = realloc(texturesMapReliesOn, texturesMapReliesOnAllocated * sizeof(uint32_t));
            elementsWithTexture = realloc(elementsWithTexture, texturesMapReliesOnAllocated * sizeof(struct ElementsArray));
         }
         CCE_ALLOC_ARRAY(elementsWithTexture[texturesMapReliesOnQuantity].elements);
         elementsWithTexture[texturesMapReliesOnQuantity].elementsQuantity = 1;
         elementsWithTexture[texturesMapReliesOnQuantity].elements[0] = iterator;
         (*(texturesMapReliesOn + texturesMapReliesOnQuantity)) = (ID - 1u); // 0u is invalid for openGL shaders, but perfectly fine here
         ++texturesMapReliesOnQuantity;
      }
   }
   uint16_t *texturesLoadedMapReliesOn = (uint16_t*) texturesMapReliesOn;
   uint16_t *literator = texturesLoadedMapReliesOn;
   uint16_t *end = texturesLoadedMapReliesOn + texturesMapReliesOnQuantity;
   uint32_t *jiterator, *kiterator = texturesMapReliesOn;
   uint32_t tmp;
   uint16_t *freeLoadedTextures = NULL;
   uint16_t freeLoadedTexturesQuantity = 0u, freeLoadedTexturesAllocated = 0u;
   struct ElementsArray *elementsWithTextureIterator = elementsWithTexture;
   isLoaded = 0u;
   for (uint16_t current_g_texture = 0u; current_g_texture < g_texturesQuantity; ++current_g_texture)
   {
      jiterator = kiterator;
      if ((g_textures + current_g_texture)->ID)
      {
         for (uint16_t *iterator = literator; iterator < end; ++iterator, ++jiterator)
         {
            if ((*jiterator) == ((g_textures + current_g_texture)->ID))
            {
               tmp = (*jiterator);
               (*jiterator) = (*kiterator);
               (*kiterator) = tmp;
               (*literator) = current_g_texture + 1u;
               SET_TEXTUREID_FOR_ELEMENTS_AND_FREE(elementsWithTextureIterator, current_g_texture + 1u);
               ++((g_textures + current_g_texture)->dependantMapsQuantity);
               ++literator;
               ++kiterator;
               ++elementsWithTextureIterator;
               isLoaded = 1u;
               break;
            }
         }
         if ((!isLoaded) && (!(g_textures + current_g_texture)->dependantMapsQuantity) && ((end - literator) > freeLoadedTexturesQuantity))
         {
            if (freeLoadedTexturesQuantity == freeLoadedTexturesAllocated)
            {
               freeLoadedTexturesAllocated += CCE_ALLOCATION_STEP;
               freeLoadedTextures = (uint16_t *) realloc(freeLoadedTextures, freeLoadedTexturesAllocated * sizeof(uint16_t));
            }
            (*(freeLoadedTextures + freeLoadedTexturesQuantity)) = current_g_texture;
            ++freeLoadedTexturesQuantity;
         }
      }
      else if ((end - literator) > freeLoadedTexturesQuantity)
      {
         if (freeLoadedTexturesQuantity == freeLoadedTexturesAllocated)
         {
            freeLoadedTexturesAllocated += CCE_ALLOCATION_STEP;
            freeLoadedTextures = (uint16_t *) realloc(freeLoadedTextures, freeLoadedTexturesAllocated * sizeof(uint16_t));
         }
         (*(freeLoadedTextures + freeLoadedTexturesQuantity)) = current_g_texture;
         ++freeLoadedTexturesQuantity;
      }
   }
   uint16_t *iterator = freeLoadedTextures, *iend = freeLoadedTextures + freeLoadedTexturesQuantity;
   while ((literator < end) && (iterator < iend))
   {
      ((g_textures + (*iterator))->ID) = *kiterator;
      (*literator) = (*iterator) + 1u;
      SET_TEXTUREID_FOR_ELEMENTS_AND_FREE(elementsWithTextureIterator, (*iterator) + 1u);
      ((g_textures + (*iterator))->dependantMapsQuantity) = 1;
      ((g_textures + (*iterator))->flags) = CCE_LOADEDTEXTURES_TOBELOADED;
      setTextureAttributes(*iterator);
      ++literator, ++kiterator, ++iterator, ++elementsWithTextureIterator;
   }
   free(freeLoadedTextures);
   uint16_t current_g_texture = g_texturesQuantity;
   while (literator < end)
   {
      if (current_g_texture >= g_texturesQuantity)
      {
         ++g_texturesQuantity;
         if (g_texturesQuantity > g_texturesQuantityAllocated)
         {
            CCE_REALLOC_ARRAY(g_textures, g_texturesQuantityAllocated + CCE_ALLOCATION_STEP);
         }
      }
      ((g_textures + current_g_texture)->ID) = (*kiterator);
      (*literator) = current_g_texture + 1u; // 0u is invalid for openGL shaders (it's the way to say "We don't need texture there"), but perfectly fine here
      SET_TEXTUREID_FOR_ELEMENTS_AND_FREE(elementsWithTextureIterator, current_g_texture + 1u);
      ((g_textures + current_g_texture)->dependantMapsQuantity) = 1;
      ((g_textures + current_g_texture)->flags) = CCE_LOADEDTEXTURES_TOBELOADED;
      setTextureAttributes(current_g_texture);
      ++literator, ++kiterator, ++current_g_texture, ++elementsWithTextureIterator;
   }
   *texturesLoadedMapReliesOnQuantity = texturesMapReliesOnQuantity;
   free(elementsWithTexture);
   if (texturesMapReliesOnQuantity > 0)
      return (uint16_t*) realloc(texturesMapReliesOn, texturesMapReliesOnQuantity * sizeof(uint16_t));
   free(texturesMapReliesOn);
   return NULL;
}

void cce__releaseTextures (uint16_t *texturesMapReliesOn, uint16_t texturesMapReliesOnQuantity)
{
   for (uint16_t *iterator = texturesMapReliesOn, *end = texturesMapReliesOn + texturesMapReliesOnQuantity; iterator < end; ++iterator)
   {
      --((g_textures + (*(iterator) - 1u))->dependantMapsQuantity);
   }
   free(texturesMapReliesOn);
   return;
}

void cce__releaseTexture (uint16_t textureID)
{
   if (textureID == 0u)
      return;
   
   --((g_textures + (textureID - 1u))->dependantMapsQuantity);
   return;
}

cce_ubyte cce__checkCollision (const uint32_t *group1firstID, uint16_t groups1quantity, const uint32_t *group2firstID, uint16_t groups2quantity,
                               const cce_void *elements1, size_t element1size, const cce_void *elements2, size_t element2size)
{
   cce_ubyte isDifferent = (elements1 != elements2);
   const uint32_t *group1IDs = group1firstID, *group2IDs, *groups1end = group1firstID + groups1quantity, *groups2end = group2firstID + groups2quantity;
   while (group1IDs < groups1end)
   {
      group2IDs = group2firstID;
      while (group2IDs < groups2end)
      {
         if (((*group1IDs != *group2IDs) || isDifferent) &&
         cceCheckCollisionMap2D((struct Map2DCollider*) (elements1 + (*group1IDs * element1size)), (struct Map2DCollider*) (elements2 + (*group2IDs * element2size))))
         {
            return 1u;
         }
         ++group2IDs;
      }
      ++group1IDs;
   }
   return 0u;
}

cce_ubyte cce__checkCollisionWithOffset (const uint32_t *group1firstID, uint16_t groups1quantity, const uint32_t *group2firstID, uint16_t groups2quantity,
                                         const cce_void *elements1, size_t element1size, const struct cce_i32vec2 *elements1offset,
                                         const cce_void *elements2, size_t element2size, const struct cce_i32vec2 *elements2offset)
{
   cce_ubyte isDifferent = (elements1 != elements2);
   const uint32_t *group1IDs = group1firstID, *group2IDs, *groups1end = group1firstID + groups1quantity, *groups2end = group2firstID + groups2quantity;
   while (group1IDs < groups1end)
   {
      group2IDs = group2firstID;
      while (group2IDs < groups2end)
      {
         struct cce_i32vec2 offset = {elements2offset->x - elements1offset->x, elements2offset->y - elements1offset->y};
         if (((*group1IDs != *group2IDs) || isDifferent) &&
         cceCheckCollisionMap2DWithOffset((struct Map2DCollider*) (elements1 + (*group1IDs * element1size)), (struct Map2DCollider*) (elements2 + (*group2IDs * element2size)), &offset))
         {
            return 1u;
         }
         ++group2IDs;
      }
      ++group1IDs;
   }
   return 0u;
}

cce_ubyte cce__fourthLogicTypeFuncMap2D(uint16_t ID, va_list argp)
{
   struct Map2D *map = (struct Map2D*) va_arg(argp, struct Map2D*);
   return cce__checkCollision((map->collisionGroups + (map->collision + ID)->group1)->elements, (map->collisionGroups + (map->collision + ID)->group1)->elementsQuantity,
                              (map->collisionGroups + (map->collision + ID)->group2)->elements, (map->collisionGroups + (map->collision + ID)->group2)->elementsQuantity,
                              (cce_void*) map->colliders, sizeof(struct Map2DCollider), (cce_void*) map->colliders, sizeof(struct Map2DCollider));
}

static void swapMap2D (struct Map2D **a, struct Map2D **b)
{
   struct Map2D *tmp = (*a);
   (*a) = (*b);
   (*b) = tmp;
}

uint16_t g_mapToLoad;
struct cce_i32vec2 g_newOffset;

CCE_PUBLIC_OPTIONS void cceSetLoadedMap2D (uint16_t number, struct cce_i32vec2 globalPosition)
{
   g_mapToLoad = number;
   g_newOffset = globalPosition;
   map2Dflags |= CCE_PROCESS_LOADEDMAP2D;
}

static struct Map2Darray* loadMap2DwithDependies (struct Map2Darray *maps, uint16_t number)
{
   if (!maps)
   {
      maps = (struct Map2Darray*) calloc(1u, sizeof(struct Map2Darray));
   }
   if (maps->main)
   {
      uint8_t oldExitMapsQuantity = maps->main->exitMapsQuantity;
      if (maps->main->ID == number)
         return maps;
      if ((((map2Dflags & CCE_PROCESS_LOGIC_FLAGS) == CCE_PROCESS_LOGIC_FOR_VISIBLE_MAPS) ||
           ((map2Dflags & CCE_PROCESS_LOGIC_FLAGS) == CCE_DONT_PROCESS_LOGIC)) &&
           ((map2Dflags & CCE_FORCE_INITIALIZE_MAP_ONLOAD) == 0))
      {
         cce__releaseTemporaryBools(maps->main->temporaryBools);
         cce__releaseUBO(maps->main->UBO_ID);
      }
      if (maps->dependies)
      {
         for (struct Map2D **iterator = maps->dependies, **end = (maps->dependies + oldExitMapsQuantity - 1u);; ++iterator)
         {
            if (((*iterator)->ID) == number)
            {
               swapMap2D(&(maps->main), iterator);
               break;
            }
            if (iterator >= end)
            {
               cceFreeMap2D(maps->main);
               maps->main = cceLoadMap2D(number);
               break;
            }
         }
      }
      else
      {
         cceFreeMap2D(maps->main);
         maps->main = cceLoadMap2D(number);
      }
      if (!(maps->main->exitMapsQuantity))
      {
         if (maps->dependies)
         {
            for (struct Map2D **iterator = maps->dependies, **end = (maps->dependies + oldExitMapsQuantity - 1u); iterator <= end; ++iterator)
            {
               cceFreeMap2D((*iterator));
            }
            free(maps->dependies);
            maps->dependies = NULL;
         }
      }
      else
      {
         struct Map2D **dependies = (struct Map2D**) malloc(maps->main->exitMapsQuantity * sizeof(struct Map2D*));
         struct Map2D **j = dependies;
         for (struct ExitMap2D *i = maps->main->exitMaps, *iend = (maps->main->exitMaps + maps->main->exitMapsQuantity); i < iend; ++i, ++j)
         {
            for (struct Map2D **k = maps->dependies, **kend = (maps->dependies + oldExitMapsQuantity);; ++k)
            {
               if (k >= kend)
               {
                  (*j) = cceLoadMap2D(i->ID);
                  break;
               }
               if ((*k) == NULL) continue;
               if ((*k)->ID == i->ID)
               {
                  (*j) = (*k);
                  (*k) = NULL;
                  break;
               }
            }
         }
         for (struct Map2D **iterator = maps->dependies, **end = (maps->dependies + oldExitMapsQuantity); iterator < end; ++iterator)
         {
            cceFreeMap2D((*iterator));
         }
         free(maps->dependies);
         maps->dependies = dependies;
      }
   }
   else
   {
      maps->main = cceLoadMap2D(number);
      if (maps->dependies)
      {
         cce__errorPrint("ENGINE::MAP2DARRAY_LOADER::DEPENDENCY_OF_NOTHING:\nMaps->dependies initialized without maps->main. Cannot free maps->dependies. Possible memory leak", NULL);
      }
      maps->dependies = (struct Map2D**) malloc(maps->main->exitMapsQuantity * sizeof(struct Map2D*));
      struct ExitMap2D *exitmap = maps->main->exitMaps;
      for (struct Map2D **iterator = maps->dependies, **end = (maps->dependies + maps->main->exitMapsQuantity - 1u); iterator <= end; ++iterator, ++exitmap)
      {
         (*iterator) = cceLoadMap2D(exitmap->ID);
      }
   }
   cce__setCurrentArrayOfMaps(maps);
   if (((map2Dflags & (CCE_PROCESS_LOGIC_ONLY_FOR_CURRENT_MAP | CCE_DONT_PROCESS_LOGIC)) == (map2Dflags & CCE_PROCESS_LOGIC_FLAGS)) && 
       ((map2Dflags & CCE_FORCE_INITIALIZE_MAP_ONLOAD) == 0))
   {
      cce__initLogicMap2D(maps->main);
   }
   return maps;
}

static inline uint8_t isMapVisiblePlusExited (struct ExitMap2D *borderInfo)
{
   struct cce_u32vec2 step = cce__getCurrentStep();
   float stepA, stepB;
   int32_t globalOffsetA, nglobalOffsetB;
   if (borderInfo->flags & 0x1)
   {
      globalOffsetA = cce__globalOffset.x;
      nglobalOffsetB = -cce__globalOffset.y;
      stepA = step.x * g_stepMultiplier;
      stepB = step.y * g_stepMultiplier;
   }
   else
   {
      globalOffsetA = cce__globalOffset.y;
      nglobalOffsetB = -cce__globalOffset.x;
      stepA = step.y * g_stepMultiplier;
      stepB = step.x * g_stepMultiplier;
   }
   uint8_t flag2 = borderInfo->flags & 0x2;
   int aDistance = (-(borderInfo->aBorder + globalOffsetA) * (flag2 - 1)); // - (flag2 > 0));
   return ((((nglobalOffsetB + stepB) > borderInfo->b1Border) && ((nglobalOffsetB - stepB) < borderInfo->b2Border)) && (aDistance < stepA)) +
          ((nglobalOffsetB > borderInfo->b1Border && nglobalOffsetB < borderInfo->b2Border) && (aDistance < 0));
}

static void cce__processNearestMap2D (struct ExitMap2D *info, uint8_t quantity)
{
   uint8_t state;
   g_nearestMapsQuantity = 0;
   for (struct ExitMap2D *iterator = info, *end = info + quantity; iterator < end; ++iterator)
   {
      state = isMapVisiblePlusExited(iterator);
      if (state == 0)
         continue;
      
      g_nearestMaps[g_nearestMapsQuantity++] = iterator - info;
      
      if (state == 2)
         cceSetLoadedMap2D(iterator->ID, (struct cce_i32vec2) {cce__globalOffset.x + iterator->xOffset, cce__globalOffset.y + iterator->yOffset});
   }
}

static void cce__loadMapsAndSetState (struct Map2Darray *maps)
{
   struct cce_i32vec2 offset = (struct cce_i32vec2) {g_newOffset.x - cce__globalOffset.x, g_newOffset.y - cce__globalOffset.y};
   cce__globalOffset = (struct cce_i32vec2) {g_newOffset.x, g_newOffset.y};
   glUniform2iv(*(uniformLocations + CCE_GLOBALOFFSET_OFFSET), 1, (GLint*) &cce__globalOffset);
   uint32_t i = 0;
   while (i < g_dynamicMap->elementsQuantity)
   {
      uint8_t flags = cce__getDynamicElementFlags(i);
      (g_dynamicMap->elements + i)->position.x -= offset.x;
      (g_dynamicMap->elements + i)->position.y -= offset.y;
      if ((flags & 0x10) > 0)
         (g_dynamicMap->elements + i)->flags |= 0x4;
            
      ++i;
   }
   cce__setToBeProcessedDynamicMap2D();
   maps = loadMap2DwithDependies(maps, g_mapToLoad);
   cce__loadedMap2Dnumber = g_mapToLoad;
}

void cce__terminateEngine2D (void)
{
   cce__terminateDynamicMap2D();
   free(g_textures);
   glDeleteTextures(1, &glTexturesArray);
   for (struct UsedUBO *iterator = g_UBOs, *end = g_UBOs + g_UBOsQuantity; iterator < end; ++iterator)
   {
      glDeleteBuffers(1, &(iterator->UBO));
   }
   free(g_UBOs);
   glDeleteBuffers(1, &g_EBO);
   free(bufferUniformsOffsets);
   free(uniformLocations);
   free(texturesPath);
   glDeleteProgram(shaderProgram);
   free(cce__elementConversionBuffer);
   cce__terminateEngine();
}

CCE_PUBLIC_OPTIONS int cceEngine2D (void)
{
   if (map2Dflags & CCE_INIT)
      return -1;
      
   cce__showWindow();
   cce__engineUpdate();
   GL_CHECK_ERRORS;
   struct Map2Darray *maps = loadMap2DwithDependies(NULL, 0u);
   cce__loadedMap2Dnumber = 0;
   cce__setCurrentArrayOfMaps(maps);
   while (!(*cce__flags & CCE_ENGINE_STOP))
   {
      cce__processDynamicMap2DElements();
      
      if (map2Dflags & CCE_PROCESS_TEXTURES)
      {
         cce__updateTexturesArray();
         map2Dflags &= ~CCE_PROCESS_TEXTURES;
      }

      if (map2Dflags & CCE_PROCESS_UBO_ARRAY)
      {
         updateUBOarray();
         map2Dflags &= ~CCE_PROCESS_UBO_ARRAY;
      }
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
      GL_CHECK_ERRORS;
      glBindTexture(GL_TEXTURE_2D_ARRAY, glTexturesArray);
      GL_CHECK_ERRORS;
      if (maps->main->exitMapsQuantity > 0)
      {
         if (map2Dflags & CCE_PROCESS_NEAREST_MAPS)
         {
            cce__processNearestMap2D(maps->main->exitMaps, maps->main->exitMapsQuantity);
            map2Dflags &= ~CCE_PROCESS_NEAREST_MAPS;
         }
         drawMap2Dcommon(maps);
      }
      else
      {
         drawMap2D(maps->main);
      }
      
      glBindVertexArray(g_dynamicMap->VAO);
      GL_CHECK_ERRORS;
      glBindBufferRange(GL_UNIFORM_BUFFER, 1u, (g_UBOs + g_dynamicMap->UBO_ID)->UBO, 0u, g_uniformBufferSize);
      GL_CHECK_ERRORS;
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_EBO);
      GL_CHECK_ERRORS;
      glDrawElements(GL_TRIANGLES, g_dynamicMap->elementsQuantity * 6, GL_UNSIGNED_INT, (void*) 0);
      GL_CHECK_ERRORS;
      cce__swapBuffers();
      cce__engineUpdate();
      processLogicMap2Dcommon(maps);
#ifndef NDEBUG
      {
         static uint16_t frames = 0;
         static float timePassed = 0.0f;
         timePassed += *cceDeltaTime;
         ++frames;
         if (timePassed > 2.0f)
         {
            printf("%f FPS\n", frames / timePassed);
            frames = 0;
            timePassed = 0.0f;
         }
      }
#endif
      if (map2Dflags & CCE_PROCESS_LOADEDMAP2D)
      {
         cce__loadMapsAndSetState(maps);
         map2Dflags &= ~CCE_PROCESS_LOADEDMAP2D;
         map2Dflags |= CCE_PROCESS_NEAREST_MAPS;
      }
   }
   for (struct Map2D **iterator = maps->dependies, **end = maps->dependies + maps->main->exitMapsQuantity; iterator < end; ++iterator)
   {
      cceFreeMap2D(*iterator);
   }
   cceFreeMap2D(maps->main);
   free(maps->dependies);
   free(maps);
   cce__terminateEngine2D();
   return 0;
}
