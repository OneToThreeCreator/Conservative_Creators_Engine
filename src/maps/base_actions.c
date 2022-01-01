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
#include <math.h>

#include "../engine_common.h"
#include "../shader.h"
#include "../path_getters.h"
#include "map2D.h"
#include "map2D_internal.h"
#include "base_actions.h"

static const struct Map2Darray *allMaps;
static const struct Map2D *currentMap;
static const struct DynamicMap2D *g_dynamicMap;
static struct UsedUBO *g_UBOs;
static const GLint *g_uniformsOffsets;
static const GLint *g_uniformLocations;
static GLuint g_shaderProgram;
cce_void *buf;
static void (*g_setUniformBufferToDefault)(GLuint, GLint);

#define PI 3.14159265f

static void moveAction (void *data)
{
   struct moveActionStruct *params = (struct moveActionStruct*) data;
   cceMoveGroupMap2D(params->groupID, params->x, params->y);
}

static void extendAction (void *data)
{
   struct extendActionStruct *params = (struct extendActionStruct*) data;
   cceExtendGroupMap2D(params->groupID, params->x, params->y);
}

static void rotateAction (void *data)
{
   struct rotateActionStruct *params = (struct rotateActionStruct*) data;
   cceRotateGroupMap2D(params->groupID, params->angle, params->xOffset, params->yOffset);
}

static void offsetTextureAction (void *data)
{
   struct offsetTextureActionStruct *params = (struct offsetTextureActionStruct*) data;
   cceOffsetTextureGroupMap2D(params->groupID, params->offsetX, params->offsetY);
}

static void changeColorAction (void *data)
{
   struct changeColorActionStruct *params = (struct changeColorActionStruct*) data;
   cceChangeColorGroupMap2D(params->groupID, params->red, params->green, params->blue, params->alpha);
}

static void setBoolAction (void *data)
{
   struct setBoolActionStruct *params = (struct setBoolActionStruct*) data;
   cceSetBool(params->boolID, params->action);
}

static void setPlotNumberAction (void *data)
{
   struct setPlotNumberActionStruct *params = (struct setPlotNumberActionStruct*) data;
   switch (params->action)
   {
      case CCE_INCREASE_PLOT_NUMBER:
      {
         cceIncreasePlotNumber(params->value);
         break;
      }
      case CCE_SET_PLOT_NUMBER:
      {
         cceSetPlotNumber(params->value);
         break;
      }
   }
}

static void startTimerAction (void *data)
{
   uint16_t ID = *((uint16_t*) data);
   cceStartTimer(currentMap->timers + ID);
}

void cce__beginBaseActions (const struct Map2D *map)
{
   currentMap = map;
   if ((g_UBOs + currentMap->UBO_ID)->flags & 0x4)
   {
      g_setUniformBufferToDefault((g_UBOs + currentMap->UBO_ID)->UBO, *(g_uniformsOffsets + 6));
      (g_UBOs + currentMap->UBO_ID)->flags &= 0x1;
   }
   glBindBuffer(GL_UNIFORM_BUFFER, (g_UBOs + currentMap->UBO_ID)->UBO);
   GL_CHECK_ERRORS;
   buf = glMapBuffer(GL_UNIFORM_BUFFER, GL_READ_WRITE);
   GL_CHECK_ERRORS;
}

void cce__endBaseActions (void)
{
   glUnmapBuffer(GL_UNIFORM_BUFFER);
   GL_CHECK_ERRORS;
   currentMap = NULL;
}

void cce__setCurrentArrayOfMaps (const struct Map2Darray *maps)
{
   allMaps = maps;
}

void cce__baseActionsInit (const struct DynamicMap2D *dynamic_map, struct UsedUBO *UBOs, const GLint *bufferUniformsOffsets, 
                      const GLint *uniformLocations, GLuint shaderProgram, void (*setUniformBufferToDefault)(GLuint, GLint))
{
   g_dynamicMap = dynamic_map;
   g_UBOs = UBOs;
   g_uniformsOffsets = bufferUniformsOffsets;
   g_uniformLocations = uniformLocations;
   g_shaderProgram = shaderProgram;
   g_setUniformBufferToDefault = setUniformBufferToDefault;
   cceRegisterAction(0u, moveAction);
   cceRegisterAction(1u, extendAction);
   cceRegisterAction(2u, rotateAction);
   cceRegisterAction(3u, offsetTextureAction);
   cceRegisterAction(4u, changeColorAction);
   cceRegisterAction(5u, setBoolAction);
   cceRegisterAction(6u, setPlotNumberAction);
   cceRegisterAction(7u, startTimerAction);
   
}

static void moveGroupInSpecificMap2D (const struct Map2D *map, uint16_t groupID, int32_t x, int32_t y)
{
   for (uint32_t *iterator = (map->moveGroups + groupID)->elementIDs, *end = (map->moveGroups + groupID)->elementIDs + 
       (map->moveGroups + groupID)->elementsQuantity; iterator < end; ++iterator)
   {
      (map->colliders + *iterator)->x += x;
      (map->colliders + *iterator)->y += y;
   }
}

/* 0 is global offset, 1 - 255 is plain offset */
CCE_PUBLIC_OPTIONS void cceMoveGroupMap2D (uint16_t groupID, int32_t x, int32_t y)
{
   if (groupID == 0u)
   {
      int32_t coords[2];
      glGetUniformiv(g_shaderProgram, *(g_uniformLocations + 1u), coords);
      GL_CHECK_ERRORS;
      coords[0] += x;
      coords[1] += y;
      glUniform2iv(*(g_uniformLocations + 1u), 1u, coords);
      GL_CHECK_ERRORS;
      moveGroupInSpecificMap2D(allMaps->main, groupID, -x, -y);
      for (struct Map2D **iterator = allMaps->dependies, **end = allMaps->dependies + allMaps->main->exitMapsQuantity; iterator < end; ++iterator)
      {
         moveGroupInSpecificMap2D(*iterator, groupID, -x, -y);
      }
   }
   else
   {
      *(((int32_t*) (buf + *(g_uniformsOffsets + CCE_MOVEGROUP_OFFSET))) + (groupID - 1u) * 2u)      += x;
      *(((int32_t*) (buf + *(g_uniformsOffsets + CCE_MOVEGROUP_OFFSET))) + (groupID - 1u) * 2u + 1u) += y;
      moveGroupInSpecificMap2D(currentMap, groupID, x, y);
   }
}

/* Group iteration is from 1, not 0 */
CCE_PUBLIC_OPTIONS void cceExtendGroupMap2D (uint16_t groupID, int32_t x, int32_t y)
{
   if (groupID == 0u) return;
   
   *(((int32_t*) (buf + *(g_uniformsOffsets + CCE_EXTENSIONGROUP_OFFSET))) + (groupID - 1u) * 2u)      += x;
   *(((int32_t*) (buf + *(g_uniformsOffsets + CCE_EXTENSIONGROUP_OFFSET))) + (groupID - 1u) * 2u + 1u) += y;
   for (uint32_t *iterator = (currentMap->extensionGroups + groupID - 1u)->elementIDs, *end = (currentMap->extensionGroups + groupID - 1u)->elementIDs + 
       (currentMap->extensionGroups + groupID - 1u)->elementsQuantity; iterator < end; ++iterator)
   {
      (currentMap->colliders + *iterator)->width  += x;
      (currentMap->colliders + *iterator)->height += y;
   }
}

CCE_PUBLIC_OPTIONS float cceNormalizeAngle (float angleInDegrees)
{
   return angleInDegrees * (1.0f/180.0f);
}

/* Group iteration is from 1, not 0 */
CCE_PUBLIC_OPTIONS void cceRotateGroupMap2D (uint8_t groupID, float normalizedAngle, int32_t xOffset, int32_t yOffset)
{
   if (groupID == 0u) return;
   float sin = sinf(normalizedAngle * PI);
   float cos = cosf(normalizedAngle * PI);
   *(((float*)   (buf + *(g_uniformsOffsets + CCE_ROTATEANGLESIN_OFFSET))) + (groupID - 1u) * 4u)      = sin;
   *(((float*)   (buf + *(g_uniformsOffsets + CCE_ROTATEANGLECOS_OFFSET))) + (groupID - 1u) * 4u)      = cos;
   *(((int32_t*) (buf + *(g_uniformsOffsets + CCE_ROTATIONOFFSET_OFFSET))) + (groupID - 1u) * 4u)      = xOffset;
   *(((int32_t*) (buf + *(g_uniformsOffsets + CCE_ROTATIONOFFSET_OFFSET))) + (groupID - 1u) * 4u + 1u) = yOffset;
}

/* Group iteration is from 1, not 0 */
CCE_PUBLIC_OPTIONS void cceChangeColorGroupMap2D (uint8_t groupID, float r, float g, float b, float a)
{
   if (groupID == 0u) return;
   *(((float*) (buf + *(g_uniformsOffsets + CCE_COLORGROUP_OFFSET))) + (groupID - 1u) * 4u)      = r;
   *(((float*) (buf + *(g_uniformsOffsets + CCE_COLORGROUP_OFFSET))) + (groupID - 1u) * 4u + 1u) = g;
   *(((float*) (buf + *(g_uniformsOffsets + CCE_COLORGROUP_OFFSET))) + (groupID - 1u) * 4u + 2u) = b;
   *(((float*) (buf + *(g_uniformsOffsets + CCE_COLORGROUP_OFFSET))) + (groupID - 1u) * 4u + 3u) = a;
}

CCE_PUBLIC_OPTIONS void cceOffsetTextureGroupMap2D (uint8_t groupID, int32_t offsetX, int32_t offsetY)
{
   if (groupID == 0u) return;
   *(((int32_t*) (buf + *(g_uniformsOffsets + CCE_TEXTUREOFFSET_OFFSET))) + (groupID - 1u) * 2u)      = offsetX;
   *(((int32_t*) (buf + *(g_uniformsOffsets + CCE_TEXTUREOFFSET_OFFSET))) + (groupID - 1u) * 2u + 1u) = offsetY;
}