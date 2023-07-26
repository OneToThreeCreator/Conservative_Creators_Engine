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

#ifndef ACTIONS_H
#define ACTIONS_H
#include <stdio.h>
#include "../engine_common.h"
#include "../engine_common_IO.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

typedef void (*ccea_actionfun)(void*, uint32_t, struct cce_buffer*);

// All actions are aligned to 32-bit boundary. Be careful with pointers and 64-bit ints!

struct cceaAction
{
   uint32_t UID;
};

struct cceaDynamicAction
{
   uint32_t UID;
   uint32_t size;
};

struct cceaDelayedAction
{
   uint32_t UID;
   // Use for whatever purpose you want - but this pad is necessary! (exactly 4 bytes, can be uint32_t, uint16_t[2] or uint8_t[4])
   uint32_t __pad;
   // Update it to avoid deletion from queue
   uint32_t timeout;
};

struct cceaDelayedDynamicAction
{
   uint32_t UID;
   // Update it to avoid deletion from queue
   uint32_t size;
   uint32_t timeout;
};

// Followed by actions to run
struct cceaActionRunner
{
   uint32_t UID;
   uint32_t totalSize;
};

// Followed by actions to run
struct cceaRunActions
{
   uint32_t UID;
   uint32_t totalSize;
};

// Followed by actions to run
struct cceaRunActionsNTimes
{
   uint32_t UID;
   uint32_t totalSize;
   uint32_t n;
};

// Followed by actions to add
struct cceaAppendListOfRunDelayedActions
{
   uint32_t UID;
   uint32_t totalSize;
};

// Followed by actions to run
struct cceaDelayActions
{
   uint32_t UID;
   uint32_t totalSize;
   uint32_t delay;
};

// Followed by actions to run
struct cceaDelayActionsPeriodic
{
   uint32_t UID;
   uint32_t totalSize;
   uint32_t delay;
};

// Followed by actions to run
struct cceaDelayActionsRepeated
{
   uint32_t UID;
   uint32_t totalSize;
   uint32_t delay;
   uint32_t repeats;
};

// Followed by actions to add
struct cceaAddActionsOnEvent
{
   uint32_t UID;
   uint32_t totalSize;
   uint32_t eventUID;
   uint32_t actionSubsUID;
};

struct cceaRemoveActionOnEvent
{
   uint32_t UID;
   uint32_t eventUID;
   uint32_t actionSubsUID;
};

struct cceaInvokeEvent
{
   uint32_t UID;
   uint32_t eventUID;
};

struct cceaTerminateEngine
{
   uint32_t UID;
};

#define CCEA_RUN_ACTIONS 0
#define CCEA_RUN_ACTIONS_ONCE 1
#define CCEA_RUN_ACTIONS_N_TIMES 2
#define CCEA_DELAY_ACTIONS 3
#define CCEA_DELAY_ACTIONS_PERIODIC 4
#define CCEA_DELAY_ACTIONS_REPEATED 5
#define CCEA_RUN_DELAYED_ACTIONS 7
#define CCEA_RUN_PERIODIC_ACTIONS 8
#define CCEA_RUN_REPEATED_ACTIONS 9
#define CCEA_APPEND_LIST_OF_RUNDELAYED_ACTIONS 11
#define CCEA_ADD_ACTIONS_ON_EVENT 12
#define CCEA_REMOVE_ACTIONS_ON_EVENT 13
#define CCEA_INVOKE_EVENT 14
#define CCEA_TERMINATE_ENGINE 15

CCE_API extern uint32_t cceaBasicActionUIDs[16];

#define CCEA_EVENT_LOAD 0
#define CCEA_EVENT_FREE 1
#define CCEA_EVENT_WRITE 2

CCE_API extern uint32_t cceaBasicEventsUIDs[3];

CCE_API extern uint32_t cceaPluginUID;

CCE_API void  cceaRunAction (struct cceaAction *action, uint32_t count, struct cce_buffer *state);
CCE_API void  cceaRegisterAction (uint32_t actionUID, ccea_actionfun action, void (*endianSwap)(void*), uint32_t actionSize);
CCE_API void  cceaRegisterEvent (uint32_t eventUID);
CCE_API void  cceaInvokeEvent (uint32_t eventUID, uint32_t count, struct cce_buffer *map);
CCE_API void  cceaAddActionOnEvent (uint32_t eventUID, uint32_t actionSubsUID, struct cceaAction *action, struct cce_buffer *map);
CCE_API void  cceaRemoveActionOnEvent (uint32_t eventUID, uint32_t actionSubsUID, struct cce_buffer *map);
CCE_API void  cceaLoadActionsPlugin  (void);
// Expects POINTERS to actions, not actions themselves
CCE_API void* cceaActionRunnerCreateDynamic (uint32_t runActionsID, uint32_t runActionsStructSize, uint32_t actionsQuantity, ...);
CCE_API void  cceaRegisterActionsFileIOFunctions (uint16_t functionSetID);

CCE_API void  cceaRunDelayedActions (struct cce_buffer *map);

#define CCEA_ACTION_SIZE_VARIABLE 0

#ifdef __cplusplus
}
#endif // __cplusplus
#include "actions_runactions.h"
#endif // ACTIONS_H
