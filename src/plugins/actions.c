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

#include <listlib.h>

#include "../../include/cce/engine_common.h"
#include "../../include/cce/engine_common_IO.h"
#include "../../include/cce/utils.h"
#include "../../include/cce/endianess.h"
#include "../../include/cce/plugins/actions.h"

#define CCE_TIMER_START 0x1
#define CCE_TIMER_STOP  0x2
#define CCE_TIMER_START_STOP 0x3
#define CCE_TIMER_AUTO_RESTART_ON_ALARM 0x4

#define CCE_PROCESS_TIMERS              0x040

struct DelayedAction
{
   uint64_t initTime;
   uint32_t delay;
   uint16_t repeatsLeft;
   uint8_t  flags;
};

CCE_ARRAY(actions, static cce_actionfun, static uint32_t);
static void (**endianSwapActions)(void*);
static struct DelayedAction *nextDelayedActionWithoutExternalTimer;
static struct DelayedAction *lastDelayedActionWithExternalTimer;
static struct DelayedAction *firstDelayedActionWithVerySmallDelay;
static struct DelayedAction *lastDelayedActionWithVerySmallDelay;
static struct list delayedActions;
static uint32_t delayedActionsWithExternalTimerQuantity;
static uint32_t delayedActionsWithVerySmallDelayQuantity;
static struct cce_timer *nextTimer;
CCE_ARRAY(timers, static struct cce_timer, static uint16_t);
uint64_t currentTime;
uint8_t g_flags;

#define EXEC_ACTION(action, count)(*((actions) + *(uint32_t*)(action)))(action, count)

struct Action
{
   uint32_t ID;
};

static void runActionsAction (const void *data, uint16_t count)
{
   const struct cceRunActions *params = data;
   const cce_void *actionsToCall = (const cce_void*) data + sizeof(struct cceRunActions) + (((params->actionQuantity - 1) | 1) - 1) * sizeof(uint16_t);
   for (const uint16_t *sizes = params->actionSizes, *end = params->actionSizes + params->actionQuantity; sizes < end; actionsToCall += *sizes++)
   {
      EXEC_ACTION(actionsToCall, count);
   }
}

static void setTimerStateAction (const void *data, uint16_t count)
{
   const struct cceSetTimerStateAction *params = data;
   uint8_t mask = -(count & 1);
   mask &= ((((params->state & CCE_CHANGETIMERSTATE_SWITCH) + 1) & (CCE_CHANGETIMERSTATE_SWITCH + 1)) - 1) | (params->state & CCE_CHANGETIMERSTATE_SWITCH_AUTO_RESTART_ON_ALARM);
   // If switch happens twice, it cancels out
   cceSetTimerState(params->ID, params->state & ~mask);
}

static void setTimerDelayAction (const void *data, uint16_t count)
{
   const struct cceSetTimerDelayAction *params = data;
   cceSetTimerDelay(params->ID, params->delay * ((count & -(params->action == CCE_ACTION_SHIFT)) + params->action != CCE_ACTION_SHIFT), params->action);
}

static void delayActionAction (const void *data, uint16_t count)
{
   const struct cceDelayAction *params = data;
   if (count == 1)
      cceDelayAction(params->timerInfo.delay, params->repeatsQuantity, params->delayedActionStructSize, ((void*) (params + 1)), params->flags);
   else
      cceDelayAction(params->timerInfo.delay / count, params->repeatsQuantity, params->delayedActionStructSize, ((void*) (params + 1)), params->flags); // Integer division is painfully slow
}

static void setEngineShouldTerminateAction (const void *data, uint16_t count)
{
   const struct cceSetEngineShouldTerminate *params = data;
   cceSetEngineShouldTerminate(((cceEngineShouldTerminate() != !(count & 1)) & (params->action == CCE_ACTION_SHIFT)) != params->state);
}

static void runActionsActionSwapEndian (void *data)
{
   struct cceRunActions *params = data;
   params->actionQuantity = cceSwapEndianInt16(params->actionQuantity);
   cceSwapEndianArrayIntN(params->actionSizes, params->actionQuantity - 2, sizeof(uint16_t));
   cce_void *actionsToCall = (cce_void*) data + sizeof(struct cceRunActions) + (((params->actionQuantity - 1) | 1) - 1) * sizeof(uint16_t);
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
   struct cceSetTimerStateAction *params = data;
   params->ID = cceSwapEndianInt16(params->ID);
}

static void setTimerDelayActionSwapEndian (void *data)
{
   struct cceSetTimerDelayAction *params = data;
   params->ID    = cceSwapEndianInt16(params->ID);
   params->delay = cceSwapEndianInt32(params->delay);
}

static void delayActionActionSwapEndian (void *data)
{
   struct cceDelayAction *params = data;
   params->delayedActionStructSize = cceSwapEndianInt32(params->delayedActionStructSize);
   params->timerInfo.delay = cceSwapEndianInt32(params->timerInfo.delay);
   params->repeatsQuantity = cceSwapEndianInt32(params->repeatsQuantity);
   (*(endianSwapActions + params->actionID))((void*) (params + 1));
}

void cce__swapActionsEndian (uint16_t *actionSizes, void *actionsToSwap, uint16_t actionsToSwapQuantity)
{
   uint16_t *iterator = actionSizes, *end = actionSizes + actionsToSwapQuantity;
   cce_void *jiterator = actionsToSwap;
   while (iterator < end)
   {
      *(uint32_t*)jiterator = cceSwapEndianInt32(*(uint32_t*)jiterator);
      if (*(endianSwapActions + *(uint32_t*)jiterator) != NULL)
         (*(endianSwapActions + *(uint32_t*)jiterator))(jiterator);
      jiterator += *iterator;
   }
}

void cce__runActions (uint16_t *actionSizes, void *actionsToCall, uint16_t actionsToCallQuantity)
{
   uint16_t *iterator = actionSizes, *end = actionSizes + actionsToCallQuantity;
   cce_void *jiterator = actionsToCall;
   while (iterator < end)
   {
      EXEC_ACTION(jiterator, 1);
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
   if (*(endianSwapActions + ((struct Action*)tmpActionStore)->ID) != NULL) \
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

void cce__actionsTerminate (void)
{
   llrmlist(&delayedActions);
   free(actions);
   free(endianSwapActions);
   free(timers);
}

static int initActions (void *data)
{
   currentTime = cceGetTime();
   CCE_UNUSED(data);
   actionsQuantity = 1 * 2;
   timersQuantity = 0;
   CCE_ALLOC_ARRAY_ZEROED(timers, 1);
   CCE_ALLOC_ARRAY_ZEROED(actions, 1);
   delayedActionsWithExternalTimerQuantity = 0;
   delayedActionsWithVerySmallDelayQuantity = 0;
   delayedActions = LL_LIST_INIT(LL_SINGLELINKED);
   endianSwapActions = (void (**)(void*)) calloc(actionsQuantity, sizeof(void (*)(void*)));
   nextTimer = NULL;
   nextDelayedActionWithoutExternalTimer = NULL;
   lastDelayedActionWithExternalTimer = NULL;
   firstDelayedActionWithVerySmallDelay = NULL;
   lastDelayedActionWithVerySmallDelay = NULL;
   cceRegisterAction(CCE_ACTION_DELAYACTION,              delayActionAction,              delayActionActionSwapEndian);
   cceRegisterAction(CCE_ACTION_RUNACTIONS,               runActionsAction,               runActionsActionSwapEndian);
   cceRegisterAction(CCE_ACTION_STARTTIMER,               setTimerStateAction,            setTimerStateActionSwapEndian);
   cceRegisterAction(CCE_ACTION_SETTIMERDELAY,            setTimerDelayAction,            setTimerDelayActionSwapEndian);
   cceRegisterAction(CCE_ACTION_SETENGINESHOULDTERMINATE, setEngineShouldTerminateAction, NULL);
   return 0;
}

static void terminateActions (void)
{
   free(timers);
   free(actions);
   free(endianSwapActions);
   timersQuantity   = 0;
   timersAllocated  = 0;
   actionsQuantity  = 0;
   actionsAllocated = 0;
   llrmlist(&delayedActions);
}

CCE_API uint8_t cceRegisterAction (uint32_t ID, cce_actionfun action, void (*endianSwap)(void*))
{
   if (ID >= actionsAllocated) do
   {
      CCE__REALLOC_ARRAY(actions, ID + 1);
      endianSwapActions = realloc(endianSwapActions, actionsAllocated * sizeof(cce_actionfun*));
      memset(actions           + oldAllocated, 0, (actionsAllocated - oldAllocated) * sizeof(cce_actionfun));
      memset(endianSwapActions + oldAllocated, 0, (actionsAllocated - oldAllocated) * sizeof(cce_actionfun));
   }
   while (0);
   actions[ID] = action;
   endianSwapActions[ID] = endianSwap;
   actionsQuantity = CCE_MAX(ID + 1, actionsQuantity + 1);
   return 0;
}

CCE_API void cceSetTimerState (uint16_t timerID, uint8_t state)
{
   if (timerID >= timersAllocated)
   {
      CCE_REALLOC_ARRAY_ZEROED(timers, timerID + 1);
   }
   timers[timerID].flags &= ~(CCE_TIMER_START_STOP | ((state & CCE_CHANGETIMERSTATE_DISABLE_AUTO_RESTART_ON_ALARM) >> 1));
   timers[timerID].flags |= (state & CCE_CHANGETIMERSTATE_SWITCH) ^ (((state & CCE_CHANGETIMERSTATE_SWITCH) == CCE_CHANGETIMERSTATE_SWITCH) << (timers[timerID].initTime == 0));
   timers[timerID].flags |= state & CCE_CHANGETIMERSTATE_ENABLE_AUTO_RESTART_ON_ALARM;
   timers[timerID].flags ^= ((state & CCE_CHANGETIMERSTATE_SWITCH_AUTO_RESTART_ON_ALARM) >> 2);
   g_flags |= CCE_PROCESS_TIMERS;
}

CCE_API void cceSetTimerDelay (uint16_t timerID, uint32_t newDelay, uint8_t actionType)
{
   if (timerID >= timersAllocated)
   {
      CCE_REALLOC_ARRAY_ZEROED(timers, timerID + 1);
   }
   if (actionType == CCE_ACTION_SHIFT)
   {
      newDelay += timers[timerID].delay;
   }
   if (timers[timerID].initTime + timers[timerID].delay >= currentTime)
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

CCE_API void cceDelayAction (uint16_t repeatsQuantity, uint32_t delayOrID, uint32_t actionStructSize, void *actionStruct, uint8_t flags)
{
   if (flags & CCE_DELAYACTION_EXECUTE_ON_START)
      EXEC_ACTION(actionStruct, 1);
   
   cce_void *data = llnodealloc(sizeof(struct DelayedAction) + actionStructSize, delayedActions.type);
   LL_NEXT(data) = NULL;
   struct DelayedAction *head = (struct DelayedAction*) data;
   if (flags & CCE_DELAYACTION_NEVER_END)
      head->repeatsLeft = UINT16_MAX;
   else
      head->repeatsLeft = repeatsQuantity;
   head->delay = delayOrID;
   if (((flags & (CCE_DELAYACTION_EXTERNAL_TIMER | CCE_DELAYACTION_START_EXTERNAL_TIMER)) == (CCE_DELAYACTION_EXTERNAL_TIMER | CCE_DELAYACTION_START_EXTERNAL_TIMER)))
      timers[delayOrID].flags |= CCE_TIMER_START;
   
   head->initTime = cceGetTime();
   head->flags = flags & (CCE_DELAYACTION_EXTERNAL_TIMER | CCE_DELAYACTION_NEVER_END | CCE_DELAYACTION_EXECUTE_ONCE_PER_TIMER_ALARM | CCE_DELAYACTION_RESTART_EXTERNAL_TIMER_ON_ALARM);
   memcpy(data + sizeof(struct DelayedAction), actionStruct, actionStructSize);
   if (flags & CCE_DELAYACTION_EXTERNAL_TIMER)
   {
      llprependchain(&delayedActions, data);
      if (delayedActionsWithExternalTimerQuantity == 0)
         lastDelayedActionWithExternalTimer = head;
      ++delayedActionsWithExternalTimerQuantity;
      return;
   }
   if (head->delay < cceGetDeltaTime() * 2)
   {
      llinsertchainp(&delayedActions, lastDelayedActionWithExternalTimer, data);
      if (delayedActionsWithVerySmallDelayQuantity == 0)
         lastDelayedActionWithVerySmallDelay = head;
      firstDelayedActionWithVerySmallDelay = head;
      ++delayedActionsWithVerySmallDelayQuantity;
      return;
   }
   if (nextDelayedActionWithoutExternalTimer == NULL || head->initTime + head->delay < nextDelayedActionWithoutExternalTimer->initTime + nextDelayedActionWithoutExternalTimer->delay)
   {
      llinsertchainp(&delayedActions, (lastDelayedActionWithVerySmallDelay == NULL) ? lastDelayedActionWithExternalTimer : lastDelayedActionWithVerySmallDelay, data);
      nextDelayedActionWithoutExternalTimer = head;
      return;
   }
   llappendchain(&delayedActions, data);
}

// Uses cache inefficiently (linked lists are bad at this). But arrays are out of luck here because actions have variable-size structs (making them hard to use with arrays)
static void runDelayedActions (void)
{
   cce_void *prevnode = NULL, *node = delayedActions.chain;
   struct DelayedAction *head;
   uint32_t i = 0;
   int64_t timerDiff;
   uint16_t count;
   // If no timers expired, no need to check actions dependant on them
   if (nextTimer != NULL && (nextTimer->initTime + nextTimer->delay <= currentTime))
   {
      while (i < delayedActionsWithExternalTimerQuantity)
      {
         head = (struct DelayedAction*)node;
         struct cce_timer *timer = timers + head->delay;
         if (timer->initTime == 0 || (timerDiff = currentTime - (timer->initTime + timer->delay)) < 0)
            goto CONTINUE;
         count = 1;
         if ((uint32_t)timerDiff >= timer->delay && !(head->flags & CCE_DELAYACTION_EXECUTE_ONCE_PER_TIMER_ALARM) &&
            (timer->flags & CCE_TIMER_AUTO_RESTART_ON_ALARM || head->flags & CCE_DELAYACTION_RESTART_EXTERNAL_TIMER_ON_ALARM) &&
            timer->delay != 0)
         {
            count += timerDiff / timer->delay;
         }
         count = CCE_MIN(count, head->repeatsLeft);
         EXEC_ACTION(node + sizeof(struct DelayedAction), count);
         head->repeatsLeft = (head->flags & CCE_DELAYACTION_NEVER_END) ? head->repeatsLeft : head->repeatsLeft - count;
         if (head->repeatsLeft == 0)
         {
            node = LL_NEXT(node);
            llrmsublistp(&delayedActions, prevnode, 1);
            --delayedActionsWithExternalTimerQuantity;
            continue;
         }
         if (head->flags & CCE_DELAYACTION_RESTART_EXTERNAL_TIMER_ON_ALARM)
         {
            timer->flags |= CCE_TIMER_START;
         }
CONTINUE:
         prevnode = node;
         node = LL_NEXT(node);
         ++i;
      }
      // Could be changed
      lastDelayedActionWithExternalTimer = (struct DelayedAction*)prevnode;
   }
   else
   {
      node     = (cce_void*) firstDelayedActionWithVerySmallDelay;
      prevnode = (cce_void*) lastDelayedActionWithExternalTimer;
   }
   i = 0;
   if (node != NULL) while (i < delayedActionsWithVerySmallDelayQuantity)
   {
      head = (struct DelayedAction*)node;
      timerDiff = (int64_t)currentTime - (int64_t)(head->initTime + head->delay);
      if (timerDiff < 0)
         goto CONTINUE2;
      count = 1;
      if ((uint64_t)timerDiff >= head->delay && (head->flags & CCE_DELAYACTION_EXECUTE_ONCE_PER_TIMER_ALARM) == 0 && head->delay > 0)
      {
         count += timerDiff / head->delay;
         timerDiff %= head->delay;
      }
      count = CCE_MIN(count, head->repeatsLeft);
      EXEC_ACTION(node + sizeof(struct DelayedAction), count);
      head->repeatsLeft = (head->flags & CCE_DELAYACTION_NEVER_END) ? head->repeatsLeft : head->repeatsLeft - count;
      if (head->repeatsLeft == 0)
      {
         node = LL_NEXT(node);
         llrmsublistp(&delayedActions, prevnode, 1);
         --delayedActionsWithVerySmallDelayQuantity;           
         if ((struct DelayedAction*)prevnode == lastDelayedActionWithExternalTimer)
            firstDelayedActionWithVerySmallDelay = (struct DelayedAction*)((i < delayedActionsWithVerySmallDelayQuantity) ? node : NULL);
         continue;
      }
      head->initTime = currentTime - timerDiff;
CONTINUE2:
      prevnode = node;
      node = LL_NEXT(node);
      ++i;
   }
   else
      node = (cce_void*) nextDelayedActionWithoutExternalTimer;
   // Could be changed
   lastDelayedActionWithVerySmallDelay = (struct DelayedAction*)prevnode;
   head = (struct DelayedAction*)node;
   if (node == NULL || ((int64_t)currentTime - (int64_t)(head->initTime + head->delay)) < 0)
      return; // Skip checking remaining actions because the one with least remaining time hasn't expired
   struct DelayedAction *prevMinDelayNode = NULL;
   struct DelayedAction *minDelayNode = NULL;
   uint64_t minimalTimeLeft = UINT64_MAX;
   while (node != NULL)
   {
      head = (struct DelayedAction*)node;
      timerDiff = (int64_t)currentTime - (int64_t)(head->initTime + head->delay);
      if (timerDiff < 0)
      {
         if (minimalTimeLeft > (uint64_t)(-timerDiff))
         {
            prevMinDelayNode = (struct DelayedAction*)prevnode;
            minDelayNode = head;
            minimalTimeLeft = -timerDiff;
         }
         goto CONTINUE3;
      }
      count = 1;
      if ((uint64_t)timerDiff >= head->delay && !(head->flags & CCE_DELAYACTION_EXECUTE_ONCE_PER_TIMER_ALARM))
      {
         count += timerDiff / head->delay;
         timerDiff %= head->delay;
      }
      count = CCE_MIN(count, head->repeatsLeft);
      EXEC_ACTION(node + sizeof(struct DelayedAction), count);
      head->repeatsLeft = (head->flags & CCE_DELAYACTION_NEVER_END) ? head->repeatsLeft : head->repeatsLeft - count;
      if (head->repeatsLeft == 0)
      {
         node = LL_NEXT(node);
         llrmsublistp(&delayedActions, prevnode, 1);
         continue;
      }
      head->initTime = currentTime - timerDiff;
      if (minimalTimeLeft > (uint64_t)CCE_MAX((int64_t)head->delay - timerDiff, 0))
      {
         prevMinDelayNode = (struct DelayedAction*)prevnode;
         minDelayNode = head;
         minimalTimeLeft = CCE_MAX((int64_t)head->delay - timerDiff, 0);
      }
CONTINUE3:
      prevnode = node;
      node = LL_NEXT(node);
      ++i;
   }
   // Allows checking action that has the least time remaining first (and skipping checking remaining ones if not expired)
   if (nextDelayedActionWithoutExternalTimer != minDelayNode && minDelayNode != NULL)
      llinsertchainp(&delayedActions, (lastDelayedActionWithVerySmallDelay == NULL) ? lastDelayedActionWithExternalTimer : lastDelayedActionWithVerySmallDelay,
                     nextDelayedActionWithoutExternalTimer = llpullnodep(&delayedActions, prevMinDelayNode));
   else if (minDelayNode == NULL)
   {
      nextDelayedActionWithoutExternalTimer = NULL;
   }
}

static void processTimers (void)
{
   struct cce_timer *minRemainsTimer;
   uint64_t minRemains;
   int64_t diff;
   if (nextTimer == NULL /* Was lost, must be found again */ || nextTimer->initTime + nextTimer->delay <= currentTime)
   {
      minRemains = INT64_MAX;
      minRemainsTimer = NULL;
      for (struct cce_timer *iterator = timers, *end = timers + timersQuantity; iterator < end; ++iterator)
      {
         diff = currentTime - (iterator->initTime + iterator->delay);
         if (diff >= 0 && ((iterator->flags & CCE_TIMER_AUTO_RESTART_ON_ALARM) || iterator->flags & CCE_TIMER_START))
         {
            diff *= (diff > 0 && iterator->initTime > 0 && iterator->delay > 0);
            if (diff > (int64_t) iterator->delay)
            {
               diff %= iterator->delay;
            }
            iterator->initTime = currentTime - diff;
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
   else if (g_flags & CCE_PROCESS_TIMERS)
   {
      minRemainsTimer = nextTimer;
      minRemains = currentTime - (nextTimer->initTime + nextTimer->delay);
      for (struct cce_timer *iterator = timers, *end = timers + timersQuantity; iterator < end; ++iterator)
      {
         if (iterator->flags & CCE_TIMER_STOP)
         {
            iterator->initTime = 0;
            continue;
         }
         
         if (!(iterator->flags & CCE_TIMER_START))
            continue;
         
         iterator->initTime = currentTime;
         if (minRemains > iterator->delay)
         {
            minRemains = iterator->delay;
            minRemainsTimer = iterator;
         }
      }
   }
   g_flags &= ~CCE_PROCESS_TIMERS;
   nextTimer = minRemainsTimer;
   return;
}

static void updateActions (void)
{
   currentTime = cceGetTime();
   runDelayedActions();
   if (g_flags & CCE_PROCESS_TIMERS)
      processTimers();
}

CCE_API int cceLoadActions (void *buffer, uint16_t sectionSize, struct cce_buffer *info, FILE *file)
{
   CCE_UNUSED(info);
   CCE_UNUSED(sectionSize);
   uint32_t onLoadActionsSize = 0, onFreeActionsSize = 0;
   struct cce_actioninfo *map = buffer;
   void *onLoadActions;
   uint16_t *onLoadActionsSizes;
   uint16_t onLoadActionsQuantity;
   sectionSize = sectionSize > 0;
   CCE__LOAD_ACTION_METADATA(map, file, onLoadActions, map->onFreeActions, onLoadActionsSize, onFreeActionsSize);
   CCE__LOAD_ACTIONS(map, file, onLoadActions, map->onFreeActions, onLoadActionsSize, onFreeActionsSize,
                     malloc(onLoadActionsQuantity * sizeof(uint16_t) + onLoadActionsSize), 
                     malloc(map->onFreeActionsQuantity * sizeof(uint16_t) + onFreeActionsSize),
                     (uint16_t*)((cce_void*)onLoadActions + onLoadActionsSize), (uint16_t*)((cce_void*)map->onFreeActions + onFreeActionsSize));
   cce__runActions(onLoadActionsSizes, onLoadActions, onLoadActionsQuantity);
   free(onLoadActions); // We no longer need that
   return 0;
}

CCE_API int cceLoadActionsDynamic (void *buffer, uint16_t sectionSize, struct cce_buffer *info, FILE *file)
{
   CCE_UNUSED(info);
   CCE_UNUSED(sectionSize);
   uint32_t onLoadActionsSize = 0, onFreeActionsSize = 0;
   struct cce_dynamicactioninfo *map = buffer;
   CCE__LOAD_ACTION_METADATA(map, file, map->onLoadActions, map->onFreeActions, onLoadActionsSize, onFreeActionsSize);
   CCE__LOAD_ACTIONS(map, file, map->onLoadActions, map->onFreeActions, onLoadActionsSize, onFreeActionsSize,
                     malloc(onLoadActionsSize), 
                     malloc(onFreeActionsSize),
                     malloc(map->onLoadActionsQuantity * sizeof(uint16_t)),
                     malloc(map->onFreeActionsQuantity * sizeof(uint16_t)));
   cce__runActions(map->onLoadActionsSizes, map->onLoadActions, map->onLoadActionsQuantity);
   map->onLoadActionsSizesAllocated  = map->onLoadActionsQuantity;
   map->onFreeActionsSizesAllocated  = map->onFreeActionsQuantity;
   return 0;
}

CCE_API void cceCreateActions (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   struct cce_dynamicactioninfo *map = buffer;
   CCE_ALLOC_ARRAY(map->onLoadActionsSizes, 1);
   CCE_ALLOC_ARRAY(map->onFreeActionsSizes, 1);
   map->onLoadActions = NULL;
   map->onLoadActionsQuantity = 0;
   map->onFreeActions = NULL;
   map->onFreeActionsQuantity = 0;
}

CCE_API void cceFreeActions (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   struct cce_actioninfo *map = buffer;
   free(map->onFreeActions);
}

CCE_API void cceFreeActionsDynamic (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   struct cce_dynamicactioninfo *map = buffer;
   free(map->onLoadActions);
   free(map->onLoadActionsSizes);
   free(map->onFreeActions);
   free(map->onFreeActionsSizes);
}

CCE_API uint16_t cceStoreActions (void *buffer, struct cce_buffer *info, FILE *file)
{
   CCE_UNUSED(info);
   struct cce_dynamicactioninfo *map = buffer;
   uint16_t tmp = cceHostEndianToLittleEndianInt16(map->onLoadActionsQuantity);
   fwrite(&tmp, sizeof(uint16_t), 1, file);
   tmp = cceHostEndianToLittleEndianInt16(map->onFreeActionsQuantity);
   fwrite(&tmp, sizeof(uint16_t), 1, file);
   size_t bytesWrittenPos = ftell(file), endPos;
   fseek(file, sizeof(uint32_t) * 2, SEEK_CUR);
   uint32_t bytesWritten[4] = {0};
   uint16_t *sizes, *end;
   uint16_t maxSize = 0;
   if (cceEndianess == CCE_BIG_ENDIAN)
   {
      sizes = map->onLoadActionsSizes, end = map->onLoadActionsSizes + map->onLoadActionsQuantity;
      for (uint16_t *iterator = sizes; iterator < end; ++iterator)
         maxSize = *iterator > maxSize ? *iterator : maxSize;
      
      sizes = map->onFreeActionsSizes, end = map->onFreeActionsSizes + map->onFreeActionsQuantity;
      for (uint16_t *iterator = sizes; iterator < end; ++iterator)
         maxSize = *iterator > maxSize ? *iterator : maxSize;
      
      cce_void *actionTMP = malloc(maxSize);
      STORE_ACTIONS_SWAP_ENDIAN(map->onLoadActions,  sizes, bytesWritten[0], tmp, actionTMP)
      STORE_ACTIONS_SWAP_ENDIAN(map->onFreeActions,  sizes, bytesWritten[1], tmp, actionTMP)
      free(actionTMP);
   }
   else
   {
      STORE_ACTIONS_PRESERVE_ENDIAN(map->onLoadActions,  sizes, bytesWritten[0])
      STORE_ACTIONS_PRESERVE_ENDIAN(map->onFreeActions,  sizes, bytesWritten[1])
   }
   endPos = ftell(file);
   fseek(file, bytesWrittenPos, SEEK_SET);
   cceHostEndianToLittleEndianArrayInt32(bytesWritten, 4);
   fwrite(bytesWritten, sizeof(uint32_t), 2, file);
   fseek(file, endPos, SEEK_SET);
   return 1;
}

CCE_API void cceLoadActionsPlugin (void)
{
   cceRegisterPlugin("actions", NULL, NULL, initActions, terminateActions, 0);
   cceRegisterUpdateCallback(updateActions);
}
