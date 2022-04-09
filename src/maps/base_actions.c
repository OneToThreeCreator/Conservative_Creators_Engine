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
#include "../platform/endianess.h"
#include "../platform/path_getters.h"
#include "map2D.h"
#include "map2D_internal.h"
#include "base_actions.h"

static const struct Map2Darray *allMaps;
static const struct Map2D *currentMap;
static const struct DynamicMap2D *g_dynamicMap;
static struct UsedUBO *g_UBOs;
static const GLint *g_uniformsOffsets;
static const GLint *g_uniformLocations;
static const GLint *g_uniformBufferSize;
static GLuint g_shaderProgram;
cce_void *g_currentMapBuffer = NULL;
cce_void *g_dynamicMapBuffer = NULL;
static void (*g_setUniformBufferToDefault)(GLuint, GLint);
static GLint uniformOffset;

#define PI 3.14159265f

static void moveAction (void *data)
{
   struct moveActionStruct *params = (struct moveActionStruct*) data;
   cceMoveGroupMap2D(params->groupID, params->x, params->y, params->action, params->mapType);
}

static void extendAction (void *data)
{
   struct extendActionStruct *params = (struct extendActionStruct*) data;
   cceExtendGroupMap2D(params->groupID, params->x, params->y, params->action, params->mapType);
}

static void rotateAction (void *data)
{
   struct rotateActionStruct *params = (struct rotateActionStruct*) data;
   cceRotateGroupMap2D(params->groupID, params->angle, params->xOffset, params->yOffset, params->action, params->mapType);
}

static void offsetTextureAction (void *data)
{
   struct offsetTextureActionStruct *params = (struct offsetTextureActionStruct*) data;
   cceOffsetTextureGroupMap2D(params->groupID, params->offsetX, params->offsetY, params->mapType);
}

static void changeColorAction (void *data)
{
   struct changeColorActionStruct *params = (struct changeColorActionStruct*) data;
   cceChangeColorGroupMap2D(params->groupID, params->red, params->green, params->blue, params->alpha, params->mapType);
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
      case CCE_SHIFT:
         cceIncreasePlotNumber(params->value);
         break;
      case CCE_SET:
         cceSetPlotNumber(params->value);
         break;
   }
}

static void startTimerAction (void *data)
{
   struct startTimerActionStruct *params = ((struct startTimerActionStruct*) data);
   switch (params->mapType)
   {
      case CCE_CURRENT_MAP2D:
         cceStartTimer(currentMap->timers + params->ID);
         break;
      case CCE_DYNAMIC_MAP2D:
         cceStartTimerDynamicMap2D(params->ID);
         break;
   }
}

static void setDynamicTimerDelayAction (void *data)
{
   struct setDynamicTimerDelayActionStruct *params = ((struct setDynamicTimerDelayActionStruct*) data);
   switch (params->action)
   {
      case CCE_SET:
      {
         cceSetTimerDelayDynamicMap2D(params->ID, params->delay);
         break;
      }
      case CCE_SHIFT:
      {
         struct Timer timer = cceGetTimerDynamicMap2D(params->ID);
         cceSetTimerDelayDynamicMap2D(params->ID, timer.delay + params->delay);
         break;
      }
   }
}

static void setGridSizeAction (void *data)
{
   cceSetGridMultiplier(*((float*) data));
}

static void loadMap2Daction (void *data)
{
   struct loadMap2DactionStruct *params = ((struct loadMap2DactionStruct*) data);
   switch (params->action)
   {
      case CCE_SHIFT:
         cceLoadedMap2Dnumber += params->ID;
         break;
      case CCE_SET:
         cceLoadedMap2Dnumber =  params->ID;
         break;
   }
}

static void moveActionSwapEndian (void *data)
{
   struct moveActionStruct *params = (struct moveActionStruct*) data;
   cceSwapEndianArrayIntN(&(params->x), 2, 4);
   params->groupID = cceSwapEndianInt16(params->groupID);
}

static void extendActionSwapEndian (void *data)
{
   struct extendActionStruct *params = (struct extendActionStruct*) data;
   cceSwapEndianArrayIntN(&(params->x), 2, 4);
   params->groupID = cceSwapEndianInt16(params->groupID);
}

static void rotateActionSwapEndian (void *data)
{
   struct rotateActionStruct *params = (struct rotateActionStruct*) data;
   cceSwapEndianArrayIntN(&(params->angle), 3, 4); // Assuming float endianess is the same as int's
}

static void offsetTextureActionSwapEndian (void *data)
{
   struct offsetTextureActionStruct *params = (struct offsetTextureActionStruct*) data;
   cceSwapEndianArrayIntN(&(params->offsetX), 2, 4);
}

static void changeColorActionSwapEndian (void *data)
{
   struct changeColorActionStruct *params = (struct changeColorActionStruct*) data;
   cceSwapEndianArrayIntN(&(params->red), 4, 4); // Assuming float endianess is the same as int's
}

static void setBoolActionSwapEndian (void *data)
{
   struct setBoolActionStruct *params = (struct setBoolActionStruct*) data;
   params->boolID = cceSwapEndianInt16(params->boolID);
}

static void setPlotNumberActionSwapEndian (void *data)
{
   struct setPlotNumberActionStruct *params = (struct setPlotNumberActionStruct*) data;
   params->value = cceSwapEndianInt16(params->value);
}

static void startTimerActionSwapEndian (void *data)
{
   struct startTimerActionStruct *params = (struct startTimerActionStruct*) data;
   params->ID = cceSwapEndianInt16(params->ID);
}

static void setDynamicTimerDelayActionSwapEndian (void *data)
{
   struct setDynamicTimerDelayActionStruct *params = (struct setDynamicTimerDelayActionStruct*) data;
   params->delay = cceSwapEndianInt32(params->delay);
   params->ID = cceSwapEndianInt16(params->ID);
}

static void setGridSizeActionSwapEndian (void *data)
{
   *((uint32_t*) data) = cceSwapEndianInt32(*(uint32_t*) data);
}

static void loadMap2DActionSwapEndian (void *data)
{
   struct loadMap2DactionStruct *params = (struct loadMap2DactionStruct*) data;
   params->ID = cceSwapEndianInt16(params->ID);
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
   g_currentMapBuffer = glMapBufferRange(GL_UNIFORM_BUFFER, uniformOffset, *g_uniformBufferSize - uniformOffset, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
   GL_CHECK_ERRORS;
   if (g_dynamicMapBuffer == NULL)
   {
      glBindBuffer(GL_UNIFORM_BUFFER, (g_UBOs + g_dynamicMap->UBO_ID)->UBO);
      GL_CHECK_ERRORS;
      g_dynamicMapBuffer = glMapBufferRange(GL_UNIFORM_BUFFER, uniformOffset, *g_uniformBufferSize - uniformOffset, GL_MAP_READ_BIT | GL_MAP_WRITE_BIT);
      GL_CHECK_ERRORS;
   }
   glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

static void endBaseActionsCommon (uint16_t uboID)
{
   struct UsedUBO *ubo = (g_UBOs + uboID);
   glBindBuffer(GL_UNIFORM_BUFFER, ubo->UBO);
   GL_CHECK_ERRORS;
   glUnmapBuffer(GL_UNIFORM_BUFFER);
   GL_CHECK_ERRORS;
   if (ubo->moveGroupValuesQuantity == 0 && ubo->extensionGroupValuesQuantity == 0)
      return;
   struct cce_ivec2 *buffer = glMapBufferRange(GL_UNIFORM_BUFFER, *(g_uniformsOffsets + CCE_MOVEGROUP_OFFSET), 2 * sizeof(struct cce_ivec2) * 255,
                                               GL_MAP_WRITE_BIT | (GL_MAP_INVALIDATE_RANGE_BIT * (((ubo->moveGroupValuesQuantity)      >= 255) &&
                                                                                                  ((ubo->extensionGroupValuesQuantity) >= 255))));
   GL_CHECK_ERRORS;
   memcpy(buffer, ubo->moveGroupValues, MIN(ubo->moveGroupValuesQuantity, 255) * sizeof(struct cce_ivec2));
   buffer += 255;
   struct
   {
      int16_t x;
      int16_t y;
   } *extensionValues = (void*) (ubo->extensionGroupValues);
   for (struct cce_ivec2 *end = buffer + MIN(ubo->extensionGroupValuesQuantity, 255); buffer < end; ++buffer, ++extensionValues)
   {
      *buffer = (struct cce_ivec2) {extensionValues->x, extensionValues->y};
   }
   glUnmapBuffer(GL_UNIFORM_BUFFER);
   GL_CHECK_ERRORS;
}

void cce__endBaseActions (void)
{
   endBaseActionsCommon(currentMap->UBO_ID);
   currentMap = NULL;
   g_currentMapBuffer = NULL;
}

void cce__endBaseActionsDynamicMap2D (void)
{
   endBaseActionsCommon(g_dynamicMap->UBO_ID);
   g_dynamicMapBuffer = NULL;
}

void cce__setCurrentArrayOfMaps (const struct Map2Darray *maps)
{
   allMaps = maps;
}

void cce__baseActionsInit (const struct DynamicMap2D *dynamic_map, struct UsedUBO *UBOs, const GLint *bufferUniformsOffsets, 
                           const GLint *uniformLocations, GLuint shaderProgram, void (*setUniformBufferToDefault)(GLuint, GLint),
                           const GLint *uniformBufferSize)
{
   g_dynamicMap = dynamic_map;
   g_UBOs = UBOs;
   g_uniformsOffsets = bufferUniformsOffsets;
   g_uniformLocations = uniformLocations;
   g_shaderProgram = shaderProgram;
   g_setUniformBufferToDefault = setUniformBufferToDefault;
   g_uniformBufferSize = uniformBufferSize;
   uniformOffset = *(g_uniformsOffsets + CCE_COLORGROUP_OFFSET);
   cceRegisterAction(0,  moveAction, moveActionSwapEndian);
   cceRegisterAction(1,  extendAction, extendActionSwapEndian);
   cceRegisterAction(2,  rotateAction, rotateActionSwapEndian);
   cceRegisterAction(3,  offsetTextureAction, offsetTextureActionSwapEndian);
   cceRegisterAction(4,  changeColorAction, changeColorActionSwapEndian);
   cceRegisterAction(5,  setBoolAction, setBoolActionSwapEndian);
   cceRegisterAction(6,  setPlotNumberAction, setPlotNumberActionSwapEndian);
   cceRegisterAction(7,  startTimerAction, startTimerActionSwapEndian);
   cceRegisterAction(8,  setDynamicTimerDelayAction, setDynamicTimerDelayActionSwapEndian);
   cceRegisterAction(9,  setGridSizeAction, setGridSizeActionSwapEndian);
   cceRegisterAction(10, loadMap2Daction, loadMap2DActionSwapEndian);
}

static inline void moveElements (int32_t *firstElementX, int32_t *firstElementY, ptrdiff_t step, struct ElementGroup *group, int32_t x, int32_t y)
{
   step = step / sizeof(int32_t); // requires alignment, most likely provided anyway
   for (uint32_t *iterator = group->elementIDs, *end = group->elementIDs + group->elementsQuantity; iterator < end; ++iterator)
   {
      *(firstElementX + step * (*iterator)) += x;
      *(firstElementY + step * (*iterator)) += y;
   }
}

CCE_PUBLIC_OPTIONS void cceMoveGlobalOffsetGroupMap2D (int32_t x, int32_t y, cce_enum actionType)
{
   switch (actionType)
   {
      case CCE_SHIFT:
      {
         int32_t coords[2];
         glGetUniformiv(g_shaderProgram, *(g_uniformLocations + 1u), coords);
         GL_CHECK_ERRORS;
         coords[0] += x;
         coords[1] += y;
         glUniform2iv(*(g_uniformLocations + 1u), 1u, coords);
         break;
      }
      case CCE_SET:
      {
         glUniform2i(*(g_uniformLocations + 1u), x, y);
         int32_t offset[2];
         glGetUniformiv(g_shaderProgram, *(g_uniformLocations + 1u), offset);
         GL_CHECK_ERRORS;
         x -= offset[0];
         y -= offset[1];
         break;
      }
      default: return;
   }
   GL_CHECK_ERRORS;
   struct Map2D *map = allMaps->main;
   if (map->moveGroupsQuantity > 0)
      moveElements(&(map->colliders->x), &(map->colliders->y), sizeof(struct Map2DCollider), map->moveGroups, -x, -y);
   for (struct Map2D **iterator = allMaps->dependies, **end = allMaps->dependies + allMaps->main->exitMapsQuantity; iterator < end; ++iterator)
   {
      if ((*iterator)->moveGroupsQuantity > 0)
         moveElements(&((*iterator)->colliders->x), &((*iterator)->colliders->y), sizeof(struct Map2DCollider), (*iterator)->moveGroups, -x, -y);
   }
   if (g_dynamicMap->moveGroupsQuantity > 0)
      moveElements(&(g_dynamicMap->elements->x), &(g_dynamicMap->elements->y), sizeof(struct DynamicMap2DElement), (struct ElementGroup*) g_dynamicMap->moveGroups, -x, -y);
}

CCE_PUBLIC_OPTIONS void cceMoveGroupMap2D (uint16_t groupID, int32_t x, int32_t y, cce_enum actionType, cce_enum mapType)
{
   if (groupID == 0)
   {
      return cceMoveGlobalOffsetGroupMap2D(x, y, actionType);
   }
   struct UsedUBO *ubo;
   int32_t *firstElementX, *firstElementY;
   ptrdiff_t elementSize;
   struct ElementGroup *group = NULL;

   switch (mapType)
   {
      case CCE_CURRENT_MAP2D:
         ubo = (g_UBOs + currentMap->UBO_ID);
         if (currentMap->moveGroupsQuantity > groupID)
         {
            firstElementX = &(currentMap->colliders->x);
            firstElementY = &(currentMap->colliders->y);
            elementSize = sizeof(struct Map2DCollider);
            group = currentMap->moveGroups + groupID;
         }
         break;
      case CCE_DYNAMIC_MAP2D:
         ubo = (g_UBOs + g_dynamicMap->UBO_ID);
         if (g_dynamicMap->moveGroupsQuantity > groupID)
         {
            firstElementX = &(g_dynamicMap->elements->x);
            firstElementY = &(g_dynamicMap->elements->y);
            elementSize = sizeof(struct DynamicMap2DElement);
            group = (struct ElementGroup*) (g_dynamicMap->moveGroups + groupID);
         }
         break;
      default: return;
   }

   switch (actionType)
   {
      case CCE_SHIFT:
      {
         (ubo->moveGroupValues + (groupID - 1u))->x += x;
         (ubo->moveGroupValues + (groupID - 1u))->y += y;
         break;
      }
      case CCE_SET:
      {
         int32_t offset[2];
         offset[0] = (ubo->moveGroupValues + (groupID - 1u))->x;
         offset[1] = (ubo->moveGroupValues + (groupID - 1u))->y;
         (ubo->moveGroupValues + (groupID - 1u))->x = x;
         (ubo->moveGroupValues + (groupID - 1u))->y = y;
         x -= offset[0];
         y -= offset[1];
         break;
      }
      default: return;
   }
   if (group != NULL)
      moveElements(firstElementX, firstElementY, elementSize, group, x, y);
}

/* Group iteration is from 1, not 0 */
CCE_PUBLIC_OPTIONS void cceExtendGroupMap2D (uint16_t groupID, int32_t x, int32_t y, cce_enum actionType, cce_enum mapType)
{
   if (groupID == 0u) return;

   struct UsedUBO *ubo;
   uint16_t *firstElementWidth, *firstElementHeight;
   ptrdiff_t elementSize;
   struct ElementGroup *group = NULL;

   switch (mapType)
   {
      case CCE_CURRENT_MAP2D:
         ubo = (g_UBOs + currentMap->UBO_ID);
         if (currentMap->extensionGroupsQuantity > groupID)
         {
            firstElementWidth = &(currentMap->colliders->width);
            firstElementHeight = &(currentMap->colliders->height);
            elementSize = sizeof(struct Map2DCollider) / sizeof(int32_t);
            group = currentMap->extensionGroups + groupID;
         }
         break;
      case CCE_DYNAMIC_MAP2D:
         ubo = (g_UBOs + g_dynamicMap->UBO_ID);
         if (g_dynamicMap->extensionGroupsQuantity > groupID)
         {
            firstElementWidth = &(g_dynamicMap->elements->width);
            firstElementHeight = &(g_dynamicMap->elements->height);
            elementSize = sizeof(struct DynamicMap2DElement) / sizeof(int32_t);
            group = (struct ElementGroup*) g_dynamicMap->extensionGroups + groupID;
         }
         break;
      default: return;
   }

   switch (actionType)
   {
      case CCE_SHIFT:
      {
         (ubo->extensionGroupValues + (groupID - 1u))->x += x;
         (ubo->extensionGroupValues + (groupID - 1u))->y += y;
         break;
      }
      case CCE_SET:
      {
         uint16_t resize[2];
         resize[0] = (ubo->extensionGroupValues + (groupID - 1u))->x;
         resize[1] = (ubo->extensionGroupValues + (groupID - 1u))->y;
         (ubo->extensionGroupValues + (groupID - 1u))->x = x;
         (ubo->extensionGroupValues + (groupID - 1u))->y = y;
         x -= resize[0];
         y -= resize[1];
         break;
      }
   }
   if (group == NULL)
      return;
   for (uint32_t *iterator = (group + groupID - 1u)->elementIDs, *end = (group + groupID - 1u)->elementIDs + (group + groupID - 1u)->elementsQuantity; iterator < end; ++iterator)
   {
      *(firstElementWidth + *iterator * elementSize)  += x;
      *(firstElementHeight + *iterator * elementSize) += y;
   }
}

CCE_PUBLIC_OPTIONS float cceNormalizeAngle (float angleInDegrees)
{
   return angleInDegrees * (1.0f/180.0f);
}

/* Group iteration is from 1, not 0 */
CCE_PUBLIC_OPTIONS void cceRotateGroupMap2D (uint8_t groupID, float normalizedAngle, int32_t xOffset, int32_t yOffset, cce_enum actionType, cce_enum mapType)
{
   if (groupID == 0u) return;

   cce_void *glBuffer;
   struct UsedUBO *ubo;
   switch (mapType)
   {
      case CCE_CURRENT_MAP2D:
         ubo = (g_UBOs + currentMap->UBO_ID);
         glBuffer = g_currentMapBuffer;
         break;
      case CCE_DYNAMIC_MAP2D:
         ubo = (g_UBOs + g_dynamicMap->UBO_ID);
         glBuffer = g_dynamicMapBuffer;
         break;
      default: return;
   }

   switch (actionType)
   {
      case CCE_SHIFT:
         *(ubo->rotationAngles + (groupID - 1u)) += normalizedAngle;
         normalizedAngle = *(ubo->rotationAngles + (groupID - 1u));
         break;
      case CCE_SET:
         *(ubo->rotationAngles + (groupID - 1u)) = normalizedAngle;
         break;
   }

   float sin = sinf(normalizedAngle * PI);
   float cos = cosf(normalizedAngle * PI);

   (((struct cce_vec2*)  (glBuffer + *(g_uniformsOffsets + CCE_ROTATEANGLESINCOS_OFFSET) - uniformOffset)) + (groupID - 1u))->x = sin;
   (((struct cce_vec2*)  (glBuffer + *(g_uniformsOffsets + CCE_ROTATEANGLESINCOS_OFFSET) - uniformOffset)) + (groupID - 1u))->y = cos;
   (((struct cce_ivec2*) (glBuffer + *(g_uniformsOffsets + CCE_ROTATIONOFFSET_OFFSET)    - uniformOffset)) + (groupID - 1u))->x = xOffset;
   (((struct cce_ivec2*) (glBuffer + *(g_uniformsOffsets + CCE_ROTATIONOFFSET_OFFSET)    - uniformOffset)) + (groupID - 1u))->y = yOffset;
}

/* Group iteration is from 1, not 0 */
CCE_PUBLIC_OPTIONS void cceChangeColorGroupMap2D (uint8_t groupID, float r, float g, float b, float a, cce_enum mapType)
{
   if (groupID == 0u) return;

   cce_void *glBuffer;
   switch (mapType)
   {
      case CCE_CURRENT_MAP2D:
         glBuffer = g_currentMapBuffer;
         break;
      case CCE_DYNAMIC_MAP2D:
         glBuffer = g_dynamicMapBuffer;
         break;
      default: return;
   }

   *(((float*) (glBuffer + *(g_uniformsOffsets + CCE_COLORGROUP_OFFSET) - uniformOffset)) + (groupID - 1u) * 4u)      = r;
   *(((float*) (glBuffer + *(g_uniformsOffsets + CCE_COLORGROUP_OFFSET) - uniformOffset)) + (groupID - 1u) * 4u + 1u) = g;
   *(((float*) (glBuffer + *(g_uniformsOffsets + CCE_COLORGROUP_OFFSET) - uniformOffset)) + (groupID - 1u) * 4u + 2u) = b;
   *(((float*) (glBuffer + *(g_uniformsOffsets + CCE_COLORGROUP_OFFSET) - uniformOffset)) + (groupID - 1u) * 4u + 3u) = a;
}

CCE_PUBLIC_OPTIONS void cceOffsetTextureGroupMap2D (uint8_t groupID, int32_t offsetX, int32_t offsetY, cce_enum mapType)
{
   if (groupID == 0u) return;

   cce_void *glBuffer;
   switch (mapType)
   {
      case CCE_CURRENT_MAP2D:
         glBuffer = g_currentMapBuffer;
         break;
      case CCE_DYNAMIC_MAP2D:
         glBuffer = g_dynamicMapBuffer;
         break;
      default: return;
   }

   (((struct cce_ivec2*) (glBuffer + *(g_uniformsOffsets + CCE_TEXTUREOFFSET_OFFSET) - uniformOffset)) + (groupID - 1u))->x = offsetX;
   (((struct cce_ivec2*) (glBuffer + *(g_uniformsOffsets + CCE_TEXTUREOFFSET_OFFSET) - uniformOffset)) + (groupID - 1u))->y = offsetY;
}
