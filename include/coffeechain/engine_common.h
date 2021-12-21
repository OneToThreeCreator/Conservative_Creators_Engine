#ifndef ENGINE_COMMON_H
#define ENGINE_COMMON_H
#ifdef __cplusplus
extern C:
{
#endif // __cplusplus
#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include "config.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__TOS_WIN__) || defined(__WINDOWS__) || \
    defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN__)
#include "cce_exports.h"
#define CCE_OPTIONS CCE_EXPORTS
#else
#define CCE_OPTIONS
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

#define CCE_ENABLE_BOOL 1u
#define CCE_DISABLE_BOOL 2u
#define CCE_SWITCH_BOOL 3u

#define CCE_FIXED_RESOLUTION 0x1
#define CCE_FIXED_ASPECT_RATIO 0x2
#define CCE_MINIMAL_ASPECT_RATIO 0x3
#define CCE_MAXIMUM_ASPECT_RATIO 0x4
#define CCE_GLOBAL_BOOL_LOGIC_ELEMENT 0x0u
#define CCE_PLOT_NUMBER_LOGIC_ELEMENT 0x1u
#define CCE_TIMER_LOGIC_ELEMENT       0x2u

struct cce_uvec2
{
   uint32_t x;
   uint32_t y;
};

struct cce_ivec2
{
   int32_t x;
   int32_t y;
};

struct Texture
{
   float    startX;
   float    startY;
   float    endX;
   float    endY;
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
   uint32_t *elementIDs;
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
   double delay;
};

CCE_OPTIONS extern const double *const cce_deltaTime;
CCE_OPTIONS extern const double *const cce_currentTime;
CCE_OPTIONS void startTimer      (struct Timer *timer);
CCE_OPTIONS uint8_t isTimerEnded (struct Timer *timer);
CCE_OPTIONS uint8_t getBool         (uint16_t boolID);
CCE_OPTIONS void setBool            (uint16_t boolID, cce_enum action);
CCE_OPTIONS void setPlotNumber      (uint16_t value);
CCE_OPTIONS void increasePlotNumber (uint16_t value);
CCE_OPTIONS uint8_t checkPlotNumber (uint16_t value);
CCE_OPTIONS size_t binarySearch (const void *const array, const size_t arraySize, const size_t typeSize, const size_t step, const size_t value);
CCE_OPTIONS uint_fast16_t* parseStringToLogicOperations (const char *const string, uint_fast8_t *const logicQuantity);

CCE_OPTIONS extern void (*cce_setWindowParameters) (cce_enum parameter, uint32_t a, uint32_t b);

#ifdef __cplusplus
}
#endif // __cplusplus
#endif // ENGINE_H
