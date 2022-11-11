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
#include <math.h>

#include "../../include/cce/engine_common.h"
#include "../../include/cce/engine_common_IO.h"
#include "../../include/cce/utils.h"
#include "../../include/cce/endianess.h"
#include "../../include/cce/map2D/actions.h"
#include "../../include/cce/map2D/map2D.h"

#include "../engine_common_internal.h"
#include "map2D_internal.h"

#define CCE_TIMER_START 0x1
#define CCE_TIMER_STOP  0x2
#define CCE_TIMER_START_STOP 0x3
#define CCE_TIMER_AUTO_RESTART_ON_ALARM 0x4

CCE_ARRAY(actions, static cce_actionfun, static uint32_t);
static void (**endianSwapActions)(void*);
static struct DelayedAction *nextDelayedActionWithoutExternalTimer;
static struct DelayedAction *lastDelayedActionWithExternalTimer;
static struct list delayedActions;
static uint32_t delayedActionsWithExternalTimerQuantity;
static struct cce_timer *nextTimer;
CCE_ARRAY(timers, static struct cce_timer, static uint16_t);

#define EXEC_ACTION(action, count)(*((actions) + *(uint32_t*)(action)))(action, count)

struct Action
{
   uint32_t ID;
};

static void transformAction (const void *data, uint8_t count)
{
   const struct transformAction *params = data;
   struct cce_i16vec2 transformation = params->transformation.move;
   uint8_t rotation = params->rotationAngle;
   if ((params->flags & 0x1) == CCE_ACTION_SHIFT)
   {
      transformation.x *= count;
      transformation.y *= count;
      rotation *= count;
   }
   switch (params->flags & 0x30)
   {
      case CCE_TRANSFORMACTION_MOVE:
         cceMoveGroup(params->groupID, transformation, params->flags & 0x1);
         break;
      case CCE_TRANSFORMACTION_SCALE:
         cceScaleGroup(params->groupID, transformation, params->flags & 0x1);
         break;
      case CCE_TRANSFORMACTION_ROTATE_WITH_OFFSET:
         cceRotateWithOffsetGroup(params->groupID, rotation, transformation, params->flags & 0x3);
         return;
   }
   if (params->flags & CCE_TRANSFORMACTION_ROTATE)
   {
      cceRotateGroup(params->groupID, rotation, params->flags & 0x3);
   }
}

static void combinedTransformAction (const void *data, uint8_t count)
{
   const struct combinedTransformAction *params = data;
   struct cce_i16vec2 transformations[3] = {params->move, params->scale, params->rotationOffset};
   uint8_t rotation = params->rotationAngle, flip = (params->cflags & 0x40) >> 4;
   transformations[0].x *= (params->cflags & CCE_COMBINEDTRANSFORMACTION_MOVE_SHIFT)             ? count : 1;
   transformations[0].y *= (params->cflags & CCE_COMBINEDTRANSFORMACTION_MOVE_SHIFT)             ? count : 1;
   transformations[1].x *= (params->cflags & CCE_COMBINEDTRANSFORMACTION_SCALE_SHIFT)            ? count : 1;
   transformations[1].y *= (params->cflags & CCE_COMBINEDTRANSFORMACTION_SCALE_SHIFT)            ? count : 1;
   transformations[2].x *= (params->cflags & CCE_COMBINEDTRANSFORMACTION_ROTATEWITHOFFSET_SHIFT) ? count : 1;
   transformations[2].y *= (params->cflags & CCE_COMBINEDTRANSFORMACTION_ROTATEWITHOFFSET_SHIFT) ? count : 1;
   rotation             *= (params->cflags & CCE_COMBINEDTRANSFORMACTION_ROTATEWITHOFFSET_SHIFT) ? count : 1;
   flip &= ~(params->cflags & CCE_COMBINEDTRANSFORMACTION_ROTATEWITHOFFSET_SHIFT) | (count & 1) << 2;
   flip >>= 1;
   cceMoveGroup(params->groupID,  transformations[0],  params->cflags & 0x1);
   cceScaleGroup(params->groupID, transformations[1], (params->cflags & 0x2) >> 1);
   cceRotateWithOffsetGroup(params->groupID, rotation, transformations[2], (params->cflags & 0x4) >> 2 | flip);
}

static void offsetTextureAction (const void *data, uint8_t count)
{
   const struct offsetTextureAction *params = data;
   if ((params->actionType & 0x1) == CCE_ACTION_SHIFT)
   {
      cceOffsetTextureGroup(params->groupID, (struct cce_i16vec2) {params->offset.x * count, params->offset.y * count}, CCE_ACTION_SHIFT);
      return;
   }
   cceOffsetTextureGroup(params->groupID, params->offset, CCE_ACTION_SET);
}

static void changeColorAction (const void *data, uint8_t count)
{
   const struct changeColorAction *params = data;
   if ((params->actionType & 0x1) == CCE_ACTION_SHIFT && count > 1)
   {
      if (params->color.rgb.type == CCE_COLOR_RGB)
      {
         cceChangeColorGroup(params->groupID, (union cce_color) {{CCE_COLOR_RGB, params->color.rgb.r * count, params->color.rgb.g * count, params->color.rgb.b * count}}, CCE_ACTION_SHIFT);
         return;
      }
      cceChangeColorGroup(params->groupID, CCE__COLOR_SET_HXX((params->color.hsv.h * count) % 3600, params->color.hsv.s * count, params->color.hsv.v * count, params->color.rgb.type), CCE_ACTION_SHIFT);
      return;
   }
   cceChangeColorGroup(params->groupID, params->color, params->actionType);
}

static void runActionsAction (const void *data, uint8_t count)
{
   const struct runActions *params = data;
   const cce_void *actionsToCall = (const cce_void*) data + sizeof(struct runActions) + (((params->actionQuantity - 1) | 1) - 1) * sizeof(uint16_t);
   for (const uint16_t *sizes = params->actionSizes, *end = params->actionSizes + params->actionQuantity; sizes < end; actionsToCall += *sizes++)
   {
      EXEC_ACTION(actionsToCall, count);
   }
}

static void setTimerStateAction (const void *data, uint8_t count)
{
   const struct setTimerStateAction *params = data;
   cceSetTimerState(params->ID, params->state);
}

static void setTimerDelayAction (const void *data, uint8_t count)
{
   const struct setTimerDelayAction *params = data;
   cceSetTimerDelay(params->ID, params->delay, params->action);
}

static void delayActionAction (const void *data, uint8_t count)
{
   const struct delayAction *params = data;
   cceDelayAction(params->timerInfo.delay, params->repeatsQuantity, params->delayedActionStructSize, ((void*) (params + 1)), params->flags);
}

static void transformActionSwapEndian (void *data)
{
   struct transformAction *params = (struct transformAction*) data;
   cceSwapEndianArrayIntN(&(params->transformation), 2, 4);
   params->groupID = cceSwapEndianInt16(params->groupID);
}

static void combinedTransformActionSwapEndian (void *data)
{
   struct combinedTransformAction *params = (struct combinedTransformAction*) data;
   cceSwapEndianArrayIntN(&(params->move), 6, 4);
   params->groupID = cceSwapEndianInt16(params->groupID);
}

static void offsetTextureActionSwapEndian (void *data)
{
   struct offsetTextureAction *params = (struct offsetTextureAction*) data;
   cceSwapEndianArrayIntN(&(params->offset), 2, 4);
}

static void changeColorActionSwapEndian (void *data)
{
   struct changeColorAction *params = (struct changeColorAction*) data;
   if (params->color.rgb.type != CCE_COLOR_RGB)
      params->color.hsv.h = cceSwapEndianInt16(params->color.hsv.h);
}

static void runActionsActionSwapEndian (void *data)
{
   struct runActions *params = data;
   params->actionQuantity = cceSwapEndianInt16(params->actionQuantity);
   cceSwapEndianArrayIntN(params->actionSizes, params->actionQuantity - 2, sizeof(uint16_t));
   cce_void *actionsToCall = (cce_void*) data + sizeof(struct runActions) + (((params->actionQuantity - 1) | 1) - 1) * sizeof(uint16_t);
   for (const uint16_t *sizes = params->actionSizes, *end = params->actionSizes + params->actionQuantity - 1; sizes < end; actionsToCall += *sizes++)
   {
      *(uint32_t*) actionsToCall = cceSwapEndianInt32(*(uint32_t*) actionsToCall);
      (*(endianSwapActions + *(uint32_t*) actionsToCall))(actionsToCall);
   }
   *(uint32_t*) actionsToCall = cceSwapEndianInt32(*(uint32_t*) actionsToCall);
   (*(endianSwapActions + *(uint32_t*) actionsToCall))(actionsToCall);
}

static void setTimerStateActionSwapEndian (void *data)
{
   struct setTimerStateAction *params = data;
   params->ID = cceSwapEndianInt16(params->ID);
}

static void setTimerDelayActionSwapEndian (void *data)
{
   struct setTimerDelayAction *params = data;
   params->ID    = cceSwapEndianInt16(params->ID);
   params->delay = cceSwapEndianInt32(params->delay);
}

static void delayActionActionSwapEndian (void *data)
{
   struct delayAction *params = data;
   params->actionID = cceSwapEndianInt32(params->actionID);
   params->delayedActionStructSize = cceSwapEndianInt32(params->delayedActionStructSize);
   params->timerInfo.delay = cceSwapEndianInt32(params->timerInfo.delay);
   params->repeatsQuantity = cceSwapEndianInt32(params->repeatsQuantity);
   (*(endianSwapActions + params->actionID))((void*) (params + 1));
}

void cce__swapActionsEndian (uint16_t *actionSizes, void *actions, uint16_t actionsQuantity)
{
   uint16_t *iterator = actionSizes, *end = actionSizes + actionsQuantity;
   cce_void *jiterator = actions;
   while (iterator < end)
   {
      *(uint32_t*)jiterator = cceSwapEndianInt32(*(uint32_t*)jiterator);
      (*(endianSwapActions + *(uint32_t*)jiterator))(jiterator);
      jiterator += *iterator;
   }
}

void cce__runActions (uint16_t *actionSizes, void *actionsToCall, uint16_t actionsQuantity)
{
   uint16_t *iterator = actionSizes, *end = actionSizes + actionsQuantity;
   cce_void *jiterator = actionsToCall;
   while (iterator < end)
   {
      EXEC_ACTION(jiterator, 1);
      jiterator += *iterator;
   }
}

#define LOAD_ACTIONS(map, sectionSize, file, onLoad, onFree, onEnter, onLeave, onLoadActionsAlloc, onFreeActionsAlloc, onEnterActionsAlloc, onLeaveActionsAlloc, \
                     onLoadActionSizesAlloc, onFreeActionSizesAlloc, onEnterActionSizesAlloc, onLeaveActionSizesAlloc) \
if (sectionSize == 0) \
      return 0; \
\
onLoad  ## Quantity = 0; \
onFree  ## Quantity = 0; \
onEnter ## Quantity = 0; \
onLeave ## Quantity = 0; \
uint32_t onLoadActionsSize = 0, onFreeActionsSize = 0, onEnterActionsSize = 0, onLeaveActionsSize = 0; \
fread(&onLoad  ## Quantity, sizeof(uint16_t), sectionSize > 0, file); \
fread(&onFree  ## Quantity, sizeof(uint16_t), sectionSize > 0, file); \
fread(&onLoadActionsSize,   sizeof(uint32_t), sectionSize > 0, file); \
fread(&onFreeActionsSize,   sizeof(uint32_t), sectionSize > 0, file); \
fread(&onEnter ## Quantity, sizeof(uint16_t), sectionSize > 1, file); \
fread(&onLeave ## Quantity, sizeof(uint16_t), sectionSize > 1, file); \
fread(&onEnterActionsSize,  sizeof(uint32_t), sectionSize > 1, file); \
fread(&onLeaveActionsSize,  sizeof(uint32_t), sectionSize > 1, file); \
\
onLoad = onLoadActionsAlloc; \
onFree = onFreeActionsAlloc; \
onEnter = onEnterActionsAlloc; \
onLeave = onLeaveActionsAlloc; \
onLoad ## Sizes = onLoadActionSizesAlloc; \
onFree  ## Sizes = onFreeActionSizesAlloc; \
onEnter ## Sizes = onEnterActionSizesAlloc; \
onLeave ## Sizes = onLeaveActionSizesAlloc; \
\
fread(onLoad  ## Sizes, sizeof(uint16_t), onLoad  ## Quantity, file); \
fread(onLoad,  1, onLoadActionsSize, file); \
fread(onFree  ## Sizes, sizeof(uint16_t), onFree  ## Quantity, file); \
fread(onFree,  1, onFreeActionsSize,  file); \
fread(onEnter ## Sizes, sizeof(uint16_t), onEnter ## Quantity, file); \
fread(onEnter, 1, onEnterActionsSize, file); \
fread(onLeave ## Sizes, sizeof(uint16_t), onLeave ## Quantity, file); \
fread(onLeave, 1, onLeaveActionsSize, file); \
\
if (g_endianess == CCE_BIG_ENDIAN) \
{ \
   cce__swapActionsEndian(onLoad  ## Sizes, onLoad,  onLoad  ## Quantity); \
   cce__swapActionsEndian(onFree  ## Sizes, onFree,  onFree  ## Quantity); \
   cce__swapActionsEndian(onEnter ## Sizes, onEnter, onEnter ## Quantity); \
   cce__swapActionsEndian(onLeave ## Sizes, onLeave, onLeave ## Quantity); \
} \

static int loadActions (void *buffer, uint8_t sectionSize, struct cce_buffer *info, FILE *file)
{
   struct ActionInfo *map = buffer;
   void *onLoadActions;
   uint16_t *onLoadActionsSizes;
   uint16_t onLoadActionsQuantity;
   void *dontCare;
   uint16_t *dontCareSizes;
   uint16_t dontCareQuantity;
   sectionSize = sectionSize > 0;
   LOAD_ACTIONS(map, sectionSize, file, onLoadActions, map->onFreeActions, dontCare, dontCare,
                malloc(onLoadActionsQuantity * sizeof(uint16_t) + onLoadActionsSize), 
                malloc(map->onFreeActionsQuantity * sizeof(uint16_t) + onFreeActionsSize),
                NULL, NULL,
                (uint16_t*)(onLoadActions + onLoadActionsSize), (uint16_t*)(map->onFreeActions + onFreeActionsSize),
                NULL, NULL)
   cce__runActions(onLoadActionsSizes, onLoadActions, onLoadActionsQuantity);
   free(onLoadActions); // We no longer need that
   return 0;
}

static int loadActionsOpenWorld (void *buffer, uint8_t sectionSize, struct cce_buffer *info, FILE *file)
{
   struct ActionInfoOpenWorld *map = buffer;
   void *onLoadActions;
   uint16_t *onLoadActionsSizes;
   uint16_t onLoadActionsQuantity;
   LOAD_ACTIONS(map, sectionSize, file, onLoadActions, map->onFreeActions, map->onEnterActions, map->onLeaveActions,
                malloc(onLoadActionsQuantity * sizeof(uint16_t) + onLoadActionsSize), 
                malloc((map->onFreeActionsQuantity + map->onEnterActionsQuantity + map->onLeaveActionsQuantity) * sizeof(uint16_t) +
                       onFreeActionsSize + onEnterActionsSize + onLeaveActionsSize),
                (map->onFreeActions + onFreeActionsSize), (map->onEnterActions + onEnterActionsSize),
                (uint16_t*)(onLoadActions + onLoadActionsSize), (uint16_t*)(map->onLeaveActions + onLeaveActionsSize),
                (map->onFreeActionsSizes + map->onFreeActionsQuantity), (map->onEnterActionsSizes + map->onEnterActionsQuantity))
   cce__runActions(onLoadActionsSizes, onLoadActions, onLoadActionsQuantity);
   free(onLoadActions); // We no longer need that
   return 0;
}

static int loadActionsDynamic (void *buffer, uint8_t sectionSize, struct cce_buffer *info, FILE *file)
{
   struct DynamicActionInfo *map = buffer;
   LOAD_ACTIONS(map, sectionSize, file, map->onLoadActions, map->onFreeActions, map->onEnterActions, map->onLeaveActions,
                malloc(onLoadActionsSize), 
                malloc(onFreeActionsSize),
                malloc(onEnterActionsSize),
                malloc(onLeaveActionsSize),
                malloc(map->onLoadActionsQuantity  * sizeof(uint16_t)),
                malloc(map->onFreeActionsQuantity  * sizeof(uint16_t)),
                malloc(map->onEnterActionsQuantity * sizeof(uint16_t)),
                malloc(map->onLeaveActionsQuantity * sizeof(uint16_t)))
   cce__runActions(map->onLoadActionsSizes, map->onLoadActions, map->onLoadActionsQuantity);
   map->onLoadActionsSizesAllocated  = map->onLoadActionsQuantity;
   map->onFreeActionsSizesAllocated  = map->onFreeActionsQuantity;
   map->onEnterActionsSizesAllocated = map->onEnterActionsQuantity;
   map->onLeaveActionsSizesAllocated = map->onLeaveActionsQuantity;
   return 0;
}

static void freeActions (void *buffer, struct cce_buffer *info)
{
   struct ActionInfo *map = buffer;
   free(map->onFreeActions);
}

static void freeActionsDynamic (void *buffer, struct cce_buffer *info)
{
   struct DynamicActionInfo *map = buffer;
   free(map->onLoadActions);
   free(map->onLoadActionsSizes);
   free(map->onFreeActions);
   free(map->onFreeActionsSizes);
   free(map->onEnterActions);
   free(map->onEnterActionsSizes);
   free(map->onLeaveActions);
   free(map->onLeaveActionsSizes);
}

#define STORE_ACTIONS_SWAP_ENDIAN(action, sizes, bytesWritten, tmpSizeStore, tmpActionStore) \
sizes = action ## Sizes, end = action ## Sizes + action ## Quantity; \
for (uint16_t *iterator = sizes; iterator < end; ++iterator) \
{ \
   tmpSizeStore = cceSwapEndianInt16(*iterator); \
   fwrite(&tmpSizeStore, sizeof(uint16_t), 1, file); \
} \
for (cce_void *iterator = (cce_void*) action; sizes < end; iterator += *sizes++) \
{ \
   memcpy(tmpActionStore, iterator, *sizes); \
   (*(endianSwapActions + ((struct Action*)tmpActionStore)->ID))(tmpActionStore); \
   fwrite(tmpActionStore, *sizes, 1, file); \
   bytesWritten += *sizes; \
}

#define STORE_ACTIONS_PRESERVE_ENDIAN(action, sizes, bytesWritten) \
fwrite(action ## Sizes, sizeof(uint16_t), action ## Quantity, file); \
sizes = action ## Sizes, end = action ## Sizes + action ## Quantity; \
for (cce_void *iterator = (cce_void*) action; sizes < end; iterator += *sizes++) \
{ \
   fwrite(iterator, *sizes, 1, file); \
   bytesWritten += *sizes; \
}

static uint8_t storeActions (void *buffer, struct cce_buffer *info, FILE *file)
{
   struct DynamicActionInfo *map = buffer;
   uint16_t tmp = cceHostEndianToLittleEndianInt16(map->onLoadActionsQuantity);
   fwrite(&tmp, sizeof(uint16_t), 1, file);
   tmp = cceHostEndianToLittleEndianInt16(map->onFreeActionsQuantity);
   fwrite(&tmp, sizeof(uint16_t), 1, file);
   size_t bytesWrittenPos = ftell(file), endPos;
   fseek(file, sizeof(uint32_t) * 2, SEEK_CUR);
   if (map->onEnterActionsQuantity > 0 || map->onLeaveActionsQuantity > 0)
   {
      tmp = cceHostEndianToLittleEndianInt16(map->onEnterActionsQuantity);
      fwrite(&tmp, sizeof(uint16_t), 1, file);
      tmp = cceHostEndianToLittleEndianInt16(map->onLeaveActionsQuantity);
      fwrite(&tmp, sizeof(uint16_t), 1, file);
      fseek(file, sizeof(uint32_t) * 2, SEEK_CUR);
   }
   uint32_t bytesWritten[4] = {0};
   uint16_t *sizes, *end;
   uint16_t maxSize = 0;
   if (g_endianess == CCE_BIG_ENDIAN)
   {
      sizes = map->onLoadActionsSizes, end = map->onLoadActionsSizes + map->onLoadActionsQuantity;
      for (uint16_t *iterator = sizes; iterator < end; ++iterator)
         maxSize = *iterator > maxSize ? *iterator : maxSize;
      
      sizes = map->onFreeActionsSizes, end = map->onFreeActionsSizes + map->onFreeActionsQuantity;
      for (uint16_t *iterator = sizes; iterator < end; ++iterator)
         maxSize = *iterator > maxSize ? *iterator : maxSize;
      
      sizes = map->onEnterActionsSizes, end = map->onEnterActionsSizes + map->onEnterActionsQuantity;
      for (uint16_t *iterator = sizes; iterator < end; ++iterator)
         maxSize = *iterator > maxSize ? *iterator : maxSize;
      
      sizes = map->onLeaveActionsSizes, end = map->onLeaveActionsSizes + map->onLeaveActionsQuantity;
      for (uint16_t *iterator = sizes; iterator < end; ++iterator)
         maxSize = *iterator > maxSize ? *iterator : maxSize;
      
      cce_void *actionTMP = malloc(maxSize);
      STORE_ACTIONS_SWAP_ENDIAN(map->onLoadActions,  sizes, bytesWritten[0], tmp, actionTMP)
      STORE_ACTIONS_SWAP_ENDIAN(map->onFreeActions,  sizes, bytesWritten[1], tmp, actionTMP)
      STORE_ACTIONS_SWAP_ENDIAN(map->onEnterActions, sizes, bytesWritten[2], tmp, actionTMP)
      STORE_ACTIONS_SWAP_ENDIAN(map->onLeaveActions, sizes, bytesWritten[3], tmp, actionTMP)
      free(actionTMP);
   }
   else
   {
      STORE_ACTIONS_PRESERVE_ENDIAN(map->onLoadActions,  sizes, bytesWritten[0])
      STORE_ACTIONS_PRESERVE_ENDIAN(map->onFreeActions,  sizes, bytesWritten[1])
      STORE_ACTIONS_PRESERVE_ENDIAN(map->onEnterActions, sizes, bytesWritten[2])
      STORE_ACTIONS_PRESERVE_ENDIAN(map->onLeaveActions, sizes, bytesWritten[3])
   }
   endPos = ftell(file);
   fseek(file, bytesWrittenPos, SEEK_SET);
   cceHostEndianToLittleEndianArrayInt32(bytesWritten, 4);
   fwrite(bytesWritten, sizeof(uint32_t), 2, file);
   if (map->onEnterActionsQuantity > 0 || map->onLeaveActionsQuantity > 0)
   {
      fseek(file, sizeof(uint16_t) * 2, SEEK_CUR);
      fwrite(bytesWritten + 2, sizeof(uint32_t), 2, file);
   }
   fseek(file, endPos, SEEK_SET);
   return (map->onEnterActionsQuantity > 0 || map->onLeaveActionsQuantity > 0) + 1;
}

void cce__actionsInit (void)
{
   actionsQuantity = CCE_BASIC_ACTIONS_QUANTITY * 2;
   timersQuantity = 0;
   CCE_ALLOC_ARRAY_ZEROED(timers);
   CCE_ALLOC_ARRAY_ZEROED(actions);
   delayedActionsWithExternalTimerQuantity = 0;
   delayedActions = LL_LIST_INIT(LL_SINGLELINKED);
   endianSwapActions = (void (**)(void*)) calloc(actionsQuantity, sizeof(void (*)(void*)));
   nextTimer = NULL;
   nextDelayedActionWithoutExternalTimer = NULL;
   lastDelayedActionWithExternalTimer = NULL;
   cceRegisterFileIOcallbacks(cce__staticMapFunctionSet,  loadActions,        freeActions,        NULL,         sizeof(struct ActionInfo));
   cceRegisterFileIOcallbacks(cce__dynamicMapFunctionSet, loadActionsDynamic, freeActionsDynamic, storeActions, sizeof(struct DynamicActionInfo));
   cceRegisterAction(CCE_TRANSFORM_ACTION,            transformAction,            transformActionSwapEndian);
   cceRegisterAction(CCE_COMBINEDTRANSFORM_ACTION,    combinedTransformAction,    combinedTransformActionSwapEndian);
   cceRegisterAction(CCE_OFFSETTEXTURE_ACTION,        offsetTextureAction,        offsetTextureActionSwapEndian);
   cceRegisterAction(CCE_CHANGECOLOR_ACTION,          changeColorAction,          changeColorActionSwapEndian);
   cceRegisterAction(CCE_DELAYACTION_ACTION,          delayActionAction,          delayActionActionSwapEndian);
   cceRegisterAction(CCE_RUNACTIONS_ACTION,           runActionsAction,          runActionsActionSwapEndian);
   //cceRegisterAction(CCE_STARTTIMER_ACTION,           startTimerAction,           startTimerActionSwapEndian);
   //cceRegisterAction(CCE_SETDYNAMICTIMERDELAY_ACTION, setDynamicTimerDelayAction, setDynamicTimerDelayActionSwapEndian);
   //cceRegisterAction(CCE_SETGRIDSIZE_ACTION,          setGridSizeAction,          setGridSizeActionSwapEndian);
   //cceRegisterAction(CCE_LOADMAP2D_ACTION,            loadMap2Daction,            loadMap2DActionSwapEndian);
}

void cce__actionsTerminate (void)
{
   llrmlist(&delayedActions);
   free(actions);
   free(endianSwapActions);
   free(timers);
}

CCE_PUBLIC_OPTIONS uint8_t cceRegisterAction (uint32_t ID, cce_actionfun action, void (*endianSwap)(void*))
{
   if (ID >= actionsAllocated)
   {
      do
      {
         CCE__REALLOC_ARRAY(actions, ID + 1);
         endianSwapActions = realloc(endianSwapActions, actionsAllocated * sizeof(cce_actionfun*));
         memset(actions           + oldAllocated, 0, (actionsAllocated - oldAllocated) * sizeof(cce_actionfun));
         memset(endianSwapActions + oldAllocated, 0, (actionsAllocated - oldAllocated) * sizeof(cce_actionfun));
      }
      while (0);
   }
   if ((ID < CCE_BASIC_ACTIONS_QUANTITY) != ((cce__map2Dflags & CCE_BASIC_ACTIONS_NOT_SET) > 0u))
      return CCE_ATTEMPT_TO_OVERRIDE_DEFAULT_ELEMENT;
   actions[ID] = action;
   endianSwapActions[ID] = endianSwap;
   actionsQuantity = CCE_MAX(ID + 1, actionsQuantity + 1);
   return 0u;
}

CCE_PUBLIC_OPTIONS void cceMoveGroup (uint8_t groupID, struct cce_i16vec2 coords, cce_enum actionType)
{
   --groupID;
   switch (actionType)
   {
      // Signed overflow is UB, so perform cast to unsigned, which is at least implementation-defined (two's complement arithmetic is required)
      case CCE_ACTION_SHIFT:
      {
         cce__transformations.move[groupID].x = (uint16_t) cce__transformations.move[groupID].x + (uint16_t) coords.x;
         cce__transformations.move[groupID].y = (uint16_t) cce__transformations.move[groupID].y + (uint16_t) coords.y;
         break;
      }
      case CCE_ACTION_SET:
      {
         coords.x = (uint16_t) coords.x - (uint16_t) cce__groupsCache.move[groupID].x;
         coords.y = (uint16_t) coords.y - (uint16_t) cce__groupsCache.move[groupID].y;
         cce__transformations.move[groupID] = coords;
         break;
      }
      default: break;
   }
   return;
}

/* Group iteration is from 1, not 0 */
CCE_PUBLIC_OPTIONS void cceScaleGroup (uint8_t groupID, struct cce_i16vec2 size, cce_enum actionType)
{
   if (groupID == 0u) return;

   --groupID;
   switch (actionType)
   {
      case CCE_ACTION_SHIFT:
      {
         cce__transformations.scale[groupID].x = (uint16_t) cce__transformations.scale[groupID].x + (uint16_t) size.x;
         cce__transformations.scale[groupID].y = (uint16_t) cce__transformations.scale[groupID].y + (uint16_t) size.y;
         break;
      }
      case CCE_ACTION_SET:
      {
         size.x = (uint16_t) size.x - (uint16_t) cce__groupsCache.scale[groupID].x;
         size.y = (uint16_t) size.y - (uint16_t) cce__groupsCache.scale[groupID].y;
         cce__transformations.scale[groupID] = size;
         break;
      }
      default: break;
   }
}

CCE_PUBLIC_OPTIONS uint8_t cceNormalizeAngle (float angleInDegrees)
{
   if (angleInDegrees < 0)
      angleInDegrees = 180.0f - angleInDegrees;
   return angleInDegrees * (255.0f/360.0f);
}

/* Group iteration is from 1, not 0 */
CCE_PUBLIC_OPTIONS void cceRotateWithOffsetGroup (uint8_t groupID, uint8_t normalizedAngle, struct cce_i16vec2 offset, cce_enum actionType)
{
   if (groupID == 0u) return;
   
   --groupID;
   switch (actionType & 1)
   {
      case CCE_ACTION_SHIFT:
      {
         cce__transformations.rotationOffset[groupID].x = (uint16_t) cce__transformations.rotationOffset[groupID].x + (uint16_t) offset.x;
         cce__transformations.rotationOffset[groupID].y = (uint16_t) cce__transformations.rotationOffset[groupID].y + (uint16_t) offset.y;
         cce__transformations.rotationAngles[groupID] += normalizedAngle;
         break;
      }
      case CCE_ACTION_SET:
      {
         offset.x = (uint16_t) offset.x - (uint16_t) cce__groupsCache.rotationOffset[groupID].x;
         offset.y = (uint16_t) offset.y - (uint16_t) cce__groupsCache.rotationOffset[groupID].y;
         normalizedAngle -= cce__groupsCache.rotationAngles[groupID];
         
         cce__transformations.rotationOffset[groupID] = offset;
         cce__transformations.rotationAngles[groupID] = normalizedAngle;
         break;
      }
      default: break;
   }
}

/* Group iteration is from 1, not 0 */
CCE_PUBLIC_OPTIONS void cceRotateGroup (uint8_t groupID, uint8_t normalizedAngle, cce_enum actionType)
{
   if (groupID == 0u) return;
   
   --groupID;
   switch (actionType & 1)
   {
      case CCE_ACTION_SHIFT:
      {
         cce__transformations.rotationAngles[groupID] += normalizedAngle;
         break;
      }
      case CCE_ACTION_SET:
      {
         normalizedAngle -= cce__groupsCache.rotationAngles[groupID];
         cce__transformations.rotationAngles[groupID] = normalizedAngle;
         break;
      }
      default: break;
   }
}

/* Group iteration is from 1, not 0 */
CCE_PUBLIC_OPTIONS void cceChangeColorGroup (uint8_t groupID, union cce_color color, cce_enum actionType)
{
   if (groupID == 0u || groupID > 128) return;
   
   --groupID;
   color = cceColorToColorType(color, cce__map2Dflags);
   switch (actionType)
   {
      case CCE_ACTION_SHIFT:
      {
         if (cce__map2Dflags & CCE_STORE_COLOR_IN == CCE_STORE_COLOR_IN_RGB)
         {
            cce__transformations.color[groupID].rgb.r += color.rgb.r;
            cce__transformations.color[groupID].rgb.g += color.rgb.g;
            cce__transformations.color[groupID].rgb.b += color.rgb.b;
         }
         else
         {
            cce__transformations.color[groupID].hsv.h += CCE_COLOR_GET_HUE(color);
            cce__transformations.color[groupID].hsv.h -= (cce__transformations.color[groupID].hsv.h >= 3600) * 3600;
            cce__transformations.color[groupID].hsv.s += color.hsv.s;
            cce__transformations.color[groupID].hsv.v += color.hsv.v;
         }
         break;
      }
      case CCE_ACTION_SET:
      {
         if (cce__map2Dflags & CCE_STORE_COLOR_IN == CCE_STORE_COLOR_IN_RGB)
         {
            color.rgb.r -= cce__groupsCache.color[groupID].rgb.r;
            color.rgb.g -= cce__groupsCache.color[groupID].rgb.g;
            color.rgb.b -= cce__groupsCache.color[groupID].rgb.b;
            cce__transformations.color[groupID].rgb = color.rgb;
         }
         else
         {
            color.hsv.h = CCE_COLOR_GET_HUE(color);
            color.hsv.h -= cce__groupsCache.color[groupID].hsv.h;
            color.hsv.h += (color.hsv.h >= 3600) * 3600; // Overflow occured
            color.hsv.s -= cce__groupsCache.color[groupID].hsv.s;
            color.hsv.v -= cce__groupsCache.color[groupID].hsv.v;
            cce__transformations.color[groupID].hsv = color.hsv; // Dont care about type, stored already in cce__map2Dflags
         }
         break;
      }
      default: break;
   }
}

CCE_PUBLIC_OPTIONS void cceOffsetTextureGroup (uint8_t groupID, struct cce_i16vec2 offset, cce_enum actionType)
{
   if (groupID == 0u) return;

   --groupID;
   switch (actionType)
   {
      case CCE_ACTION_SHIFT:
      {
         cce__transformations.textureOffset[groupID].x = (uint16_t) cce__transformations.textureOffset[groupID].x + (uint16_t) offset.x;
         cce__transformations.textureOffset[groupID].y = (uint16_t) cce__transformations.textureOffset[groupID].y + (uint16_t) offset.y;
         break;
      }
      case CCE_ACTION_SET:
      {
         offset.x = (uint16_t) offset.x - (uint16_t) cce__groupsCache.textureOffset[groupID].x;
         offset.y = (uint16_t) offset.y - (uint16_t) cce__groupsCache.textureOffset[groupID].y;
         cce__transformations.textureOffset[groupID] = offset;
         break;
      }
      default: break;
   }
}

CCE_PUBLIC_OPTIONS void cceSetTimerState (uint16_t timerID, uint8_t state)
{
   if (timerID >= timersAllocated)
   {
      CCE_REALLOC_ARRAY_ZEROED(timers, timerID + 1);
   }
   timers[timerID].flags &= ~(CCE_TIMER_START_STOP | ((state & CCE_CHANGETIMERSTATE_DISABLE_AUTO_RESTART_ON_ALARM) >> 1));
   timers[timerID].flags |= (state & CCE_CHANGETIMERSTATE_SWITCH) ^ (((state & CCE_CHANGETIMERSTATE_SWITCH) == CCE_CHANGETIMERSTATE_SWITCH) << (timers[timerID].initTime == 0));
   timers[timerID].flags |= state & CCE_CHANGETIMERSTATE_ENABLE_AUTO_RESTART_ON_ALARM;
   timers[timerID].flags ^= (state & CCE_CHANGETIMERSTATE_SWITCH_AUTO_RESTART_ON_ALARM >> 2);
   cce__map2Dflags |= CCE_PROCESS_TIMERS;
}

CCE_PUBLIC_OPTIONS void cceSetTimerDelay (uint16_t timerID, uint32_t newDelay, uint8_t actionType)
{
   if (timerID >= timersAllocated)
   {
      CCE_REALLOC_ARRAY_ZEROED(timers, timerID + 1);
   }
   if (actionType == CCE_ACTION_SHIFT)
   {
      newDelay += timers[timerID].delay;
   }
   if (timers[timerID].initTime + timers[timerID].delay >= cce__currentTime)
   {
      // Avoid making troubles to delayAction handlers
      timers[timerID].initTime += (int64_t) timers[timerID].delay - (int64_t) newDelay;
   }
   else if (timers + timerID == nextTimer && timers[timerID].delay < newDelay)
   {
      nextTimer = NULL; // Must be found again
   }
   else if (timers[timerID].initTime + newDelay < nextTimer->initTime + nextTimer->delay)
   {
      nextTimer = timers + timerID;
   }
   timers[timerID].delay = newDelay;
}

CCE_PUBLIC_OPTIONS void cceDelayAction (uint16_t repeatsQuantity, uint32_t delayOrID, uint32_t actionStructSize, void *actionStruct, uint8_t flags)
{
   if (flags & CCE_DELAYACTION_EXECUTE_ON_START)
      EXEC_ACTION(actionStruct, 1);
   
   cce_void *data = llnodealloc(sizeof(struct DelayedAction) + actionStructSize, delayedActions.type);
   LL_NEXT(data) = NULL;
   struct DelayedAction *head = (struct DelayedAction*) data;
   if (flags & CCE_DELAYACTION_NEVER_END)
      head->repeatsLeft = 1;
   else
      head->repeatsLeft = repeatsQuantity;
   head->delay = delayOrID;
   head->initTime = 0;
   if ((flags & CCE_DELAYACTION_EXTERNAL_TIMER | CCE_DELAYACTION_START_EXTERNAL_TIMER))
      timers[delayOrID].flags |= CCE_TIMER_START;
         
   head->initTime = cce__currentTime;
   head->flags = flags & (CCE_DELAYACTION_EXTERNAL_TIMER | CCE_DELAYACTION_NEVER_END | CCE_DELAYACTION_EXECUTE_ONCE_PER_TIMER_ALARM | CCE_DELAYACTION_RESTART_EXTERNAL_TIMER_ON_ALARM);
   memcpy(data + sizeof(struct DelayedAction), actionStruct, actionStructSize);
   
   if (flags & CCE_DELAYACTION_EXTERNAL_TIMER)
   {
      llprependchain(&delayedActions, data);
      if (delayedActionsWithExternalTimerQuantity == 0)
         lastDelayedActionWithExternalTimer = head;
      ++delayedActionsWithExternalTimerQuantity;
   }
   else
   {
      if (nextDelayedActionWithoutExternalTimer != NULL || head->initTime + head->delay < nextDelayedActionWithoutExternalTimer->initTime + nextDelayedActionWithoutExternalTimer->delay)
         llinsertchainp(&delayedActions, lastDelayedActionWithExternalTimer, data);
      else
         llappendchain(&delayedActions, data);
   }
}

// Uses cache inefficiently (linked lists are bad). But arrays are out of luck here because actions have variable-size structs (making them hard to use with arrays)
void cce__runDelayedActions (void)
{
   cce_void *prevnode = NULL, *node = delayedActions.chain;
   struct DelayedAction *head;
   uint32_t i = 0;
   int32_t timerDiff;
   uint8_t count;
   // If no timers expired, no need to check actions dependant on them
   if (nextTimer != NULL && (nextTimer->initTime + nextTimer->delay <= cce__currentTime))
   {
      while (i < delayedActionsWithExternalTimerQuantity)
      {
         head = (struct DelayedAction*) node;
         struct cce_timer *timer = timers + head->delay;
         if (timer->initTime == 0 || (timerDiff = cce__currentTime - (timer->initTime + timer->delay)) < 0)
            goto CONTINUE;
         count = 1;
         if (timerDiff >= timer->delay && !(head->flags & CCE_DELAYACTION_EXECUTE_ONCE_PER_TIMER_ALARM) &&
            (timer->flags & CCE_TIMER_AUTO_RESTART_ON_ALARM || head->flags & CCE_DELAYACTION_RESTART_EXTERNAL_TIMER_ON_ALARM) &&
            timer->delay != 0)
         {
            count = timerDiff / timer->delay + 1;
            count = CCE_MAX(count, head->repeatsLeft | -((head->flags & CCE_DELAYACTION_NEVER_END) > 0));
         }
         EXEC_ACTION(node + sizeof(struct DelayedAction), 1);
         head->repeatsLeft -= count * ((head->flags & CCE_DELAYACTION_NEVER_END) == 0);
         if (head->repeatsLeft == 0)
         {
            llrmsublistp(&delayedActions, prevnode, 1);
            --delayedActionsWithExternalTimerQuantity;
            if (prevnode == NULL)
               node = delayedActions.chain;
            else
               node = LL_NEXT(prevnode);
         }
         else
         {
            if (head->flags & CCE_DELAYACTION_RESTART_EXTERNAL_TIMER_ON_ALARM)
            {
               timer->flags |= CCE_TIMER_START;
            }
   CONTINUE:
            prevnode = node;
            node = LL_NEXT(node);
            ++i;
         }
      }
      // Could be changed
      lastDelayedActionWithExternalTimer = (struct DelayedAction*) prevnode;
   }
   else
   {
      node     = (cce_void*) nextDelayedActionWithoutExternalTimer;
      prevnode = (cce_void*) lastDelayedActionWithExternalTimer;
   }
   head = (struct DelayedAction*) node;
   if (node == NULL || (timerDiff = cce__currentTime - (head->initTime + head->delay)) < 0)
      return; // Skip checking remaining actions because the one with least remaining time hasn't expired
   struct DelayedAction *prevMinDelayNode = lastDelayedActionWithExternalTimer;
   uint16_t minimalTimeLeft = UINT16_MAX;
   while (node != NULL)
   {
      head = (struct DelayedAction*) node;
      if ((timerDiff = cce__currentTime - (head->initTime + head->delay)) < 0)
      {
         if (minimalTimeLeft > (uint16_t)(-timerDiff))
         {
            prevMinDelayNode = (struct DelayedAction*) prevnode;
            minimalTimeLeft = -timerDiff;
         }
         goto CONTINUE2;
      }
      count = 1;
      if (timerDiff >= head->delay && !(head->flags & CCE_DELAYACTION_EXECUTE_ONCE_PER_TIMER_ALARM) && head->delay > 0)
      {
         count = timerDiff / head->delay + 1;
         count = CCE_MAX(count, head->repeatsLeft | -((head->flags & CCE_DELAYACTION_NEVER_END) > 0));
      }
      EXEC_ACTION(node + sizeof(struct DelayedAction), 1);
      
      head->repeatsLeft -= count * ((head->flags & CCE_DELAYACTION_NEVER_END) == 0);
      if (head->repeatsLeft == 0)
      {
         llrmsublistp(&delayedActions, prevnode, 1);
         --delayedActionsWithExternalTimerQuantity;
         if (prevnode == NULL)
            node = delayedActions.chain;
         else
            node = LL_NEXT(prevnode);
      }
      else
      {
         head->initTime = cce__currentTime;
CONTINUE2:
         prevnode = node;
         node = LL_NEXT(node);
         ++i;
      }
   }
   // Allows checking action that has the least time remaining first (and skipping checking remaining ones)
   llinsertchainp(&delayedActions, lastDelayedActionWithExternalTimer, nextDelayedActionWithoutExternalTimer = llpullnodep(&delayedActions, prevMinDelayNode));
}

void cce__processTimers (void)
{
   struct cce_timer *minRemainsTimer;
   uint64_t minRemains;
   int64_t diff;
   if (nextTimer == NULL /* Was lost, must be found again */ || nextTimer->initTime + nextTimer->delay <= cce__currentTime)
   {
      minRemains = INT64_MAX;
      minRemainsTimer = NULL;
      for (struct cce_timer *iterator = timers, *end = timers + timersQuantity; iterator < end; ++iterator)
      {
         diff = cce__currentTime - (iterator->initTime + iterator->delay);
         if (diff >= 0 && iterator->flags & CCE_TIMER_AUTO_RESTART_ON_ALARM || iterator->flags & CCE_TIMER_START)
         {
            diff *= (diff > 0 && iterator->initTime > 0 && iterator->delay > 0);
            if (diff > (int64_t) iterator->delay)
            {
               diff %= iterator->delay;
            }
            iterator->initTime = cce__currentTime - diff;
            diff = -iterator->delay - diff;
         }
         else if (diff >= 0 || iterator->flags & CCE_TIMER_STOP)
         {
            iterator->initTime = 0;
            continue;
         }
         if (minRemains > (uint64_t) -diff)
         {
            minRemains = -diff;
            minRemainsTimer = iterator;
         }
      }
   }
   else if (cce__map2Dflags & CCE_PROCESS_TIMERS)
   {
      minRemainsTimer = nextTimer;
      minRemains = cce__currentTime - (nextTimer->initTime + nextTimer->delay);
      for (struct cce_timer *iterator = timers, *end = timers + timersQuantity; iterator < end; ++iterator)
      {
         if (iterator->flags & CCE_TIMER_STOP)
         {
            iterator->initTime = 0;
            continue;
         }
         
         if (!(iterator->flags & CCE_TIMER_START))
            continue;
         
         iterator->initTime = cce__currentTime;
         if (minRemains > iterator->delay)
         {
            minRemains = iterator->delay;
            minRemainsTimer = iterator;
         }
      }
   }
   cce__map2Dflags &= ~CCE_PROCESS_TIMERS;
   nextTimer = minRemainsTimer;
   return;
}
