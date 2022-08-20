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

#ifndef ENGINE_COMMON_H
#define ENGINE_COMMON_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdint.h>
#include <stddef.h>
#include "config.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__TOS_WIN__) || defined(__WINDOWS__) || \
    defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN__)
#include "cce_exports.h"
#define CCE_PUBLIC_OPTIONS CCE_EXPORTS
#else
#define CCE_PUBLIC_OPTIONS
#endif // Windows

#ifdef __OPTIMIZE_SIZE__
typedef int8_t cce_byte;
typedef uint8_t cce_ubyte;
typedef int32_t cce_int;
typedef uint32_t cce_uint;
#else
typedef int_fast8_t cce_byte;
typedef uint_fast8_t cce_ubyte;
typedef int_fast32_t cce_int;
typedef uint_fast32_t cce_uint;
#endif // __OPTIMIZE_SIZE__
typedef uint8_t cce_void;
typedef uint_fast8_t cce_enum;

#define CCE_ENABLE_BOOL 0x1
#define CCE_DISABLE_BOOL 0x0
#define CCE_SWITCH_BOOL 0x2

#define CCE_FIXED_RESOLUTION 0x4
#define CCE_FIXED_ASPECT_RATIO 0x5
#define CCE_MINIMAL_ASPECT_RATIO 0x6
#define CCE_MAXIMUM_ASPECT_RATIO 0x7
#define CCE_GLOBAL_BOOL_LOGIC_ELEMENT 0x8
#define CCE_PLOT_NUMBER_LOGIC_ELEMENT 0x9
#define CCE_TIMER_LOGIC_ELEMENT       0xA

struct cce_u16vec2
{
   uint16_t x;
   uint16_t y;
};

struct cce_i16vec2
{
   int16_t x;
   int16_t y;
};

struct cce_u32vec2
{
   uint32_t x;
   uint32_t y;
};

struct cce_i32vec2
{
   int32_t x;
   int32_t y;
};

struct cce_f32vec2
{
   float x;
   float y;
};

struct Texture
{
   struct cce_u16vec2 position;
   struct cce_u16vec2 size;
   uint32_t ID; /* 0 is no texture */
};

struct ElementLogic
{
   uint8_t        logicElementsQuantity; /* maximum is 32 values (because it's already has 512 MiB size, operations doubles per every value), this is overkill anyway */
   uint8_t        actionsQuantity;
   uint16_t      *logicElements;
   uint_fast16_t *operations;            /* operations = truth table (Table values is operation output, bools offsets calculates this way: 2 ^ (q - n) where q is quantity, n is bool order). */
   uint64_t       elementType;           /* logicElement type: 00 bool, 01 plotNumberValue, 10 timer, 11 collisionGroup. Reading from 0x1 to max.*/
   uint32_t      *actionIDs;
   uint32_t      *actionsArgOffsets;
   cce_void      *actionsArg;
};

struct ElementGroup
{   
   uint32_t *elements;
   uint16_t  elementsQuantity;
};

struct CollisionGroup
{
   uint16_t group1;
   uint16_t group2;
};

struct Timer
{
   double initTime;
   float delay;
};

CCE_PUBLIC_OPTIONS extern const double *const cceDeltaTime;
CCE_PUBLIC_OPTIONS extern const double *const cceCurrentTime;
CCE_PUBLIC_OPTIONS void cceStartTimer      (struct Timer *timer);
CCE_PUBLIC_OPTIONS uint8_t cceIsTimerExpired (struct Timer *timer);
CCE_PUBLIC_OPTIONS void resetTimerDelayCompensation (void);
CCE_PUBLIC_OPTIONS uint8_t cceGetBool         (uint16_t boolID);
CCE_PUBLIC_OPTIONS void cceSetBool            (uint16_t boolID, cce_enum action);
CCE_PUBLIC_OPTIONS void cceSetPlotNumber      (uint16_t value);
CCE_PUBLIC_OPTIONS void cceIncreasePlotNumber (uint16_t value);
CCE_PUBLIC_OPTIONS uint8_t cceCheckPlotNumber (uint16_t value);
CCE_PUBLIC_OPTIONS size_t cceBinarySearch (const void *const array, const size_t arraySize, const size_t typeSize, const size_t step, const size_t value);
CCE_PUBLIC_OPTIONS uint_fast16_t* cceParseStringToLogicOperations (const char *const string, uint_fast8_t *const logicQuantity);
CCE_PUBLIC_OPTIONS cce_ubyte cceCheckCollision (int32_t element1_x, int32_t element1_y, int32_t element1_width, int32_t element1_height,
                                                int32_t element2_x, int32_t element2_y, int32_t element2_width, int32_t element2_height);

CCE_PUBLIC_OPTIONS extern void (*cceSetWindowParameters) (cce_enum parameter, uint32_t a, uint32_t b);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // ENGINE_H
