/*
   Conservative Creator's Engine - open source engine for making games.
   Copyright © 2020-2023 Andrey Gaivoronskiy

   This file is part of Conservative Creator's Engine.

   Conservative Creator's Engine is free software: you can redistribute it and/or modify it under 
   the terms of the GNU Lesser General Public License as published by the Free Software Foundation,
   either version 2.1 of the License, or (at your option) any later version.

   Conservative Creator's Engine is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
   PURPOSE. See the GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License along
   with Conservative Creator's Engine. If not, see <https://www.gnu.org/licenses/old-licenses/lgpl-2.1.html>.
*/

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <math.h>

#include <listlib.h>

#include "../../include/cce/engine_common.h"
#include "../../include/cce/engine_common_IO.h"
#include "../../include/cce/utils.h"
#include "../../include/cce/endianess.h"
#include "../../include/cce/plugins/actions.h"
#include "../../include/cce/plugins/actions_internal.h"

CCE_ARRAY_STRUCT(cce_uidsResizable,    uint32_t, uint32_t);
CCE_ARRAY_STRUCT(cce_actionsResizable, struct cceaAction, uint32_t);

struct ccea_actioninfo
{
   struct list                      delayedActions;
   uint32_t                         currentMapTime;
   uint16_t                         eventsQuantity;
   
   // Slow linear-time performance for insertion and deletion. 
   // The only way to improve it (I found) is to use binary trees (not yet implemented). At least it is cache-friendly...
   struct cce_uidsResizable    *actionSubsUIDs;
   struct cce_actionsResizable *onEventActions;
};

struct ccea_runActionsDelayed
{
   uint32_t actionID;
   uint32_t totalSize;
   uint32_t timeout;
};

struct ccea_runActionsPeriodic
{
   uint32_t actionID;
   uint32_t totalSize;
   uint32_t timeout;
   uint32_t delay;
};

struct ccea_runActionsDelayedRepeated
{
   uint32_t actionID;
   uint32_t totalSize;
   uint32_t timeout;
   uint32_t delay;
   uint32_t repeatsLeft;
};


#define CCE_ACTIONS_SWAPPING_FROM_HOST_ENDIAN 0x1
#define CCE_ACTIONS_INITIALIZING 0x2

CCE_ARRAY(g_actions, static ccea_actionfun, static uint32_t);
static void   (**g_endianSwapActions)(void*) = NULL;
static uint32_t *g_actionSizes = NULL;

// Becomes hash table after initialization to speed up searches (designed to do A LOT of them).
// It would be better to avoid this, but then saving it all in a file would need a lot of mess to be created. #KISS
static uint32_t *g_actionUIDs = NULL;

CCE_ARRAY(g_eventUIDs, static uint32_t, static uint16_t);

CCE_API uint32_t cceaBasicActionUIDs[16];

CCE_API uint32_t cceaBasicEventsUIDs[3];

CCE_API uint32_t cceaPluginUID = 0;

static uint32_t **g_eventUIDsSorted = NULL;
static uint8_t g_flags;

static void runActions (void *data, uint32_t count, struct cce_buffer *state)
{
   struct cceaRunActions *params = data;
   ccea__runActions((struct cceaAction*)(params + 1), params->totalSize - sizeof(struct cceaRunActions), count, state);
}

static void runActionsOnce (void *data, uint32_t count, struct cce_buffer *state)
{
   CCE_UNUSED(count);
   struct cceaRunActions *params = data;
   ccea__runActions((struct cceaAction*)(params + 1), params->totalSize - sizeof(struct cceaRunActions), 1, state);
}

static void runActionsNTimes (void *data, uint32_t count, struct cce_buffer *state)
{
   struct cceaRunActionsNTimes *params = data;
   ccea__runActions((struct cceaAction*)(params + 1), params->totalSize - sizeof(struct cceaRunActionsNTimes), count * params->n, state);
}

static void runDelayedActions (void *data, uint32_t count, struct cce_buffer *state)
{
   struct ccea_runActionsDelayed *params = data;
   ccea__runActions((struct cceaAction*)(params + 1), params->totalSize - sizeof(struct ccea_runActionsDelayed), count, state);
}

static void runPeriodicActions (void *data, uint32_t count, struct cce_buffer *state)
{
   struct ccea_runActionsPeriodic *params = data;
   uint32_t currentTime = ((struct ccea_actioninfo*)CCE_GET_FUNCTION_BUFFER(state, cceaPluginUID))->currentMapTime;
   count *= (currentTime - params->timeout) / params->delay + 1;
   params->timeout = currentTime - (currentTime - params->timeout) % params->delay + params->delay;
   ccea__runActions((struct cceaAction*)(params + 1), params->totalSize - sizeof(struct ccea_runActionsPeriodic), count, state);
}

static void runRepeatedlyDelayedActions (void *data, uint32_t count, struct cce_buffer *state)
{
   struct ccea_runActionsDelayedRepeated *params = data;
   uint32_t currentTime = ((struct ccea_actioninfo*)CCE_GET_FUNCTION_BUFFER(state, cceaPluginUID))->currentMapTime;
   count *= CCE_MIN((currentTime - params->timeout) / params->delay + 1, params->repeatsLeft);
   if (count < params->repeatsLeft)
      params->timeout = currentTime - (currentTime - params->timeout) % params->delay + params->delay;
   ccea__runActions((struct cceaAction*)(params + 1), params->totalSize - sizeof(struct ccea_runActionsDelayedRepeated), count, state);
   params->repeatsLeft -= count;
}

static void delayActions (void *data, uint32_t count, struct cce_buffer *state)
{
   CCE_UNUSED(count);
   struct cceaDelayActions *params = data;
   uint32_t currentTime = ((struct ccea_actioninfo*)CCE_GET_FUNCTION_BUFFER(state, cceaPluginUID))->currentMapTime;
   ccea__delayDynamicAction(cceaBasicActionUIDs[CCEA_RUN_DELAYED_ACTIONS], params->delay + currentTime, params + 1, params->totalSize - sizeof(struct cceaDelayActions), state);
}

static void delayActionsPeriodic (void *data, uint32_t count, struct cce_buffer *state)
{
   CCE_UNUSED(count);
   struct cceaDelayActionsPeriodic *params = data;
   uint32_t currentTime = ((struct ccea_actioninfo*)CCE_GET_FUNCTION_BUFFER(state, cceaPluginUID))->currentMapTime;
   ccea__delayDynamicAction(cceaBasicActionUIDs[CCEA_RUN_PERIODIC_ACTIONS], params->delay + currentTime, (cce_void*)params + 2 * sizeof(uint32_t), params->totalSize - 2 * sizeof(uint32_t), state);
}

static void delayActionsRepeated (void *data, uint32_t count, struct cce_buffer *state)
{
   CCE_UNUSED(count);
   struct cceaDelayActionsRepeated *params = data;
   uint32_t currentTime = ((struct ccea_actioninfo*)CCE_GET_FUNCTION_BUFFER(state, cceaPluginUID))->currentMapTime;
   ccea__delayDynamicAction(cceaBasicActionUIDs[CCEA_RUN_REPEATED_ACTIONS], params->delay + currentTime, (cce_void*)params + 2 * sizeof(uint32_t), params->totalSize - 2 * sizeof(uint32_t), state);
}

static void terminateEngine (void *data, uint32_t count, struct cce_buffer *state)
{
   CCE_UNUSED(data);
   CCE_UNUSED(count);
   CCE_UNUSED(state);
   cceSetEngineShouldTerminate(1);
}

#define UID_TO_ID(_uid, _id) \
do \
{ \
   _id = cceUIDToHash(_uid, g_actionsAllocated); \
   while (g_actionUIDs[_id] != _uid) \
   { \
      _id += 1; \
      if (_id >= g_actionsAllocated) \
         _id = 0; \
   } \
} \
while (0)

static void addActionsOnEvent (void *data, uint32_t count, struct cce_buffer *state)
{
   struct cceaAddActionsOnEvent *params = data;
   
   if (count > 1)
   {
      struct cceaRunActionsNTimes wrapper;
      wrapper.UID = cceaBasicActionUIDs[CCEA_RUN_ACTIONS_N_TIMES];
      wrapper.totalSize = sizeof(struct cceaRunActionsNTimes);
      wrapper.n = count;
      ccea__addActionsOnEvent(params->eventUID, params->actionSubsUID, params->totalSize - sizeof(struct cceaAddActionsOnEvent),
                              (struct cceaAction*)(params + 1), (struct cceaActionRunner*)&wrapper, state);
      return;
   }
   struct cceaDynamicAction *action = (struct cceaDynamicAction*)(params + 1);
   uint32_t actionID;
   UID_TO_ID(action->UID, actionID);
   uint32_t size = (g_actionSizes[actionID] == 0) ? action->size : g_actionSizes[actionID];
   if (params->totalSize == size + sizeof(struct cceaAddActionsOnEvent))
   {
      ccea__addActionsOnEvent(params->eventUID, params->actionSubsUID, params->totalSize - sizeof(struct cceaAddActionsOnEvent),
                              (struct cceaAction*)(params + 1), NULL, state);
      return;
   }
   struct cceaRunActions wrapper;
   wrapper.UID = cceaBasicActionUIDs[CCEA_RUN_ACTIONS];
   wrapper.totalSize = sizeof(struct cceaRunActions);
   ccea__addActionsOnEvent(params->eventUID, params->actionSubsUID, params->totalSize - sizeof(struct cceaAddActionsOnEvent),
                           (struct cceaAction*)(params + 1), (struct cceaActionRunner*)&wrapper, state);
}

static void removeActionsOnEvent (void *data, uint32_t count, struct cce_buffer *state)
{
   CCE_UNUSED(count);
   struct cceaRemoveActionOnEvent *params = data;
   cceaRemoveActionOnEvent(params->eventUID, params->actionSubsUID, state);
}

static void invokeEvent (void *data, uint32_t count, struct cce_buffer *state)
{
   struct cceaInvokeEvent *params = data;
   cceaInvokeEvent(params->eventUID, count, state);
}

static void appendListOfRunDelayedAction (void *data, uint32_t count, struct cce_buffer *state)
{
   CCE_UNUSED(count);
   struct cceaAppendListOfRunDelayedActions *params = data;
   struct ccea_actioninfo *actionInfo = (struct ccea_actioninfo*)CCE_GET_FUNCTION_BUFFER(state, cceaPluginUID);
   uint32_t size = 0;
   for (cce_void *it = (cce_void*)(params + 1), *end = (cce_void*)(params + 1) + params->totalSize - sizeof(struct cceaAppendListOfRunDelayedActions); it < end; it += size)
   {
      struct cceaDelayedDynamicAction *action = (struct cceaDelayedDynamicAction*)it;
      uint32_t ID;
      UID_TO_ID(action->UID, ID);
      size = (g_actionSizes[ID] == 0) ? action->size : g_actionSizes[ID];
      #ifndef NDEBUG
      if (size == 0)
         fprintf(stderr, "ENGINE::ACTIONS_PLUGIN::INFINITE_LOOP: action (uid: %u) has 0 size. Something went very wrong...", action->UID), abort();
      #endif
      void *node = llnodealloc(size, LL_SINGLELINKED);
      memcpy(node, action, size);
      llappendchain(&actionInfo->delayedActions, node);
   }
}

#define RUNACTIONS_SWAP_ENDIAN(data, params, paramsSize) \
if (g_flags & CCE_ACTIONS_SWAPPING_FROM_HOST_ENDIAN) \
{ \
   ccea__fromHostEndianActionsArray((struct cceaAction*)((cce_void*)(params) + (paramsSize)), (params)->totalSize - (paramsSize)); \
   g_flags |= CCE_ACTIONS_SWAPPING_FROM_HOST_ENDIAN; \
} \
else \
   ccea__toHostEndianActionsArray((struct cceaAction*)((cce_void*)(params) + (paramsSize)), (params)->totalSize - (paramsSize))

static void runActionsActionSwapEndian (void *data)
{
   struct cceaRunActions *params = data;
   RUNACTIONS_SWAP_ENDIAN(data, params, sizeof(params));
}

static void runActionsSwapEndianInt1 (void *data)
{
   uint32_t *params = data;
   params[2] = cceSwapEndianInt32(params[2]);
   RUNACTIONS_SWAP_ENDIAN(data, (struct cceaRunActions*)params, 3 * sizeof(uint32_t));
}

static void runActionsSwapEndianInt2 (void *data)
{
   uint32_t *params = data;
   params[2] = cceSwapEndianInt32(params[2]);
   params[3] = cceSwapEndianInt32(params[3]);
   RUNACTIONS_SWAP_ENDIAN(data, (struct cceaRunActions*)params, 4 * sizeof(uint32_t));
}

static void runActionsSwapEndianInt3 (void *data)
{
   uint32_t *params = data;
   params[2] = cceSwapEndianInt32(params[2]);
   params[3] = cceSwapEndianInt32(params[3]);
   params[4] = cceSwapEndianInt32(params[4]);
   RUNACTIONS_SWAP_ENDIAN(data, (struct cceaRunActions*)params, 5 * sizeof(uint32_t));
}

static void swapEndianInt1 (void *data)
{
   uint32_t *params = data;
   cceSwapEndianArrayInt32(params + 1, 1);
}

static void swapEndianInt2 (void *data)
{
   uint32_t *params = data;
   cceSwapEndianArrayInt32(params + 1, 2);
}

static void noSwapEndian (void *data)
{
   CCE_UNUSED(data);
}

void ccea__swapActionsEndian (uint16_t *actionSizes, void *actionsToSwap, uint16_t actionsToSwapQuantity)
{
   uint16_t *iterator = actionSizes, *end = actionSizes + actionsToSwapQuantity;
   cce_void *jiterator = actionsToSwap;
   while (iterator < end)
   {
      *(uint32_t*)jiterator = cceSwapEndianInt32(*(uint32_t*)jiterator);
      if (*(g_endianSwapActions + *(uint32_t*)jiterator) != NULL)
         (*(g_endianSwapActions + *(uint32_t*)jiterator))(jiterator);
      jiterator += *iterator;
   }
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
   if (*(g_endianSwapActions + ((struct Action*)tmpActionStore)->ID) != NULL) \
      (*(g_endianSwapActions + ((struct Action*)tmpActionStore)->ID))(tmpActionStore); \
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

#define EXEC_ACTION_GET_ID(action, count, state, _id) \
UID_TO_ID(((struct cceaAction*)action)->UID, _id); \
(g_actions[_id])(action, count, state)

#define EXEC_ACTION(action, count, state) \
do \
{ \
   uint32_t _id; \
   EXEC_ACTION_GET_ID(action, count, state, _id); \
} \
while (0)

CCE_API void ccea__runActions (struct cceaAction *actions, uint32_t totalActionsSize, uint32_t count, struct cce_buffer *state)
{
   cce_void *action;
   uint32_t actionID;
   for (uint32_t size = 0; size < totalActionsSize; size += (g_actionSizes[actionID] == 0) ? ((struct cceaDynamicAction*)actions)->size : g_actionSizes[actionID])
   {
      action = ((cce_void*)actions) + size;
      EXEC_ACTION_GET_ID(action, count, state, actionID);
   }
}

CCE_API void cceaRunAction (struct cceaAction *action, uint32_t count, struct cce_buffer *state)
{
   EXEC_ACTION(action, count, state);
}

CCE_API uint8_t ccea__doActionsSwapFromHostEndian (void)
{
   return (g_flags & CCE_ACTIONS_SWAPPING_FROM_HOST_ENDIAN) != 0;
}

CCE_API void ccea__fromHostEndianActionsArray (struct cceaAction *actions, uint32_t totalActionsSize)
{
   g_flags |= CCE_ACTIONS_SWAPPING_FROM_HOST_ENDIAN;
   uint32_t size = 0;
   while (size < totalActionsSize)
   {
      uint32_t ID;
      UID_TO_ID(actions->UID, ID);
      (*((g_endianSwapActions) + ID))(actions);
      size += g_actionSizes[ID];
      if (g_actionSizes[ID] == 0)
      {
         size += ((struct cceaDynamicAction*)actions)->size;
         ((struct cceaDynamicAction*)actions)->size = cceSwapEndianInt32(((struct cceaDynamicAction*)actions)->size);
      }
      actions->UID = cceSwapEndianInt32(actions->UID);
      actions += size;
   }
   g_flags &= ~CCE_ACTIONS_SWAPPING_FROM_HOST_ENDIAN;
}

CCE_API void ccea__toHostEndianActionsArray (struct cceaAction *actions, uint32_t totalActionsSize)
{
   uint32_t size = 0;
   while (size < totalActionsSize)
   {
      actions->UID = cceSwapEndianInt32(actions->UID);
      uint32_t ID;
      UID_TO_ID(actions->UID, ID);
      (*((g_endianSwapActions) + ID))(actions);
      size += g_actionSizes[ID];
      if (g_actionSizes[ID] == 0)
      {
         ((struct cceaDynamicAction*)actions)->size = cceSwapEndianInt32(((struct cceaDynamicAction*)actions)->size);
         size += ((struct cceaDynamicAction*)actions)->size;
      }
      actions += size;
   }
}

#define ADD_TO_HASH_TABLE(_uid, _hash) \
_hash = cceUIDToHash(_uid, g_actionsAllocated); \
while (g_actionUIDs[_hash] != 0) \
{ \
   _hash += 1; \
   if (_hash >= g_actionsAllocated) \
      _hash = 0; \
} \
g_actionUIDs[_hash] = _uid

static void resizeHashTable (uint32_t newSize)
{
   g_actionsAllocated = newSize;
   uint32_t *oldTable = g_actionUIDs;
   ccea_actionfun *oldActions = g_actions;
   uint32_t *oldSizes = g_actionSizes;
   void (**oldActionsSwapEndian)(void*) = g_endianSwapActions;
   g_actionUIDs        = calloc(newSize, sizeof(uint32_t));
   g_actions           = calloc(newSize, sizeof(ccea_actionfun));
   g_endianSwapActions = calloc(newSize, sizeof(void (*)(void*)));
   g_actionSizes       = calloc(newSize, sizeof(uint32_t));
   
   uint32_t hash;
   for (uint32_t i = 0, j = 0; j < g_actionsQuantity; ++i)
   {
      if (oldTable[i] == 0)
         continue;
      ADD_TO_HASH_TABLE(oldTable[i], hash);
      g_actions[hash] = oldActions[i];
      g_endianSwapActions[hash] = oldActionsSwapEndian[i];
      g_actionSizes[hash] = oldSizes[i];
      ++j;
   }
   free(oldTable);
   free(oldActions);
   free(oldActionsSwapEndian);
   free(oldSizes);
}

CCE_API void cceaRegisterAction (uint32_t actionUID, ccea_actionfun action, void (*endianSwap)(void*), uint32_t actionSize)
{
   // Let's not build hash table until we know (at least approximately) how big it should be
   if (g_flags & CCE_ACTIONS_INITIALIZING)
   {
      if (g_actionsQuantity >= g_actionsAllocated) do
      {
         CCE__REALLOC_ARRAY(g_actions, g_actionsQuantity + 1u);
         g_endianSwapActions = realloc(g_endianSwapActions, g_actionsAllocated * sizeof(void (*)(void*)));
         g_actionSizes       = realloc(g_actionSizes,       g_actionsAllocated * sizeof(uint32_t));
         if (g_flags & CCE_ACTIONS_INITIALIZING)
            g_actionUIDs     = realloc(g_actionUIDs,        g_actionsAllocated * sizeof(uint32_t));
         memset(g_actions           + oldAllocated, 0, (g_actionsAllocated - oldAllocated) * sizeof(ccea_actionfun));
         memset(g_endianSwapActions + oldAllocated, 0, (g_actionsAllocated - oldAllocated) * sizeof(void (*)(void*)));
         memset(g_actionSizes       + oldAllocated, 0, (g_actionsAllocated - oldAllocated) * sizeof(uint32_t));
      }
      while (0);
      g_actionUIDs[g_actionsQuantity] = actionUID;
      g_actions[g_actionsQuantity] = action;
      g_endianSwapActions[g_actionsQuantity] = (endianSwap == NULL) ? noSwapEndian : endianSwap;
      g_actionSizes[g_actionsQuantity] = actionSize;
      ++g_actionsQuantity;
      return;
   }
   // g_actionUIDsAllocated * 0.625. Maximum effective load factor for this type of hash table is 0.7
   if (g_actionsQuantity > (g_actionsAllocated >> 1 | g_actionsAllocated >> 3))
      resizeHashTable(g_actionsAllocated << 1);
   uint32_t hash;
   ADD_TO_HASH_TABLE(actionUID, hash);
   g_actions[hash] = action;
   g_endianSwapActions[hash] = (endianSwap == NULL) ? noSwapEndian : endianSwap;
   g_actionSizes[hash] = actionSize;
   ++g_actionsQuantity;
}

// Expects POINTERS to actions, not actions themselves
CCE_API void* cceaRunActionsCreateDynamic (uint32_t runActionsUID, uint32_t runActionsStructSize, uint32_t actionsQuantity, ...)
{
   va_list args, argcp;
   va_start(args, actionsQuantity);
   va_copy(argcp, args);
   size_t totalSize = runActionsStructSize;
   uint32_t *actionsSize = malloc(actionsQuantity * sizeof(uint32_t));
   for (uint32_t i = 0; i < actionsQuantity; ++i)
   {
      struct cceaDynamicAction *action = va_arg(argcp, void*);
      uint32_t id;
      UID_TO_ID(action->UID, id);
      actionsSize[i] = (g_actionSizes[id] == 0) ? action->size : g_actionSizes[id];
      totalSize += actionsSize[i];
   }
   va_end(argcp);
   cce_void *runActions = malloc(totalSize);
   *(uint32_t*)runActions = runActionsUID;
   *(uint32_t*)(runActions + sizeof(uint32_t)) = totalSize;
   cce_void *pos = runActions + runActionsStructSize;
   for (uint32_t i = 0; i < actionsQuantity; pos += actionsSize[i++])
   {
      struct cceaDynamicAction *action = va_arg(args, void*);
      memcpy(pos, action, actionsSize[i]);
   }
   va_end(args);
   return runActions;
}

CCE_API void cceaRegisterEvent (uint32_t eventUID)
{
   if (g_eventUIDsQuantity >= g_eventUIDsAllocated)
   {
      CCE_CEIL_TO_POWER_OF_TWO(g_eventUIDsQuantity + 1, g_eventUIDsAllocated);
      CCE_REALLOC_UID_ARRAY(g_eventUIDs, g_eventUIDsSorted, g_eventUIDsQuantity, g_eventUIDsAllocated);
   }
   CCE_INSERT_INTO_UID_ARRAY(eventUID, g_eventUIDs, g_eventUIDsSorted, g_eventUIDsQuantity);
   ++g_eventUIDsQuantity;
}

CCE_API void cceaInvokeEvent (uint32_t eventUID, uint32_t count, struct cce_buffer *map)
{
   struct ccea_actioninfo *actionInfo = (struct ccea_actioninfo*)CCE_GET_FUNCTION_BUFFER(map, cceaPluginUID);
   uint32_t eventID;
   CCE_FIND_FROM_UID_ARRAY(eventUID, g_eventUIDs, g_eventUIDsSorted, g_eventUIDsQuantity, eventID, 
                           fprintf(stderr, "ENGINE::ACTIONS_PLUGIN::EVENT_NOT_FOUND:\nCan't find event %s (uid: %u)\n", cceUIDToName(eventUID), eventUID); return);
   ccea__runActions(actionInfo->onEventActions[eventID].data, actionInfo->onEventActions[eventID].dataQuantity, count, map);
}

CCE_API void ccea__addActionsOnEvent (uint32_t eventUID, uint32_t actionSubsUID, uint32_t totalActionsSize, struct cceaAction *actions, struct cceaActionRunner *wrapper, struct cce_buffer *map)
{
   struct ccea_actioninfo *actionInfo = (struct ccea_actioninfo*)CCE_GET_FUNCTION_BUFFER(map, cceaPluginUID);
   uint32_t eventID;
   CCE_FIND_FROM_UID_ARRAY(eventUID, g_eventUIDs, g_eventUIDsSorted, g_eventUIDsQuantity, eventID, 
                           fprintf(stderr, "ENGINE::ACTIONS_PLUGIN::EVENT_NOT_FOUND:\nCan't find event %s (uid: %u)\n", cceUIDToName(eventUID), eventUID); return);
   if (actionInfo->actionSubsUIDs[eventID].dataQuantity >= actionInfo->actionSubsUIDs[eventID].dataAllocated)
      CCE_REALLOC_ARRAY(actionInfo->actionSubsUIDs[eventID].data, actionInfo->actionSubsUIDs[eventID].dataQuantity + 1);
   struct cce_actionsResizable *actionsOnEvent = &actionInfo->onEventActions[eventID];
   uint32_t size = totalActionsSize + (wrapper == NULL ? 0 : wrapper->totalSize);
   if (actionsOnEvent->dataQuantity + size / sizeof(struct cceaAction)  > actionsOnEvent->dataAllocated)
      CCE_REALLOC_ARRAY(actionsOnEvent->data, actionsOnEvent->dataQuantity + size / sizeof(struct cceaAction));
   actionInfo->actionSubsUIDs[eventID].data[actionInfo->actionSubsUIDs[eventID].dataQuantity++] = actionSubsUID;
   cce_void *it = ((cce_void*)actionsOnEvent->data) + actionsOnEvent->dataQuantity;
   if (wrapper != NULL)
   {
      memcpy(it, wrapper, wrapper->totalSize);
      ((struct cceaActionRunner*)it)->totalSize += totalActionsSize;
      it += wrapper->totalSize;
   }
   memcpy(it, actions, totalActionsSize);
   actionsOnEvent->dataQuantity += size;
}

CCE_API void cceaAddActionOnEvent (uint32_t eventUID, uint32_t actionSubsUID, struct cceaAction *action, struct cce_buffer *map)
{
   uint32_t actionID;
   UID_TO_ID(action->UID, actionID);
   uint32_t size = (g_actionSizes[actionID] == 0) ? ((struct cceaDynamicAction*)action)->size : g_actionSizes[actionID];
   ccea__addActionsOnEvent(eventUID, actionSubsUID, size, action, NULL, map);
}

CCE_API void cceaRemoveActionOnEvent (uint32_t eventUID, uint32_t actionSubsUID, struct cce_buffer *map)
{
   struct ccea_actioninfo *actionInfo = (struct ccea_actioninfo*)CCE_GET_FUNCTION_BUFFER(map, cceaPluginUID);
   uint32_t eventID;
   CCE_FIND_FROM_UID_ARRAY(eventUID, g_eventUIDs, g_eventUIDsSorted, g_eventUIDsQuantity, eventID, 
                           fprintf(stderr, "ENGINE::ACTIONS_PLUGIN::EVENT_NOT_FOUND:\nCan't find event %s (uid: %u)\n", cceUIDToName(eventUID), eventUID); return);
   if (actionInfo->actionSubsUIDs[eventID].dataQuantity == 0)
      return;
   // Linear search. The next search is linear anyway, but even if it would not, we still need to resize array which is again linear. The only real exit is making binary tree
   uint32_t actionSubsID = 0;
   while (actionInfo->actionSubsUIDs[eventID].data[actionSubsID] != actionSubsUID)
   {
      ++actionSubsID;
      if (actionSubsID >= actionInfo->actionSubsUIDs[eventID].dataQuantity)
         return;
   }
   memmove(actionInfo->actionSubsUIDs[eventID].data + actionSubsID, actionInfo->actionSubsUIDs[eventID].data + actionSubsID + 1, --actionInfo->actionSubsUIDs[eventID].dataQuantity - actionSubsID);
   uint32_t offset = 0;
   struct cce_actionsResizable *actions = &actionInfo->onEventActions[eventID];
   for (uint32_t i = 0; i < actionSubsID; ++i)
   {
      struct cceaDynamicAction *action = (struct cceaDynamicAction*)(((cce_void*)actions->data) + offset);
      uint32_t actionID;
      UID_TO_ID(action->UID, actionID);
      offset += g_actionSizes[actionID] == 0 ? action->size : g_actionSizes[actionID];
   }
   struct cceaDynamicAction *action = (struct cceaDynamicAction*)(((cce_void*)actions->data) + offset);
   uint32_t actionID;
   UID_TO_ID(action->UID, actionID);
   uint32_t size = g_actionSizes[actionID] == 0 ? action->size : g_actionSizes[actionID];
   memmove(((cce_void*)actions->data) + offset, ((cce_void*)actions->data) + offset + size, (actions->dataQuantity -= size) - offset);
}

CCE_API void ccea__delayDynamicAction (uint32_t UID, uint32_t timeout, void *data, uint32_t dataSize, struct cce_buffer *map)
{
   struct ccea_actioninfo *actionInfo = (struct ccea_actioninfo*)CCE_GET_FUNCTION_BUFFER(map, cceaPluginUID);
   struct cceaDelayedDynamicAction *action = llnodealloc(sizeof(struct cceaDelayedDynamicAction) + dataSize, LL_SINGLELINKED);
   action->UID = UID;
   action->timeout = timeout;
   action->size = sizeof(struct cceaDelayedDynamicAction) + dataSize;
   memcpy(action + 1, data, dataSize);
   llappendchain(&actionInfo->delayedActions, action);
}

CCE_API void cceaRunDelayedActions (struct cce_buffer *map)
{
   struct ccea_actioninfo *actionInfo = (struct ccea_actioninfo*)CCE_GET_FUNCTION_BUFFER(map, cceaPluginUID);
   uint32_t currentTime = (actionInfo->currentMapTime += cceGetFrameDeltaTime());
   struct list *delayedActions = &actionInfo->delayedActions;
   // TODO: sort delayedActions according to remaining time
   void *node = delayedActions->chain, *prevnode = NULL;
   while (node != NULL)
   {
      struct cceaDelayedDynamicAction *action = node;
      if (cceIsTimeout(currentTime, action->timeout))
      {
         EXEC_ACTION(action, 1, map);
         if (cceIsTimeout(currentTime, action->timeout))
         {
            node = LL_NEXT(node);
            llnodefree(llpullnodep(delayedActions, prevnode), LL_SINGLELINKED);
            continue;
         }
      }
      prevnode = node;
      node = LL_NEXT(node);
   }
}

int loadActions (void *buffer, uint16_t sectionSize, struct cce_buffer *info, FILE *file)
{
   struct ccea_actioninfo *map = buffer;
   fread(&map->currentMapTime, sizeof(uint32_t), 1, file);
   // 8K of stack memory! May not run well on some platforms with small stack sizes.
   uint32_t uids[0xFF];
   fread(uids, sizeof(uint32_t), sectionSize, file);
   map->actionSubsUIDs = calloc(g_eventUIDsQuantity, sizeof(struct cce_uidsResizable));
   for (uint32_t i = 0; i < sectionSize; ++i)
   {
      uint32_t size, eventID;
      fread(&size, sizeof(uint32_t), 1, file);
      CCE_FIND_FROM_UID_ARRAY(uids[i], g_eventUIDs, g_eventUIDsSorted, g_eventUIDsQuantity, eventID, 
                              fprintf(stderr, "ENGINE::ACTIONS_PLUGIN::EVENT_NOT_FOUND:\nCan't find event %s (uid: %u)\n", cceUIDToName(uids[i]), uids[i]); return -1);
      uids[i] = eventID;
      map->actionSubsUIDs[eventID].dataAllocated = (map->actionSubsUIDs[eventID].dataQuantity = size);
      if (size == 0)
      {
         map->actionSubsUIDs[eventID].data = NULL;
         continue;
      }
      map->actionSubsUIDs[eventID].data = malloc(size * sizeof(uint32_t));
      fread(map->actionSubsUIDs[eventID].data, sizeof(uint32_t), size, file);
   }
   map->onEventActions = calloc(g_eventUIDsQuantity, sizeof(struct cce_actionsResizable));
   map->delayedActions = LL_LIST_INIT(LL_SINGLELINKED);
   for (uint32_t i = 0; i < sectionSize; ++i)
   {
      uint32_t size;
      uint32_t UID = 0;
      fread(&size, sizeof(uint32_t), 1, file);
      if (g_eventUIDs[i] == cceaBasicEventsUIDs[0] && (fread(&UID, sizeof(uint32_t), 1, file), UID) == cceaBasicActionUIDs[CCEA_APPEND_LIST_OF_RUNDELAYED_ACTIONS])
      {
         uint32_t delayedActionsSize;
         fread(&delayedActionsSize, sizeof(uint32_t), 1, file);
         struct cceaDynamicAction *actions = malloc(delayedActionsSize);
         actions->UID = UID;
         actions->size = delayedActionsSize;
         fread(actions + 1, delayedActionsSize - sizeof(struct cceaAppendListOfRunDelayedActions), 1, file);
         EXEC_ACTION(actions, 1, info);
         size -= delayedActionsSize;
         map->onEventActions[uids[i]].dataAllocated = map->onEventActions[uids[i]].dataQuantity = size;
         if (size == 0 || size > delayedActionsSize)
         {
            free(actions);
            if (size == 0)
               continue;
            map->onEventActions[uids[i]].data = malloc(size);
         }
         else
         {
            map->onEventActions[uids[i]].data = realloc(actions, size);
         }
         fread(map->onEventActions[uids[i]].data, size, 1, file);
         continue;
      }
      if (size == 0)
      {
         map->onEventActions[uids[i]].dataAllocated = map->onEventActions[uids[i]].dataQuantity = size;
         map->onEventActions[uids[i]].data = NULL;
         continue;
      }
      if (UID != 0)
      {
         size -= sizeof(uint32_t);
         fseek(file, -sizeof(uint32_t), SEEK_CUR);
      }
      map->onEventActions[uids[i]].dataAllocated = map->onEventActions[uids[i]].dataQuantity = size;
      map->onEventActions[uids[i]].data = malloc(size);
      fread(map->onEventActions[uids[i]].data, size, 1, file);
   }
   map->eventsQuantity = g_eventUIDsQuantity;
   cceaInvokeEvent(cceaBasicEventsUIDs[0], 1, info);
   return 0;
}

void createActions (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   struct ccea_actioninfo *map = buffer;
   map->delayedActions = LL_LIST_INIT(LL_SINGLELINKED);
   map->eventsQuantity = g_eventUIDsQuantity;
   map->actionSubsUIDs = calloc(g_eventUIDsQuantity, sizeof(struct cce_uidsResizable));
   map->onEventActions = calloc(g_eventUIDsQuantity, sizeof(struct cce_actionsResizable));
   map->currentMapTime = 0;
}

void freeActions (void *buffer, struct cce_buffer *info)
{
   cceaInvokeEvent(g_eventUIDs[1], 1, info);
   struct ccea_actioninfo *map = buffer;
   llrmlist(&map->delayedActions);
   for (uint16_t i = 0; i < map->eventsQuantity; ++i)
   {
      free(map->actionSubsUIDs[i].data);
      free(map->onEventActions[i].data);
   }
   free(map->actionSubsUIDs);
   free(map->onEventActions);
}

uint16_t storeActions (void *buffer, struct cce_buffer *info, FILE *file)
{
   struct ccea_actioninfo *map = buffer;
   cceaInvokeEvent(cceaBasicEventsUIDs[2], 1, info);
   fwrite(&map->currentMapTime, sizeof(uint32_t), 1, file);
   uint16_t eventsQuantity = 0, i = 0;
   for (struct cce_actionsResizable *it = map->onEventActions; i < map->eventsQuantity; ++it, ++i)
   {
      if (it->dataQuantity == 0 && !(g_eventUIDs[i] == cceaBasicEventsUIDs[CCEA_EVENT_LOAD] && map->delayedActions.size > 0))
         continue;
      ++eventsQuantity;
      fwrite(&g_eventUIDs[i], sizeof(uint32_t), 1, file);
   }
   for (i = 0; i < map->eventsQuantity; ++i)
   {
      if (map->actionSubsUIDs[i].dataQuantity == 0 && !(g_eventUIDs[i] == cceaBasicEventsUIDs[CCEA_EVENT_LOAD] && map->delayedActions.size > 0))
         continue;
      fwrite(&map->actionSubsUIDs[i].dataQuantity, sizeof(uint32_t), 1, file);
      fwrite(map->actionSubsUIDs[i].data, sizeof(uint32_t), map->actionSubsUIDs[i].dataQuantity, file);
   }
   for (i = 0; i < map->eventsQuantity; ++i)
   {
      if (g_eventUIDs[i] == cceaBasicEventsUIDs[CCEA_EVENT_LOAD] && map->delayedActions.size > 0)
      {
         uint32_t totalSize = sizeof(struct cceaAppendListOfRunDelayedActions);
         fseek(file, sizeof(uint32_t) + sizeof(struct cceaAppendListOfRunDelayedActions), SEEK_CUR);
         for (struct cceaDelayedDynamicAction *node = map->delayedActions.chain; node != NULL; node = LL_NEXT(node))
         {
            uint32_t ID;
            UID_TO_ID(node->UID, ID);
            uint32_t size = g_actionSizes[ID] == 0 ? node->size : g_actionSizes[ID];
            fwrite(node, size, 1, file);
            totalSize += size;
         }
         fseek(file, -(int32_t)sizeof(uint32_t) - ((int32_t)totalSize), SEEK_CUR);
         {
            uint32_t size = totalSize + map->onEventActions[i].dataQuantity;
            fwrite(&size, sizeof(uint32_t), 1, file);
         }
         fwrite(&cceaBasicActionUIDs[CCEA_APPEND_LIST_OF_RUNDELAYED_ACTIONS], sizeof(uint32_t), 1, file);
         fwrite(&totalSize, sizeof(uint32_t), 1, file);
         fseek(file, totalSize - sizeof(struct cceaAppendListOfRunDelayedActions), SEEK_CUR);
         if (map->onEventActions[i].dataQuantity == 0)
            continue;
      }
      else
      {
         if (map->onEventActions[i].dataQuantity == 0)
            continue;
         fwrite(&map->onEventActions[i].dataQuantity, sizeof(uint32_t), 1, file);
      }
      fwrite(map->onEventActions[i].data, map->onEventActions[i].dataQuantity, 1, file);
   }
   return eventsQuantity;
}

static int initActions (void *data)
{
   CCE_UNUSED(data);
   g_endianSwapActions = (void (**)(void*)) calloc(g_actionsQuantity, sizeof(void (*)(void*)));
   cceaBasicActionUIDs[CCEA_RUN_ACTIONS] = cceNameToUID("ccern");
   cceaBasicActionUIDs[CCEA_RUN_ACTIONS_ONCE] = cceNameToUID("ccern1t");
   cceaBasicActionUIDs[CCEA_RUN_ACTIONS_N_TIMES] = cceNameToUID("ccernnt");
   
   cceaBasicActionUIDs[CCEA_DELAY_ACTIONS] = cceNameToUID("ccedl");
   cceaBasicActionUIDs[CCEA_DELAY_ACTIONS_PERIODIC] = cceNameToUID("ccedlpr");
   cceaBasicActionUIDs[CCEA_DELAY_ACTIONS_REPEATED] = cceNameToUID("ccedlrp");
   
   cceaBasicActionUIDs[CCEA_RUN_DELAYED_ACTIONS] = cceNameToUID("ccerndl");
   cceaBasicActionUIDs[CCEA_RUN_PERIODIC_ACTIONS] = cceNameToUID("ccerndp");
   cceaBasicActionUIDs[CCEA_RUN_REPEATED_ACTIONS] = cceNameToUID("ccerndr");
   
   cceaBasicActionUIDs[CCEA_APPEND_LIST_OF_RUNDELAYED_ACTIONS] = cceNameToUID("ccealrd");
   cceaBasicActionUIDs[CCEA_TERMINATE_ENGINE] = cceNameToUID("cceterm");
   
   cceaBasicActionUIDs[CCEA_ADD_ACTIONS_ON_EVENT] = cceNameToUID("cceaaoe");
   cceaBasicActionUIDs[CCEA_REMOVE_ACTIONS_ON_EVENT] = cceNameToUID("cceraoe");
   cceaBasicActionUIDs[CCEA_INVOKE_EVENT] = cceNameToUID("cceinve");
   
   cceaRegisterAction(cceaBasicActionUIDs[CCEA_RUN_ACTIONS],                       runActions,                   runActionsActionSwapEndian, CCEA_ACTION_SIZE_VARIABLE);
   cceaRegisterAction(cceaBasicActionUIDs[CCEA_RUN_ACTIONS_ONCE],                  runActionsOnce,               runActionsActionSwapEndian, CCEA_ACTION_SIZE_VARIABLE);
   cceaRegisterAction(cceaBasicActionUIDs[CCEA_RUN_ACTIONS_N_TIMES],               runActionsNTimes,             runActionsSwapEndianInt1,   CCEA_ACTION_SIZE_VARIABLE);
   cceaRegisterAction(cceaBasicActionUIDs[CCEA_DELAY_ACTIONS],                     delayActions,                 runActionsSwapEndianInt1,   CCEA_ACTION_SIZE_VARIABLE);
   cceaRegisterAction(cceaBasicActionUIDs[CCEA_DELAY_ACTIONS_PERIODIC],            delayActionsPeriodic,         runActionsSwapEndianInt1,   CCEA_ACTION_SIZE_VARIABLE);
   cceaRegisterAction(cceaBasicActionUIDs[CCEA_DELAY_ACTIONS_REPEATED],            delayActionsRepeated,         runActionsSwapEndianInt2,   CCEA_ACTION_SIZE_VARIABLE);
   cceaRegisterAction(cceaBasicActionUIDs[CCEA_RUN_DELAYED_ACTIONS],               runDelayedActions,            runActionsSwapEndianInt1,   CCEA_ACTION_SIZE_VARIABLE);
   cceaRegisterAction(cceaBasicActionUIDs[CCEA_RUN_PERIODIC_ACTIONS],              runPeriodicActions,           runActionsSwapEndianInt2,   CCEA_ACTION_SIZE_VARIABLE);
   cceaRegisterAction(cceaBasicActionUIDs[CCEA_RUN_REPEATED_ACTIONS],              runRepeatedlyDelayedActions,  runActionsSwapEndianInt3,   CCEA_ACTION_SIZE_VARIABLE);
   cceaRegisterAction(cceaBasicActionUIDs[CCEA_APPEND_LIST_OF_RUNDELAYED_ACTIONS], appendListOfRunDelayedAction, runActionsActionSwapEndian, CCEA_ACTION_SIZE_VARIABLE);
   cceaRegisterAction(cceaBasicActionUIDs[CCEA_ADD_ACTIONS_ON_EVENT],              addActionsOnEvent,            runActionsSwapEndianInt2,   CCEA_ACTION_SIZE_VARIABLE);
   cceaRegisterAction(cceaBasicActionUIDs[CCEA_REMOVE_ACTIONS_ON_EVENT],           removeActionsOnEvent,         swapEndianInt2,             sizeof(struct cceaRemoveActionOnEvent));
   cceaRegisterAction(cceaBasicActionUIDs[CCEA_INVOKE_EVENT],                      invokeEvent,                  swapEndianInt1,             sizeof(struct cceaInvokeEvent));
   cceaRegisterAction(cceaBasicActionUIDs[CCEA_TERMINATE_ENGINE],                  terminateEngine,              NULL,                       sizeof(uint32_t));
   
   cceaBasicEventsUIDs[0] = cceNameToUID("cceload");
   cceaBasicEventsUIDs[1] = cceNameToUID("ccefree");
   cceaBasicEventsUIDs[2] = cceNameToUID("ccesave");
   cceaRegisterEvent(cceaBasicEventsUIDs[0]);
   cceaRegisterEvent(cceaBasicEventsUIDs[1]);
   cceaRegisterEvent(cceaBasicEventsUIDs[2]);
   return 0;
}

static int postInitActions (void)
{
   resizeHashTable(g_actionsAllocated << (g_actionsQuantity > (g_actionsAllocated >> 1 | g_actionsAllocated >> 3)));
   g_flags &= ~CCE_ACTIONS_INITIALIZING;
   return 0;
}

static void terminateActions (void)
{
   free(g_actions);
   free(g_endianSwapActions);
}

CCE_API void cceaLoadActionsPlugin (void)
{
   if (cceaPluginUID == 0)
      cceaPluginUID = cceNameToUID("cceacde");
   cceRegisterPlugin(cceaPluginUID, NULL, NULL, initActions, postInitActions, terminateActions, 0);
   g_flags |= CCE_ACTIONS_INITIALIZING;
}

CCE_API void cceaRegisterActionsFileIOFunctions (uint16_t functionSetID)
{
   cceRegisterFileIOcallbacks(functionSetID, cceaPluginUID, loadActions, freeActions, createActions, storeActions, sizeof(struct ccea_actioninfo));
}
