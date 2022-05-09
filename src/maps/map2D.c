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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

#include "../engine_common.h"
#include "../engine_common_internal.h"
#include "../shader.h"
#include "../platform/endianess.h"
#include "../platform/path_getters.h"
#include "../external/stb_image.h"
#include "map2D.h"
#include "map2D_internal.h"

CCE_PUBLIC_OPTIONS uint16_t cceLoadedMap2Dnumber;

static uint32_t                   g_texturesWidth;
static uint32_t                   g_texturesHeight;
static struct LoadedTextures     *g_textures;
static uint16_t                   g_texturesQuantity;
static uint16_t                   g_texturesQuantityAllocated;
static GLuint                     glTexturesArray;
static uint16_t                   glTexturesArraySize;
static struct UsedUBO            *g_UBOs;
static uint16_t                   g_UBOsQuantity;
static uint16_t                   g_UBOsQuantityAllocated;
static GLuint                     g_cleanUBO;
static GLint                      g_uniformBufferSize;
static GLuint                     g_EBO;
static uint32_t                   g_elementBufferSize = 0;

static struct DynamicMap2D *g_dynamicMap;

static void (*cce_setUniformBufferToDefault)(GLuint, GLint);
static void (*drawMap2Ddependant)(struct Map2D*);
static GLint *bufferUniformsOffsets;
static GLint *uniformLocations;
static GLuint shaderProgram;

static char *texturesPath = NULL;
static size_t texturesPathLength;

static cce_flag map2Dflags;

/* CBO - clear buffer object. Requires function glClearBufferSubData to present in openGL */
static void setUniformBufferToDefault_withCBOext (GLuint UBO, GLint RotateAngleSinCosOffset)
{
   glBindBuffer(GL_UNIFORM_BUFFER, UBO);
   GL_CHECK_ERRORS;
   glClearBufferSubData(GL_UNIFORM_BUFFER, GL_R32F, 0u, g_uniformBufferSize, GL_RED, GL_FLOAT, NULL);
   GL_CHECK_ERRORS;
   struct cce_vec2 zeroOne = {0.0f, 1.0f};
   glClearBufferSubData(GL_UNIFORM_BUFFER, GL_RG32F, RotateAngleSinCosOffset, 255 * sizeof(struct cce_vec2) , GL_RG, GL_FLOAT, &zeroOne);
   GL_CHECK_ERRORS;
}

static void setUniformBufferToDefault_withoutCBOext (GLuint UBO, GLint RotateAngleSinCosOffset)
{
   glBindBuffer(GL_UNIFORM_BUFFER, UBO);
   GL_CHECK_ERRORS;
   void *uboData = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
   GL_CHECK_ERRORS;
   memset(uboData, 0, g_uniformBufferSize);
   struct cce_vec2 *sinCos = (struct cce_vec2*) (((cce_void*) uboData) + RotateAngleSinCosOffset);
   for (struct cce_vec2 *end = sinCos + 255; sinCos < end; ++sinCos)
   {
      sinCos->y = 1.0f;
   }
   glUnmapBuffer(GL_UNIFORM_BUFFER);
   GL_CHECK_ERRORS;
}

static void drawMap2D (struct Map2D *map)
{
   glBindVertexArray(map->VAO);
   GL_CHECK_ERRORS;
   glBindBufferRange(GL_UNIFORM_BUFFER, 1u, (g_UBOs + map->UBO_ID)->UBO, 0u, g_uniformBufferSize);
   GL_CHECK_ERRORS;
   glDrawElements(GL_TRIANGLES, map->elementsQuantity * 6, GL_UNSIGNED_INT, (void*) 0);
   GL_CHECK_ERRORS;
}

static void drawMap2DcleanUBO (struct Map2D *map)
{
   glBindVertexArray(map->VAO);
   GL_CHECK_ERRORS;
   glBindBufferRange(GL_UNIFORM_BUFFER, 1u, g_cleanUBO, 0u, g_uniformBufferSize);
   GL_CHECK_ERRORS;
   glDrawElements(GL_TRIANGLES, map->elementsQuantity * 6, GL_UNSIGNED_INT, (void*) 0);
   GL_CHECK_ERRORS;
}

static void drawMap2Dmain (struct Map2Darray *maps)
{
   drawMap2D(maps->main);
}

static uint8_t lastNearestMap2D;

static void drawMap2Dnearest (struct Map2Darray *maps)
{
   drawMap2D(maps->main);
   struct ExitMap2D *exitMap = maps->main->exitMaps + lastNearestMap2D;
   glUniform2i(*(uniformLocations + 2), exitMap->xOffset, exitMap->yOffset);
   drawMap2Ddependant(*(maps->dependies + lastNearestMap2D));
   glUniform2i(*(uniformLocations + 2), 0, 0);
}

static void drawMap2Dall (struct Map2Darray *maps)
{
   drawMap2D(maps->main);
   struct ExitMap2D *exitMap = maps->main->exitMaps;
   for (struct Map2D **iterator = maps->dependies, **end = maps->dependies + maps->main->exitMapsQuantity; iterator < end; ++iterator, ++exitMap)
   {
      glUniform2i(*(uniformLocations + 2), exitMap->xOffset, exitMap->yOffset);
      drawMap2Ddependant(*iterator);
   }
   glUniform2i(*(uniformLocations + 2), 0, 0);
}

static cce_ubyte cce__fourthLogicTypeFuncDynamicMap2Dnearest (uint16_t ID, va_list argp)
{
   struct Map2D *map = va_arg(argp, struct Map2D*);
   struct Map2D **nearestMaps = va_arg(argp, struct Map2D**);
   size_t nearestMapsQuantity = va_arg(argp, size_t);
   struct cce_ivec2 *offsets = va_arg(argp, struct cce_ivec2*);
   return cce__checkCollisionDynamicMap2DmultipleMaps(ID, map, nearestMaps, nearestMapsQuantity, offsets, sizeof(struct cce_ivec2));

}

static cce_ubyte cce__fourthLogicTypeFuncDynamicMap2Dall (uint16_t ID, va_list argp)
{
   struct Map2Darray *maps = va_arg(argp, struct Map2Darray*);
   return cce__checkCollisionDynamicMap2DmultipleMaps(ID, maps->main, maps->dependies, maps->main->exitMapsQuantity, (struct cce_ivec2*) &(maps->main->exitMaps->xOffset), sizeof(struct ExitMap2D));
}

static void dontProcessLogicMap2D (struct Map2Darray *maps)
{
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
   cce__processLogicMap2D(*(maps->dependies + lastNearestMap2D));
   struct cce_ivec2 offset = {(maps->main->exitMaps + lastNearestMap2D)->xOffset, (maps->main->exitMaps + lastNearestMap2D)->yOffset};
   cce__processLogicDynamicMap2D(g_dynamicMap, maps->main, cce__fourthLogicTypeFuncDynamicMap2Dnearest, maps->main, maps->dependies + lastNearestMap2D, 1, &offset);
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

static void (*drawMap2Dcommon) (struct Map2Darray*);
static void (*processLogicMap2Dcommon) (struct Map2Darray*);

#define CCE_RENDER_MAP_FLAGS (CCE_RENDER_ONLY_CURRENT_MAP | CCE_RENDER_CLOSEST_MAP | CCE_RENDER_ALL_LOADED_MAPS)
#define CCE_PROCESS_LOGIC_FLAGS (CCE_PROCESS_LOGIC_ONLY_FOR_CURRENT_MAP | CCE_DONT_PROCESS_LOGIC | \
CCE_PROCESS_LOGIC_FOR_CLOSEST_MAP | CCE_PROCESS_LOGIC_FOR_ALL_MAPS)
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
      case CCE_RENDER_CLOSEST_MAP:
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
         flags |= CCE_RENDER_CLOSEST_MAP;
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
      case CCE_PROCESS_LOGIC_FOR_CLOSEST_MAP:
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
   if ((((flags & CCE_PROCESS_LOGIC_FLAGS) == CCE_PROCESS_LOGIC_ONLY_FOR_CURRENT_MAP) ||
        ((flags & CCE_PROCESS_LOGIC_FLAGS) == CCE_DONT_PROCESS_LOGIC)) &&
        ((flags & CCE_FORCE_INITIALIZE_MAP_ONLOAD) == 0))
   {
      glGenBuffers(1, &g_cleanUBO);
      glBindBuffer(GL_UNIFORM_BUFFER, g_cleanUBO);
      GL_CHECK_ERRORS;
      glBufferData(GL_UNIFORM_BUFFER, g_uniformBufferSize, NULL, GL_DYNAMIC_DRAW);
      GL_CHECK_ERRORS;
      cce_setUniformBufferToDefault(g_cleanUBO, *(bufferUniformsOffsets + CCE_ROTATEANGLESINCOS_OFFSET));
      drawMap2Ddependant = drawMap2DcleanUBO;
   }
   else
   {
      drawMap2Ddependant = drawMap2D;
   }
   map2Dflags |= flags;
}

static GLuint createTextureArray (uint16_t newSize)
{
   GLuint texture;
   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
   glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA, g_texturesWidth, g_texturesHeight, newSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   GL_CHECK_ERRORS;   
   
   glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   
   return texture;
}

CCE_PUBLIC_OPTIONS void cceSetGridMultiplier (float multiplier)
{
   struct cce_uvec2 aspectRatio = cce__getCurrentStep();
   glUniform2f(*uniformLocations, 1.0f / (aspectRatio.x * multiplier), 1.0f / (aspectRatio.y * multiplier));
}

CCE_PUBLIC_OPTIONS int cceInitEngine2D (uint16_t globalBoolsQuantity, uint32_t textureMaxWidth, uint32_t textureMaxHeight,
                                        const char *windowLabel, const char *resourcePath, cce_flag flags)
{
   if (!resourcePath)
   {
      resourcePath = getenv("CCE_RESOURCE_PATH");
      if (!resourcePath || *resourcePath == '\0')
      {
         fputs("ENGINE::INIT::NO_RESOURCE_PATH:\nEngine could not load the game without knowing where it is", stderr);
         return -1;
      }
   }
   size_t pathLength = strlen(resourcePath) + 1u;
   char *pathBuffer = malloc((pathLength + 11u) * sizeof(char));
   memcpy(pathBuffer, resourcePath, pathLength);
   *(pathBuffer + pathLength) = '\0';
   
   map2Dflags = CCE_INIT;
   if (cce__initEngine(windowLabel, globalBoolsQuantity) != 0)
   {
      free(pathBuffer);
      return -1;
   }
   
   {
      #ifdef SYSTEM_SHADER_PATH
      shaderProgram = makeVFshaderProgram(SYSTEM_SHADER_PATH "/vertex_shader.glsl", SYSTEM_SHADER_PATH "/fragment_shader.glsl", 330u, "", "");
      if (shaderProgram == 0u)
      #endif // SYSTEM_SHADER_PATH
      {
         cceAppendPath(pathBuffer, pathLength + 11u, "shaders");
         char *vertexPath   = cce__createNewPathFromOldPath(pathBuffer, "vertex_shader.glsl",   0u);
         char *fragmentPath = cce__createNewPathFromOldPath(pathBuffer, "fragment_shader.glsl", 0u);
         shaderProgram = makeVFshaderProgram(vertexPath, fragmentPath, 330u, "", "");
         free(vertexPath);
         free(fragmentPath);
         *(pathBuffer + pathLength) = '\0';
      }
   }
   if (!shaderProgram)
   {
      fputs("ENGINE::INIT::SHADERS_CANNOT_BE_LOADED", stderr);
      cce__terminateEngine();
      free(pathBuffer);
      return -1;
   }
   
   uniformLocations = malloc(3 * sizeof(GLint));
   *uniformLocations = glGetUniformLocation(shaderProgram, "InverseStep");
   GL_CHECK_ERRORS;
   *(uniformLocations + 1) = glGetUniformLocation(shaderProgram, "GlobalMoveCoords");
   GL_CHECK_ERRORS;
   *(uniformLocations + 2) = glGetUniformLocation(shaderProgram, "MapOffset");
   GL_CHECK_ERRORS;
   {
      const GLchar *uniformNames[] = {"Colors", "MoveCoords", "Extension", "TextureOffset", "RotationOffset", "RotateAngleSinCos"};
      GLuint indices[6];
      glGetUniformIndices(shaderProgram, 6, uniformNames, indices);
      GL_CHECK_ERRORS;
      bufferUniformsOffsets = (GLint*) malloc(6 * sizeof(GLint));
      glGetActiveUniformsiv(shaderProgram, 6, indices, GL_UNIFORM_OFFSET, bufferUniformsOffsets);
      GL_CHECK_ERRORS;
      glUniformBlockBinding(shaderProgram, glGetUniformBlockIndex(shaderProgram, "Variables"), 1u);
      GL_CHECK_ERRORS;
   }
   g_UBOsQuantityAllocated = CCE_ALLOCATION_STEP;
   g_UBOs = (struct UsedUBO*) malloc(g_UBOsQuantityAllocated * sizeof(struct UsedUBO));
   {
      GLint maxUniformOffset = 0;
      uint8_t i = 0, maxI;
      for (GLint *iterator = bufferUniformsOffsets, *end = bufferUniformsOffsets + 6; iterator < end; ++iterator, ++i)
      {
         if (maxUniformOffset < (*iterator))
         {
            maxUniformOffset = (*iterator);
            maxI = i;
         }
      }
      switch (maxI)
      {
         case 0:
         {
            g_uniformBufferSize = maxUniformOffset + (4/*GLint and GLfloat*/ * 4/*vec4*/ * 255/*array*/);
            break;
         }
         case 1:
         case 2:
         case 3:
         case 4:
         case 5:
         {
            g_uniformBufferSize = maxUniformOffset + (4/*GLint and GLfloat*/ * 2/*vec2*/ * 255/*array*/);
            break;
         }
      }
   }
   if (GLAD_GL_ARB_clear_buffer_object)
   {
      cce_setUniformBufferToDefault = setUniformBufferToDefault_withCBOext;
   }
   else
   {
      cce_setUniformBufferToDefault = setUniformBufferToDefault_withoutCBOext;
   }
   
   for (struct UsedUBO *iterator = g_UBOs, *end = g_UBOs + g_UBOsQuantityAllocated; iterator < end; ++iterator)
   {
      glGenBuffers(1u, &(iterator->UBO));
      GL_CHECK_ERRORS;
      glBindBuffer(GL_UNIFORM_BUFFER, iterator->UBO);
      GL_CHECK_ERRORS;
      glBufferData(GL_UNIFORM_BUFFER, (g_uniformBufferSize), NULL, GL_DYNAMIC_DRAW);
      GL_CHECK_ERRORS;
      iterator->flags = 0u;
      cce_setUniformBufferToDefault(iterator->UBO, *(bufferUniformsOffsets + CCE_ROTATEANGLESINCOS_OFFSET));
      iterator->moveGroupValues = NULL;
      iterator->extensionGroupValues = NULL;
   }
   glEnable(GL_BLEND);
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   glGenBuffers(1, &g_EBO);
   cce__initMap2DLoaders(&g_EBO, &map2Dflags);
   cceAppendPath(pathBuffer, pathLength + 11, "maps");
   cceSetMap2Dpath(pathBuffer);
   *(pathBuffer + pathLength) = '\0';
   
   g_texturesWidth = textureMaxWidth;
   g_texturesHeight = textureMaxHeight;
   g_textures = (struct LoadedTextures*) calloc(CCE_ALLOCATION_STEP, sizeof(struct LoadedTextures));
   g_texturesQuantity = 0u;
   g_texturesQuantityAllocated = CCE_ALLOCATION_STEP;
   glTexturesArray = createTextureArray(CCE_ALLOCATION_STEP);
   glTexturesArraySize = CCE_ALLOCATION_STEP;
   g_elementBufferSize = 0;
   stbi_set_flip_vertically_on_load(1);
   cceAppendPath(pathBuffer, pathLength + 11, "textures");
   cceSetTexturesPath(resourcePath);
   *(pathBuffer + pathLength) = '\0';
   g_dynamicMap = cce__initDynamicMap2D(g_EBO);
   cce__baseActionsInit(g_dynamicMap, g_UBOs, bufferUniformsOffsets, uniformLocations, shaderProgram, cce_setUniformBufferToDefault, &g_uniformBufferSize, &map2Dflags);
   cceSetFlags2D(flags);
   map2Dflags &= ~CCE_INIT;
   free(pathBuffer);
   glUseProgram(shaderProgram);
   cceSetGridMultiplier(1.0f);
   return 0;
}

CCE_PUBLIC_OPTIONS void cceSetTexturesPath (const char *path)
{
   free(texturesPath);
   texturesPath = cce__createNewPathFromOldPath(path, "img_", 10u);
   texturesPathLength = strlen(texturesPath);
}

// Messes with gl state!
void cce__setAttribPointerVAO (void)
{
   /* Pointers */
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
   
   /* I'm lazy */
   for (uint8_t i = 0u; i < 10; ++i)
   {
      glEnableVertexAttribArray(i);
      GL_CHECK_ERRORS;
   }
}

void cce__extendElementBufferIfNecessary (uint32_t minimalSize)
{
   if (g_elementBufferSize >= minimalSize)
      return;

   g_elementBufferSize = (minimalSize & ~(CCE_ALLOCATION_STEP - 1u)) + CCE_ALLOCATION_STEP;
   glBindBuffer(GL_COPY_WRITE_BUFFER, g_EBO);
   GL_CHECK_ERRORS;
   glBufferData(GL_COPY_WRITE_BUFFER, g_elementBufferSize * 6 * sizeof(uint32_t), NULL, GL_STATIC_DRAW); // There are 6 indexes per one rectangle
   GL_CHECK_ERRORS;

   uint32_t values[6] = {0, 1, 2, 1, 3, 2};
   uint32_t *buffer = glMapBuffer(GL_COPY_WRITE_BUFFER, GL_WRITE_ONLY);
   GL_CHECK_ERRORS;
   memcpy(buffer, values, 6 * sizeof(uint32_t));
   buffer += 6;
   for (uint32_t *bufferend = buffer + (g_elementBufferSize * 6); buffer < bufferend; buffer += 6)
   {
      for (uint32_t *iterator = values, *end = values + 6; iterator < end; ++iterator)
      {
         *iterator += 4; // One rectangle has 4 vertices
      }
      memcpy(buffer, values, 6 * sizeof(uint32_t));
   }
   glUnmapBuffer(GL_COPY_WRITE_BUFFER);
   GL_CHECK_ERRORS;
}

void cce__elementToMap2DElementVertices (struct Map2DElementVertices *buffer, int32_t x, int32_t y, uint16_t width, uint16_t height,
                                         uint8_t *moveGroups, uint8_t moveGroupsQuantity, uint8_t *extensionGroups, uint8_t extensionGroupsQuantity,
                                         uint8_t globalOffset, uint8_t rotationGroup, struct Texture *textureInfo, uint16_t textureID,
                                         uint8_t *textureOffsetGroups, uint8_t textureOffsetGroupsQuantity, uint8_t *colorGroups, uint8_t colorGroupsQuantity)
{
   uint8_t i = 3;
   float textureInfoX[2] = {textureInfo->endX, textureInfo->startX};
   float textureInfoY[2] = {textureInfo->endY, textureInfo->startY};
   #define GET_SIGN_FROM_BIT(n, bit)(1-((((n)&(1<<(bit)))>>(bit))*2))
   for (struct Map2DElementVertices *vend = buffer + 4; buffer < vend; ++buffer, --i)
   {
      buffer->vertexCoords.x  = width  * GET_SIGN_FROM_BIT(i, 0);
      buffer->vertexCoords.y  = height * GET_SIGN_FROM_BIT(i, 1);
      buffer->textureCoords.x = textureInfoX[i & 1];
      buffer->textureCoords.y = textureInfoY[(i & 2) >> 1];
      buffer->position.x      = x;
      buffer->position.y      = y;
      buffer->transformGroups.rotateGroupID  = rotationGroup;
      buffer->transformGroups.isGlobalOffset = globalOffset;
      memcpy(buffer->moveIDs, moveGroups, MAX(moveGroupsQuantity, 4));
      memset(buffer->moveIDs + moveGroupsQuantity, 0, 4 - MAX(moveGroupsQuantity, 4));
      memcpy(buffer->extendIDs, extensionGroups, MAX(extensionGroupsQuantity, 4));
      memset(buffer->extendIDs + extensionGroupsQuantity, 0, 4 - MAX(extensionGroupsQuantity, 4));
      buffer->textureFragmentSize.x          = textureInfo->endX - textureInfo->startX;
      buffer->textureFragmentSize.y          = textureInfo->endY - textureInfo->startY;
      buffer->textureID                      = textureID;
      memcpy(buffer->textureOffsetIDs, textureOffsetGroups, MAX(textureOffsetGroupsQuantity, 4));
      memset(buffer->textureOffsetIDs + textureOffsetGroupsQuantity, 0, 4 - MAX(textureOffsetGroupsQuantity, 4));
      memcpy(buffer->colorIDs, colorGroups, MAX(colorGroupsQuantity, 4));
      memset(buffer->colorIDs + colorGroupsQuantity, 0, 4 - MAX(colorGroupsQuantity, 4));
   }
}

void cce__updateTexturesArray (void)
{
   GLuint freeTexturesQuantityFromEnd = 0;
   for (struct LoadedTextures *iterator = g_textures + g_texturesQuantity - 1;
        (iterator >= g_textures) && (iterator->dependantMapsQuantity == 0u); --iterator, ++freeTexturesQuantityFromEnd);

   g_texturesQuantity -= freeTexturesQuantityFromEnd;
   uint16_t size = CCE_CEIL_SIZE_TO_ALLOCATION_STEP(g_texturesQuantity);
   if (size < g_texturesQuantityAllocated)
   {
      g_textures = realloc(g_textures, size * sizeof(struct LoadedTextures));
      g_texturesQuantityAllocated = size;
   }
   int width, height, nrChannels;
   uint8_t arrayResized = 0u;
   cce_ubyte *data;
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
            shortToString(texturesPath, iterator->ID, ".png");
            data = stbi_load(texturesPath, &width, &height, &nrChannels, 4);
            *(texturesPath + texturesPathLength) = '\0';
            if (!data)
            {
               memcpy((texturesPath + texturesPathLength), "dummy.png", 10u);
               data = stbi_load(texturesPath, &width, &height, &nrChannels, 4);
               *(texturesPath + texturesPathLength) = '\0';
               if (!data)
               {
                  shortToString(texturesPath, iterator->ID, ".png");
                  cce__criticalErrorPrint("ENGINE::TEXTURE::DUMMY::FAILED_TO_LOAD:\nFailed to load dummy texture requested because %s was not found.", texturesPath);
               }
            }
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, g_texturesWidth - width, g_texturesHeight - height, (iterator - g_textures), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
            GL_CHECK_ERRORS;
            stbi_image_free(data);
            iterator->flags &= ~CCE_LOADEDTEXTURES_TOBELOADED;
         }
         else if (arrayResized)
         {
            glCopyTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, 0, 0, g_texturesWidth, g_texturesHeight);
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

static void updateUBOarray (void)
{
   uint16_t freeUBOsQuantityFromEnd = 0;
   struct UsedUBO *end = g_UBOs + g_UBOsQuantity;
   for (struct UsedUBO *iterator = g_UBOs; iterator < end; ++iterator)
   {
      if ((iterator->flags & 0x3) == 0x3)
      {
         iterator->flags &= 0x1;
         cce_setUniformBufferToDefault(iterator->UBO, *(bufferUniformsOffsets + CCE_ROTATEANGLESINCOS_OFFSET));
      }
      if (iterator->flags & 0x1)
         freeUBOsQuantityFromEnd = 0;
      else
         ++freeUBOsQuantityFromEnd;
   }
   g_UBOsQuantity -= freeUBOsQuantityFromEnd;
   uint16_t size = CCE_CEIL_SIZE_TO_ALLOCATION_STEP(g_UBOsQuantity);
   if (size < g_UBOsQuantityAllocated)
   {
      for (struct UsedUBO *iterator = end - freeUBOsQuantityFromEnd; iterator < end; ++iterator)
      {
         glDeleteBuffers(1, &(iterator->UBO));
         free(iterator->moveGroupValues);
         free(iterator->extensionGroupValues);
      }
      g_UBOs = realloc(g_UBOs, size);
      g_UBOsQuantityAllocated = size;
   }
}

uint16_t cce__getFreeUBO (void)
{
   map2Dflags |= CCE_PROCESS_UBO_ARRAY;
   for (struct UsedUBO *iterator = g_UBOs, *end = g_UBOs + g_UBOsQuantity; iterator < end; ++iterator)
   {
      if (iterator->flags & 0x1)
         continue;
      iterator->flags |= 0x1;
      return (uint16_t) (iterator - g_UBOs);
   }
   if (g_UBOsQuantity >= g_UBOsQuantityAllocated)
   {
      g_UBOsQuantityAllocated += CCE_ALLOCATION_STEP;
      g_UBOs = realloc(g_UBOs, g_UBOsQuantityAllocated * sizeof(struct UsedUBO));
      for (struct UsedUBO *iterator = g_UBOs + g_UBOsQuantityAllocated - CCE_ALLOCATION_STEP, *end = g_UBOs + g_UBOsQuantityAllocated; iterator < end; ++iterator)
      {
         glGenBuffers(1u, &(iterator->UBO));
         GL_CHECK_ERRORS;
         glBindBuffer(GL_UNIFORM_BUFFER, iterator->UBO);
         GL_CHECK_ERRORS;
         glBufferData(GL_UNIFORM_BUFFER, (g_uniformBufferSize), NULL, GL_DYNAMIC_DRAW);
         GL_CHECK_ERRORS;
         iterator->flags = 0u;
         cce_setUniformBufferToDefault(iterator->UBO, *(bufferUniformsOffsets + CCE_ROTATEANGLESINCOS_OFFSET));
         iterator->moveGroupValues=NULL;
         iterator->extensionGroupValues = NULL;
      }
   }
   struct UsedUBO *ubo = g_UBOs + g_UBOsQuantity;
   ubo->flags |= 0x1;
   return g_UBOsQuantity++;
}

void cce__releaseUBO (uint16_t ID)
{
   (g_UBOs + ID)->flags = 0x2;
   return;
}

void cce__releaseUnusedUBO (uint16_t ID)
{
   (g_UBOs + ID)->flags = 0x0;
   return;
}

void cce__allocateUBObuffers (uint16_t uboID, uint16_t moveGroupsQuantity, uint16_t extensionGroupsQuantity)
{
   if (uboID > g_UBOsQuantity)
   {
      return;
   }
   struct UsedUBO *ubo = (g_UBOs + uboID);
   if (moveGroupsQuantity > 0)
   {
      ubo->moveGroupValues      = realloc(ubo->moveGroupValues, moveGroupsQuantity           * sizeof(struct cce_ivec2));
      memset(ubo->moveGroupValues, 0, moveGroupsQuantity      * sizeof(struct cce_ivec2));
   }
   else
   {
      free(ubo->moveGroupValues);
      ubo->moveGroupValues = NULL;
   }
   ubo->moveGroupValuesQuantity = moveGroupsQuantity;
   if (extensionGroupsQuantity > 0)
   {
      ubo->extensionGroupValues = realloc(ubo->extensionGroupValues, extensionGroupsQuantity * sizeof(*(ubo->extensionGroupValues)));
      memset(ubo->extensionGroupValues, 0, extensionGroupsQuantity * sizeof(*(ubo->extensionGroupValues)));
   }
   else
   {
      free(ubo->extensionGroupValues);
      ubo->extensionGroupValues = NULL;
   }
   ubo->extensionGroupValuesQuantity = extensionGroupsQuantity;

   if (ubo->flags & 0x2)
   {
      cce_setUniformBufferToDefault(ubo->UBO, *(bufferUniformsOffsets + CCE_ROTATEANGLESINCOS_OFFSET));
      ubo->flags &= 0x1;
   }
}

// Gets called before processing of map logic begins
void cce__initLogicMap2D (struct Map2D *map)
{
   map->UBO_ID = cce__getFreeUBO();
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

static void extendLoadedTextures (uint16_t amount)
{
   g_texturesQuantityAllocated = CCE_CEIL_SIZE_TO_ALLOCATION_STEP(amount);
   g_textures = realloc(g_textures, g_texturesQuantityAllocated * sizeof(struct LoadedTextures));
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
      if (current_g_texture < g_texturesQuantity)
      {
         current_g_texture = g_texturesQuantity;
         ++g_texturesQuantity;
         if (g_texturesQuantity > g_texturesQuantityAllocated)
         {
            extendLoadedTextures(CCE_ALLOCATION_STEP);
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
   return current_g_texture + 1u;
}

uint16_t* cce__loadTexturesMap2D (struct Map2DElement *elements, uint32_t elementsQuantity, uint16_t *texturesLoadedMapReliesOnQuantity)
{
   map2Dflags |= CCE_PROCESS_TEXTURES;
   uint32_t *texturesMapReliesOn = NULL;
   uint16_t  texturesMapReliesOnQuantity = 0u, texturesMapReliesOnAllocated = 0u;
   cce_ubyte isLoaded = 0u;
   for (struct Map2DElement *iterator = elements, *end = elements + elementsQuantity - 1u; iterator < end; ++iterator)
   {
      uint32_t ID = iterator->textureInfo.ID;
      if (ID == 0u) continue;
      for (uint32_t *jiterator = texturesMapReliesOn, *jend = texturesMapReliesOn + texturesMapReliesOnQuantity; jiterator < jend; ++jiterator)
      {
         if ((*jiterator) == (ID - 1u))
         {
            isLoaded = 1u;
            break;
         }
      }
      if (!isLoaded)
      {
         if (texturesMapReliesOnQuantity == texturesMapReliesOnAllocated)
         {
            texturesMapReliesOnAllocated += CCE_ALLOCATION_STEP;
            texturesMapReliesOn = (uint32_t*) realloc(texturesMapReliesOn, texturesMapReliesOnAllocated * sizeof(uint32_t));
         }
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
               elements->textureInfo.ID = current_g_texture + 1u;
               ++((g_textures + current_g_texture)->dependantMapsQuantity);
               ++literator;
               ++kiterator;
               ++elements;
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
      elements->textureInfo.ID = (*iterator) + 1u;
      ((g_textures + (*iterator))->dependantMapsQuantity) = 1;
      ((g_textures + (*iterator))->flags) = CCE_LOADEDTEXTURES_TOBELOADED;
      ++literator, ++kiterator, ++iterator, ++elements;
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
            extendLoadedTextures(CCE_ALLOCATION_STEP);
         }
      }
      ((g_textures + current_g_texture)->ID) = (*kiterator);
      (*literator) = current_g_texture + 1u; // 0u is invalid for openGL shaders (it's the way to say "We don't need texture here"), but perfectly fine here
      elements->textureInfo.ID = current_g_texture + 1u;
      ((g_textures + current_g_texture)->dependantMapsQuantity) = 1;
      ((g_textures + current_g_texture)->flags) = CCE_LOADEDTEXTURES_TOBELOADED;
      ++literator, ++kiterator, ++current_g_texture, ++elements;
   }
   *texturesLoadedMapReliesOnQuantity = texturesMapReliesOnQuantity;
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
                                         const cce_void *elements1, size_t element1size, const struct cce_ivec2 *elements1offset,
                                         const cce_void *elements2, size_t element2size, const struct cce_ivec2 *elements2offset)
{
   cce_ubyte isDifferent = (elements1 != elements2);
   const uint32_t *group1IDs = group1firstID, *group2IDs, *groups1end = group1firstID + groups1quantity, *groups2end = group2firstID + groups2quantity;
   while (group1IDs < groups1end)
   {
      group2IDs = group2firstID;
      while (group2IDs < groups2end)
      {
         struct cce_ivec2 offset = {elements2offset->x - elements1offset->x, elements2offset->y - elements1offset->y};
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
   return cce__checkCollision((map->collisionGroups + (map->collision + ID)->group1)->elementIDs, (map->collisionGroups + (map->collision + ID)->group1)->elementsQuantity,
                              (map->collisionGroups + (map->collision + ID)->group2)->elementIDs, (map->collisionGroups + (map->collision + ID)->group2)->elementsQuantity,
                              (cce_void*) map->colliders, sizeof(struct Map2DCollider), (cce_void*) map->colliders, sizeof(struct Map2DCollider));
}

static void swapMap2D (struct Map2D **a, struct Map2D **b)
{
   struct Map2D *tmp = (*a);
   (*a) = (*b);
   (*b) = tmp;
}

/* Manages dynamic memory! */
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
      if ((((map2Dflags & CCE_PROCESS_LOGIC_FLAGS) == CCE_PROCESS_LOGIC_ONLY_FOR_CURRENT_MAP) ||
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
         cce__errorPrint("ENGINE::MAP2DARRAY_LOADER::DEPENDENCY_OF_NOTHING:\nMaps->dependies initialized without maps->main. Impossible to free maps->dependies. Possible memory leak", NULL);
      }
      maps->dependies = (struct Map2D**) malloc(maps->main->exitMapsQuantity * sizeof(struct Map2D*));
      struct ExitMap2D *exitmap = maps->main->exitMaps;
      for (struct Map2D **iterator = maps->dependies, **end = (maps->dependies + maps->main->exitMapsQuantity - 1u); iterator <= end; ++iterator, ++exitmap)
      {
         (*iterator) = cceLoadMap2D(exitmap->ID);
      }
   }
   cce__setCurrentArrayOfMaps(maps);
   if ((((map2Dflags & CCE_PROCESS_LOGIC_FLAGS) == CCE_PROCESS_LOGIC_ONLY_FOR_CURRENT_MAP) ||
        ((map2Dflags & CCE_PROCESS_LOGIC_FLAGS) == CCE_DONT_PROCESS_LOGIC)) &&
        ((map2Dflags & CCE_FORCE_INITIALIZE_MAP_ONLOAD) == 0))
   {
      cce__initLogicMap2D(maps->main);
   }
   return maps;
}

static inline int getMapBorderDistance (struct ExitMap2D *borderInfo)
{
   int32_t globalOffsetA, globalOffsetB;
   int32_t globalOffset[2];
   glGetUniformiv(shaderProgram, *(uniformLocations + 1), globalOffset);
   GL_CHECK_ERRORS;
   if (borderInfo->flags & 0x1)
   {
      globalOffsetA = -globalOffset[0];
      globalOffsetB = -globalOffset[1];
   }
   else
   {
      globalOffsetA = -globalOffset[1];
      globalOffsetB = -globalOffset[0];
   }
   if (globalOffsetB > borderInfo->b1Border && globalOffsetB <= borderInfo->b2Border)
   {
      if (borderInfo->flags & 0x2)
      {
         return -(borderInfo->aBorder - globalOffsetA) - 1;
      }
      else
      {
         return borderInfo->aBorder - globalOffsetA;
      }
   }
   else
   {
      return INT32_MAX;
   }
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
   cceLoadedMap2Dnumber = 0;
   cce__setCurrentArrayOfMaps(maps);
   int32_t closestMapDistance, currentDistance;
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
      closestMapDistance = INT32_MAX;
      lastNearestMap2D = 0;
      if (maps->main->exitMapsQuantity > 0)
      {
         for (struct ExitMap2D *iterator = maps->main->exitMaps, *end = maps->main->exitMaps + maps->main->exitMapsQuantity; iterator < end; ++iterator)
         {
            currentDistance = getMapBorderDistance(iterator);
            if (currentDistance < closestMapDistance)
            {
               closestMapDistance = currentDistance;
               lastNearestMap2D = iterator - maps->main->exitMaps;
            }
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
      if (closestMapDistance < 0)
      {
         int32_t globalOffset[2];
         glGetUniformiv(shaderProgram, *(uniformLocations + 1), globalOffset);
         GL_CHECK_ERRORS;
         struct ExitMap2D *exitMap = (maps->main->exitMaps + lastNearestMap2D);
         globalOffset[0] = globalOffset[0] + exitMap->xOffset;
         globalOffset[1] = globalOffset[1] + exitMap->yOffset;
         glUniform2i(*(uniformLocations + 1), globalOffset[0], globalOffset[1]);
         GL_CHECK_ERRORS;
         uint32_t elements = 0;
         for (uint32_t *iterator = g_dynamicMap->moveGroups->elementIDs, *end = g_dynamicMap->moveGroups->elementIDs + g_dynamicMap->moveGroups->elementsQuantity;
              iterator < end; ++iterator)
         {
            (g_dynamicMap->elements + *iterator)->x -= exitMap->xOffset;
            (g_dynamicMap->elements + *iterator)->y -= exitMap->yOffset;
            for (uint32_t i = *iterator, iend = elements; i > iend; --i)
            {
               if ((cce__getDynamicElementFlags(i - 1) & 0x12) == 0x00) // No collider AND doesn't follow global offset
                  continue;
               
               cceDeleteMap2DElementDynamicMap2D(i - 1);
            }
            elements = *iterator + 1;
         }
         cceLoadedMap2Dnumber = exitMap->ID;
         while (elements < g_dynamicMap->elementsQuantity)
         {
            cceDeleteMap2DElementDynamicMap2D(elements);
            ++elements;
         }
      }
      else
      {
         if (cceLoadedMap2Dnumber == maps->main->ID)
         {
            continue;
         }
         else
         {
            uint32_t elements = 0;
            for (uint32_t *iterator = g_dynamicMap->moveGroups->elementIDs, *end = g_dynamicMap->moveGroups->elementIDs + g_dynamicMap->moveGroups->elementsQuantity;
                 iterator < end; ++iterator)
            {
               for (uint32_t i = *iterator, iend = elements; i > iend; --i)
               {
                  if ((cce__getDynamicElementFlags(i - 1) & 0x12) == 0x00) // No collider AND doesn't follow global offset
                     continue;
                  
                  cceDeleteMap2DElementDynamicMap2D(i - 1);
               }
               elements = *iterator + 1;
            }
            while (elements < g_dynamicMap->elementsQuantity)
            {
               cceDeleteMap2DElementDynamicMap2D(elements);
               ++elements;
            }
         }
      }
      maps = loadMap2DwithDependies(maps, cceLoadedMap2Dnumber);
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
