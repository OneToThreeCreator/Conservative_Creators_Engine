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
#define CCE_API CCE_EXPORTS
#include <stdint.h>

#define CCE_UNUSED(x) (void)(x)

#define CCE__MACRO_CONCAT(x, y) x ## y
#define CCE_MACRO_CONCAT(x, y) CCE__MACRO_CONCAT(x, y)

#define CCE__MACRO_TO_STR(x) #x 
#define CCE_MACRO_TO_STR(x) CCE__MACRO_TO_STR(x)

// Compares string and literal, returns true if they are equal. x must be at least as big as literal (or buffer overrun will happen).
#define CCE_STREQ(x, literal) (memcmp(x, literal, strlen(literal) + 1) == 0)

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
((result) |= (x) >> 1, \
(result) |= (x) >> 2, \
(result) |= (x) >> 4, \
(sizeof(result) >= 2) ? (result) |= ((uint16_t)x) >> 8  : 0, \
(sizeof(result) >= 4) ? (result) |= ((uint32_t)x) >> 16 : 0, \
(sizeof(result) >= 8) ? (result) |= ((uint64_t)x) >> 32 : 0, \
(result) -= (result) >> 1)

#define CCE__CHAR_DELIMITER 0x1
#define CCE__CHAR_WHITESPACE_LIKE 0x2

extern const uint8_t  cce__charType[128];
extern const uint8_t  cce__debruijnToBitPosition64[64];
extern const uint8_t  cce__debruijnToBitPosition32[32];
extern const uint64_t cce__debruijnNumberLS6;
extern const uint32_t cce__debruijnNumberLS5;

#define cceIsCharWhitespaceLike(ch) (((unsigned)ch) < 128u ? (cce__charType[(unsigned) ch] & CCE__CHAR_WHITESPACE_LIKE) == CCE__CHAR_WHITESPACE_LIKE : 0)
#define cceIsCharDelimiter(ch)      (((unsigned)ch) < 128u ? (cce__charType[(unsigned) ch] & CCE__CHAR_DELIMITER)       == CCE__CHAR_DELIMITER : 0)

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

#define CCE_MAX(x,y) (((x)>(y))?(x): (y))
#define CCE_MIN(x,y) (((x)<(y))?(x): (y))
#define CCE_ABS(x)   (((x)>=0) ?(x):(-x))
#define CCE_CLAMP(x, min, max) (((x) < (max)) ? (((x) > (min)) ? (x) : (min)) : (max))

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

#define CCE_ALLOC_ARRAY(name, size)        (name) = malloc(CCE_CEIL_TO_POWER_OF_TWO(size, (name ## Allocated)) * sizeof(*(name)))
#define CCE_ALLOC_ARRAY_ZEROED(name, size) (name) = calloc(CCE_CEIL_TO_POWER_OF_TWO(size, (name ## Allocated)),  sizeof(*(name)))

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

CCE_API char*    cceReverseMemory (char *memory, size_t size);
CCE_API uint32_t cceGetCharSizeUTF8 (const unsigned char *ch);
CCE_API uint32_t cceGetCharUTF8 (const unsigned char *ch);
CCE_API struct UnicodeCharWithSize cceGetCharWithSizeUTF8 (const unsigned char *ch);
CCE_API uint32_t cceGetCharFromStringUTF8 (const char *string, size_t position);
CCE_API uint8_t  cceCeilToPowerOfTwoInt8 (uint8_t x);
CCE_API uint16_t cceCeilToPowerOfTwoInt16 (uint16_t x);
CCE_API uint32_t cceCeilToPowerOfTwoInt32 (uint32_t x);
CCE_API uint64_t cceCeilToPowerOfTwoInt64 (uint64_t x);
CCE_API uint8_t  cceU8Pow  (uint8_t base,  uint8_t exponent);
CCE_API uint16_t cceU16Pow (uint16_t base, uint16_t exponent);
CCE_API uint32_t cceU32Pow (uint32_t base, uint32_t exponent);
CCE_API uint64_t cceU64Pow (uint64_t base, uint64_t exponent);
CCE_API uint8_t  cceKeepHighBitInt8 (uint8_t x);
CCE_API uint16_t cceKeepHighBitInt16 (uint16_t x);
CCE_API uint32_t cceKeepHighBitInt32 (uint32_t x);
CCE_API uint64_t cceKeepHighBitInt64 (uint64_t x);
CCE_API float cceFastSinInt8 (uint8_t x);
CCE_API char* cceConvertIntToBase64String (size_t number, char *buffer, uint8_t symbolsQuantity);

CCE_API size_t cceStringToLowercase (char *str);
CCE_API void cceMemoryToLowercase (char *str, size_t size);
CCE_API size_t cceStringToUppercase (char *str);
CCE_API void cceMemoryToUppercase (char *str, size_t size);
CCE_API uint8_t cceStringToBool (const char *str);

#define cceFastCosInt8(x) cceFastSinInt8(x + 64u)

#define CCE__STRING_TO_SXVECY(sign, signUpper, uIfUnsigned, bits, comp) \
CCE_API struct cce_ ## sign ## bits ## vec ## comp cceStringTo ## signUpper ## bits ## Vec ## comp (const char *string)

#define CCE__STRING_TO_XVECY(bits, comp) \
CCE__STRING_TO_SXVECY(i, I, , bits, comp); \
CCE__STRING_TO_SXVECY(u, U, u, bits, comp)

#define CCE__STRING_TO_XVEC(bits) \
CCE__STRING_TO_XVECY(bits, 1); \
CCE__STRING_TO_XVECY(bits, 2); \
CCE__STRING_TO_XVECY(bits, 3); \
CCE__STRING_TO_XVECY(bits, 4)

CCE__STRING_TO_XVEC(8);
CCE__STRING_TO_XVEC(16);
CCE__STRING_TO_XVEC(32);
CCE__STRING_TO_XVEC(64);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // UTILS_H
