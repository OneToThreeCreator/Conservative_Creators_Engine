/*
   Conservative Creator's Engine - open source engine for making games.
   Copyright © 2020-2023 Andrey Gaivoronskiy

   This file is part of Conservative Creator's Engine.

   Conservative Creator's Engine is free software: you can redistribute it and/or modify it under 
   the terms of the GNU Lesser General Public License as published by the Free Software Foundation,
   either version 2 of the License, or (at your option) any later version.

   Conservative Creator's Engine is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
   PURPOSE. See the GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License along
   with Conservative Creator's Engine. If not, see <https://www.gnu.org/licenses/>.
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

struct cce_actioninfo
{
   struct cce_action *onFreeActions;
   uint16_t          *onFreeActionsSizes;
   uint16_t           onFreeActionsQuantity;
};

struct cce_dynamicactioninfo
{
   struct cce_action *onFreeActions;
   uint16_t          *onFreeActionsSizes;
   uint16_t           onFreeActionsQuantity;
   uint16_t           onFreeActionsSizesAllocated;
   uint16_t           onLoadActionsQuantity;
   uint16_t           onLoadActionsSizesAllocated;
   struct cce_action *onLoadActions;
   uint16_t          *onLoadActionsSizes;
};

/*
   Every structure here has 32-bit alignment. Using 64-bit aligned structures is not recommended, due to a possibility of misaligned reading
*/

#define CCE_ACTION_SHIFT 0x0
#define CCE_ACTION_SET   0x1

typedef void (*cce_actionfun)(const void*, uint16_t);

#define CCE_CHANGETIMERSTATE_START  0x1
#define CCE_CHANGETIMERSTATE_STOP   0x2
#define CCE_CHANGETIMERSTATE_SWITCH 0x3
#define CCE_CHANGETIMERSTATE_ENABLE_AUTO_RESTART_ON_ALARM  0x4
#define CCE_CHANGETIMERSTATE_DISABLE_AUTO_RESTART_ON_ALARM 0x8
#define CCE_CHANGETIMERSTATE_SWITCH_AUTO_RESTART_ON_ALARM  0x10

struct cceSetTimerStateAction
{
   uint32_t actionID;
   uint16_t ID;
   cce_enum state;
   uint8_t cce__pad;
};

struct cceSetTimerDelayAction
{
   uint32_t actionID;
   uint32_t delay;
   uint16_t ID;
   cce_enum action;
   uint8_t cce__pad;
};

#define CCE_DELAYACTION_EXECUTE_ONCE_PER_TIMER_ALARM 0x1
// repeatsQuantity field is ignored
#define CCE_DELAYACTION_NEVER_END 0x2

#define CCE_DELAYACTION_EXTERNAL_TIMER 0x4
#define CCE_DELAYACTION_RESTART_EXTERNAL_TIMER_ON_ALARM 0x08
#define CCE_DELAYACTION_START_EXTERNAL_TIMER 0x10

// Doesn't count as repeat!
#define CCE_DELAYACTION_EXECUTE_ON_START 0x20

// action data should be located right after this struct
struct cceDelayAction
{
   uint32_t actionID;
   union
   {
      uint32_t delay;
      uint32_t timerID; // technically 16 bit
   } timerInfo;
   uint32_t delayedActionStructSize;
   uint16_t repeatsQuantity;
   uint8_t flags;
   uint8_t cce__pad;
};

// Followed by actions to run
struct cceRunActions
{
   uint32_t actionID;
   uint16_t actionQuantity;
   uint16_t actionSizes[1]; // Can be more than 1 - depends on quantity. Last is omitted! When even must be pad to keep everithing aligned.
};

struct cceSetEngineShouldTerminate
{
   uint32_t actionID;
   uint8_t  state;
   cce_enum action;
};

CCE_API void    cceSetTimerDelay  (uint16_t timerID, uint32_t newDelay, uint8_t actionType);
CCE_API void    cceSetTimerState  (uint16_t timerID, uint8_t state);
CCE_API void    cceDelayAction    (uint16_t repeatsQuantity, uint32_t delayOrID, uint32_t actionStructSize, void *actionStruct, uint8_t flags);
CCE_API uint8_t cceRegisterAction (uint32_t ID, cce_actionfun action, void (*endianSwap)(void*));

CCE_API int      cceLoadActions        (void *buffer, uint16_t sectionSize, struct cce_buffer *info, FILE *file);
CCE_API int      cceLoadActionsDynamic (void *buffer, uint16_t sectionSize, struct cce_buffer *info, FILE *file);
CCE_API void     cceCreateActions      (void *buffer, struct cce_buffer *info);
CCE_API void     cceFreeActions        (void *buffer, struct cce_buffer *info);
CCE_API void     cceFreeActionsDynamic (void *buffer, struct cce_buffer *info);
CCE_API uint16_t cceStoreActions       (void *buffer, struct cce_buffer *info, FILE *file);
CCE_API void     cceLoadActionsPlugin  (void);

#define CCE_ACTION_STARTTIMER 0
#define CCE_ACTION_SETTIMERDELAY 1
#define CCE_ACTION_DELAYACTION 2
#define CCE_ACTION_RUNACTIONS 3
#define CCE_ACTION_SETENGINESHOULDTERMINATE 4

#define CCE__LOAD_ACTION_METADATA(map, file, onLoad, onFree, onLoadSize, onFreeSize) \
fread(&onLoad  ## Quantity, sizeof(uint16_t), 1, file); \
fread(&onFree  ## Quantity, sizeof(uint16_t), 1, file); \
fread(&onLoadSize,   sizeof(uint32_t), 1, file); \
fread(&onFreeSize,   sizeof(uint32_t), 1, file); \
onLoad ## Quantity = cceLittleEndianToHostEndianInt16(onLoad ## Quantity); \
onFree ## Quantity = cceLittleEndianToHostEndianInt16(onFree ## Quantity); \
onLoadSize =  cceLittleEndianToHostEndianInt16(onLoadSize); \
onFreeSize =  cceLittleEndianToHostEndianInt16(onFreeSize)

#define CCE__LOAD_ACTIONS(map, file, onLoad, onFree, onLoadSize, onFreeSize, onLoadActionsAlloc, onFreeActionsAlloc, \
                         onLoadActionSizesAlloc, onFreeActionSizesAlloc) \
do \
{ \
   onLoad = onLoadActionsAlloc; \
   onFree = onFreeActionsAlloc; \
   onLoad ## Sizes = onLoadActionSizesAlloc; \
   onFree  ## Sizes = onFreeActionSizesAlloc; \
   \
   fread(onLoad ## Sizes, sizeof(uint16_t), onLoad ## Quantity, file); \
   fread(onLoad,  1, onLoadSize, file); \
   fread(onFree ## Sizes, sizeof(uint16_t), onFree ## Quantity, file); \
   fread(onFree,  1, onFreeSize, file); \
   \
   if (cceEndianess == CCE_BIG_ENDIAN) \
   { \
      cceSwapEndianArrayIntN(onLoad ## Sizes, onLoad ## Quantity, 2); \
      cceSwapEndianArrayIntN(onFree ## Sizes, onFree ## Quantity, 2); \
      cce__swapActionsEndian(onLoad ## Sizes, onLoad, onLoad  ## Quantity); \
      cce__swapActionsEndian(onFree ## Sizes, onFree, onFree  ## Quantity); \
   } \
} \
while (0)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ACTIONS_H
