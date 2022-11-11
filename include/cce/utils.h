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

#ifndef UTILS_H
#define UTILS_H

#include "cce_exports.h"
#define CCE_PUBLIC_OPTIONS CCE_EXPORTS
#include <stdint.h>

#define CCE_UNUSED(x) (void)(x)

#define CCE__MACRO_TO_STR(x) #x 
#define CCE_MACRO_TO_STR(x) CCE__MACRO_TO_STR(x)

#define CCE_CEIL_TO_POWER_OF_TWO(x, result) \
(result = (x) - ((x) != 0), \
(result) |= (result) >> 1, \
(result) |= (result) >> 2, \
(result) |= (result) >> 4, \
(sizeof(result) >= 2) ? (result) |= (result) >> 8  : 0, \
(sizeof(result) >= 4) ? (result) |= (result) >> 16 : 0, \
(sizeof(result) >= 8) ? (result) |= (result) >> 32 : 0, \
/*Avoid overflow, set number to 2^n - 1 instead*/\
((result) += ((result) >> (sizeof(result) * 8 - 1)) == 0))

#define CCE_KEEP_HIGH_BIT(x, result) \
((result) |= (result) >> 1, \
(result) |= (result) >> 2, \
(result) |= (result) >> 4, \
(sizeof(result) >= 2) ? (result) |= (result) >> 8  : 0, \
(sizeof(result) >= 4) ? (result) |= (result) >> 16 : 0, \
(sizeof(result) >= 8) ? (result) |= (result) >> 32 : 0, \
(result) -= (result) >> 1)

extern const uint8_t  cce__debruijnToBitPosition64[64];
extern const uint8_t  cce__debruijnToBitPosition32[32];
extern const uint64_t cce__debruijnNumberLS6;
extern const uint32_t cce__debruijnNumberLS5;

#define CCE_LOWEST_BIT_INDEX(x) \
((sizeof(x) == 8) ? cce__debruijnToBitPosition64[(uint64_t)(((x) & -(x)) * cce__debruijnNumberLS6) >> 58] : cce__debruijnToBitPosition32[(uint32_t)((x & -x) * cce__debruijnNumberLS5) >> 27])

#define CCE_HIGHEST_BIT_INDEX(x) \
((sizeof(x) == 8) ? cce__debruijnToBitPosition64[(uint64_t)(cceKeepHighBitInt64(x) * cce__debruijnNumberLS6) >> 58] : cce__debruijnToBitPosition32[(uint32_t)(cceKeepHighBitInt32(x) * cce__debruijnNumberLS5) >> 27])

#define CCE_GET_EXPONENT_ABOVE_NUMBER(x) \
((sizeof(x) == 8) ? cce__debruijnToBitPosition64[(uint64_t)(cceCeilToPowerOfTwoInt64(x) * cce__debruijnNumberLS6) >> 58] : cce__debruijnToBitPosition32[(uint32_t)(cceCeilToPowerOfTwoInt32(x) * cce__debruijnNumberLS5) >> 27])

#ifdef __cplusplus
#ifndef UTILS_HPP
#include "utils.hpp"
#endif // UTILS_HPP
extern "C"
{
#include <stddef.h>

#else

#include <stdlib.h>
#include <string.h>

#define CCE_MAX(x,y) (((x)>(y))?(x):(y))
#define CCE_MIN(x,y) (((x)<(y))?(x):(y))
#define CCE_ABS(x)   (((x)>=0) ?(x):(-x))

// sizeType MUST be unsigned
#define CCE_ARRAY(name, type, sizeType) \
type * name = NULL; \
sizeType name ## Quantity = 0; \
sizeType name ## Allocated = 0

#define CCE_ARRAY_STRUCT(name, type, sizeType) \
struct name \
{ \
   type * data; \
   sizeType dataQuantity; \
   sizeType dataAllocated; \
} \

#define CCE_ALLOC_ARRAY(name)        (name) = malloc(((name ## Allocated) = 1) * sizeof(*(name)))
#define CCE_ALLOC_ARRAY_ZEROED(name) (name) = calloc(((name ## Allocated) = 1),  sizeof(*(name)))

#define CCE__REALLOC_ARRAY(name, newQuantity) \
size_t oldAllocated = name ## Allocated; \
CCE_CEIL_TO_POWER_OF_TWO(newQuantity, name ## Allocated); \
if ((name ## Allocated) == oldAllocated) \
   break; \
(name) = realloc(name, (name ## Allocated) * sizeof(*(name)))

#define CCE_REALLOC_ARRAY(name, newQuantity) \
do \
{ \
   CCE__REALLOC_ARRAY(name, newQuantity); \
} \
while(0)

#define CCE_REALLOC_ARRAY_ZEROED(name, newQuantity) \
do \
{ \
   CCE__REALLOC_ARRAY(name, newQuantity); \
   if ((name ## Allocated) > oldAllocated) \
      memset((name) + oldAllocated, 0, ((name ## Allocated) - oldAllocated) * sizeof(*name)); \
} \
while(0)

#endif // __cplusplus

struct UnicodeCharWithSize
{
   uint32_t ch;
   uint32_t size;
};

CCE_PUBLIC_OPTIONS const void* cceBinarySearchFirstAscending  (const void *array, size_t arraySize, size_t typeSize, size_t step, size_t value);
CCE_PUBLIC_OPTIONS const void* cceBinarySearchFirstDescending (const void *array, size_t arraySize, size_t typeSize, size_t step, size_t value);
CCE_PUBLIC_OPTIONS const void* cceBinarySearchCMP             (const void *array, size_t arraySize, size_t step, int (*cmp)(const void*, const void*), const void *value);
CCE_PUBLIC_OPTIONS char*    cceReverseMemory (char *memory, size_t size);
CCE_PUBLIC_OPTIONS uint32_t cceGetCharSizeUTF8 (const unsigned char *ch);
CCE_PUBLIC_OPTIONS uint32_t cceGetCharUTF8 (const unsigned char *ch);
CCE_PUBLIC_OPTIONS struct UnicodeCharWithSize cceGetCharWithSizeUTF8 (const unsigned char *ch);
CCE_PUBLIC_OPTIONS uint32_t cceGetCharFromStringUTF8 (const char *string, size_t position);
CCE_PUBLIC_OPTIONS uint8_t  cceCeilToPowerOfTwoInt8 (uint8_t x);
CCE_PUBLIC_OPTIONS uint16_t cceCeilToPowerOfTwoInt16 (uint16_t x);
CCE_PUBLIC_OPTIONS uint32_t cceCeilToPowerOfTwoInt32 (uint32_t x);
CCE_PUBLIC_OPTIONS uint64_t cceCeilToPowerOfTwoInt64 (uint64_t x);
CCE_PUBLIC_OPTIONS uint8_t  cceKeepHighBitInt8 (uint8_t x);
CCE_PUBLIC_OPTIONS uint16_t cceKeepHighBitInt16 (uint16_t x);
CCE_PUBLIC_OPTIONS uint32_t cceKeepHighBitInt32 (uint32_t x);
CCE_PUBLIC_OPTIONS uint64_t cceKeepHighBitInt64 (uint64_t x);
CCE_PUBLIC_OPTIONS float cceFastSinInt8 (uint8_t x);

#define cceFastCosInt8(x) cceFastSinInt8(x + 64)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // UTILS_H
