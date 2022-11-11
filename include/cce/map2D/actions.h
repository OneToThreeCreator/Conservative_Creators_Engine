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

#ifndef ACTIONS_H
#define ACTIONS_H
#include <stdint.h>
#include "../engine_common.h"

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#define CCE_ACTION_SHIFT 0x0
#define CCE_ACTION_SET   0x1

/*
   Every structure here has 32-bit alignment. Using 64-bit aligned structures is not recommended, due to a possibility of misaligned reading
*/

#define CCE_ROTATE_FLIP 0x2

#define CCE_TRANSFORMACTION_FLIP                CCE_ROTATE_FLIP
#define CCE_TRANSFORMACTION_MOVE                0x4
#define CCE_TRANSFORMACTION_SCALE               0x8
#define CCE_TRANSFORMACTION_ROTATE_WITH_OFFSET  0x1C
#define CCE_TRANSFORMACTION_ROTATE              0x10


typedef void (*cce_actionfun)(const void*, uint8_t);

struct transformAction
{
   uint32_t actionID;
   union
   {
      struct cce_i16vec2 move;
      // Fixed-point (1/32) scaling factor. Negative means scaling down (dividing by this)!
      struct cce_i16vec2 scale;
      struct cce_i16vec2 rotationOffset;
   } transformation;
   uint16_t groupID;
   uint8_t rotationAngle;
   uint8_t flags;
};

#define CCE_COMBINEDACTION_A1_SHIFT 0x0
#define CCE_COMBINEDACTION_A2_SHIFT 0x0
#define CCE_COMBINEDACTION_A3_SHIFT 0x0
#define CCE_COMBINEDACTION_A4_SHIFT 0x0
#define CCE_COMBINEDACTION_A1_SET 0x1
#define CCE_COMBINEDACTION_A2_SET 0x2
#define CCE_COMBINEDACTION_A3_SET 0x4
#define CCE_COMBINEDACTION_A4_SET 0x8

#define CCE_COMBINEDTRANSFORMACTION_MOVE_SHIFT             CCE_COMBINEDACTION_A1_SHIFT
#define CCE_COMBINEDTRANSFORMACTION_SCALE_SHIFT            CCE_COMBINEDACTION_A2_SHIFT
#define CCE_COMBINEDTRANSFORMACTION_ROTATEWITHOFFSET_SHIFT CCE_COMBINEDACTION_A3_SHIFT
#define CCE_COMBINEDTRANSFORMACTION_MOVE_SET               CCE_COMBINEDACTION_A1_SET
#define CCE_COMBINEDTRANSFORMACTION_SCALE_SET              CCE_COMBINEDACTION_A2_SET
#define CCE_COMBINEDTRANSFORMACTION_ROTATEWITHOFFSET_SET   CCE_COMBINEDACTION_A3_SET
#define CCE_COMBINEDTRANSFORMACTION_FLIP                   0x40

struct combinedTransformAction
{
   uint32_t actionID;
   struct cce_i16vec2 move;
   // Fixed-point (1/32) scaling factor. Negative means scaling down (dividing by this)!
   struct cce_i16vec2 scale;
   struct cce_i16vec2 rotationOffset;
   uint16_t groupID;
   uint8_t rotationAngle;
   uint8_t cflags;
};

struct offsetTextureAction
{
   uint32_t actionID;
   struct cce_i16vec2 offset;
   uint8_t groupID;
   uint8_t actionType;
   uint16_t __pad;
};

struct changeColorAction
{
   uint32_t actionID;
   union cce_color color;
   uint8_t groupID;
   uint8_t actionType;
   uint16_t __pad;
};

#define CCE_CHANGETIMERSTATE_START  0x1
#define CCE_CHANGETIMERSTATE_STOP   0x2
#define CCE_CHANGETIMERSTATE_SWITCH 0x3
#define CCE_CHANGETIMERSTATE_ENABLE_AUTO_RESTART_ON_ALARM  0x4
#define CCE_CHANGETIMERSTATE_DISABLE_AUTO_RESTART_ON_ALARM 0x8
#define CCE_CHANGETIMERSTATE_SWITCH_AUTO_RESTART_ON_ALARM  0x10


struct setTimerStateAction
{
   uint32_t actionID;
   uint16_t ID;
   cce_enum state;
   uint8_t __pad;
};

struct setTimerDelayAction
{
   uint32_t actionID;
   uint32_t delay;
   uint16_t ID;
   cce_enum action;
   uint8_t __pad;
};

#define CCE_LOADMAPACTION_DONT_REPLACE_MAIN_MAP 0x01

struct loadMap2Daction
{
   uint32_t actionID;
   struct cce_i32vec2 offset;
   uint16_t ID;
   cce_enum action;
   uint8_t flags;
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
struct delayAction
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
   uint8_t __pad;
};

struct runActions
{
   uint32_t actionID;
   uint16_t actionQuantity;
   uint16_t actionSizes[1]; // Can be more than 1 - depends on quantity. Last is omitted! When even must be pad to keep everithing aligned.
};

CCE_PUBLIC_OPTIONS void    cceMoveGroup (uint8_t groupID,  struct cce_i16vec2 coords, cce_enum actionType);
CCE_PUBLIC_OPTIONS void    cceScaleGroup (uint8_t groupID, struct cce_i16vec2 size,   cce_enum actionType);
CCE_PUBLIC_OPTIONS uint8_t cceNormalizeAngle (float angleInDegrees);
CCE_PUBLIC_OPTIONS void    cceRotateWithOffsetGroup (uint8_t groupID, uint8_t normalizedAngle, struct cce_i16vec2 offset, cce_enum actionType);
CCE_PUBLIC_OPTIONS void    cceRotateGroup (uint8_t groupID, uint8_t normalizedAngle, cce_enum actionType);
CCE_PUBLIC_OPTIONS void    cceOffsetTextureGroup (uint8_t groupID, struct cce_i16vec2 offset, cce_enum actionType);
CCE_PUBLIC_OPTIONS void    cceChangeColorGroup (uint8_t groupID, union cce_color color, cce_enum actionType);
CCE_PUBLIC_OPTIONS void    cceSetTimerDelay (uint16_t timerID, uint32_t newDelay, uint8_t actionType);
CCE_PUBLIC_OPTIONS void    cceSetTimerState (uint16_t timerID, uint8_t state);
CCE_PUBLIC_OPTIONS void    cceDelayAction (uint16_t repeatsQuantity, uint32_t delayOrID, uint32_t actionStructSize, void *actionStruct, uint8_t flags);
CCE_PUBLIC_OPTIONS uint8_t cceRegisterAction (uint32_t ID, cce_actionfun action, void (*endianSwap)(void*));

#define CCE_TRANSFORM_ACTION 0
#define CCE_COMBINEDTRANSFORM_ACTION 1
#define CCE_OFFSETTEXTURE_ACTION 2
#define CCE_CHANGECOLOR_ACTION 3
#define CCE_STARTTIMER_ACTION 4
#define CCE_SETDYNAMICTIMERDELAY_ACTION 5
#define CCE_SETGRIDSIZE_ACTION 6
#define CCE_LOADMAP2D_ACTION 7
#define CCE_DELAYACTION_ACTION 8
#define CCE_RUNACTIONS_ACTION 9

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ACTIONS_H
