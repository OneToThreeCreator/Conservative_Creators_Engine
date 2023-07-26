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
#ifndef UTILS_H
#define UTILS_H

#include "cce_exports.h"
#include "attributes.h"
#include "engine_common.h"
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
/* Avoid overflow, set number to 2^n - 1 instead */\
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

#define CCE__RETURN_128TH_ARG(  _1,   _2,   _3,   _4,   _5,   _6,   _7,   _8,   _9,  _10, \
                               _11,  _12,  _13,  _14,  _15,  _16,  _17,  _18,  _19,  _20, \
                               _21,  _22,  _23,  _24,  _25,  _26,  _27,  _28,  _29,  _30, \
                               _31,  _32,  _33,  _34,  _35,  _36,  _37,  _38,  _39,  _40, \
                               _41,  _42,  _43,  _44,  _45,  _46,  _47,  _48,  _49,  _50, \
                               _51,  _52,  _53,  _54,  _55,  _56,  _57,  _58,  _59,  _60, \
                               _61,  _62,  _63,  _64,  _65,  _66,  _67,  _68,  _69,  _70, \
                               _71,  _72,  _73,  _74,  _75,  _76,  _77,  _78,  _79,  _80, \
                               _81,  _82,  _83,  _84,  _85,  _86,  _87,  _88,  _89,  _90, \
                               _91,  _92,  _93,  _94,  _95,  _96,  _97,  _98,  _99, _100, \
                              _101, _102, _103, _104, _105, _106, _107, _108, _109, _110, \
                              _111, _112, _113, _114, _115, _116, _117, _118, _119, _120, \
                              _121, _122, _123, _124, _125, _126, _127, _128, ...) _128

#define CCE_COUNT_ARGS(...) \
CCE__RETURN_128TH_ARG(__VA_ARGS__, 127, 126, 125, 124, 123, 122, 121, 120, \
                         119, 118, 117, 116, 115, 114, 113, 112, 111, 110, \
                         109, 108, 107, 106, 105, 104, 103, 102, 101, 100, \
                          99,  98,  97,  96,  95,  94,  93,  92,  91,  90, \
                          89,  88,  87,  86,  85,  84,  83,  82,  81,  80, \
                          79,  78,  77,  76,  75,  74,  73,  72,  71,  70, \
                          69,  68,  67,  66,  65,  64,  63,  62,  61,  60, \
                          59,  58,  57,  56,  55,  54,  53,  52,  51,  50, \
                          49,  48,  47,  46,  45,  44,  43,  42,  41,  40, \
                          39,  38,  37,  36,  35,  34,  33,  32,  31,  30, \
                          29,  28,  27,  26,  25,  24,  23,  22,  21,  20, \
                          19,  18,  17,  16,  15,  14,  13,  12,  11,  10, \
                           9,   8,   7,   6,   5,   4,   3,   2,   1,   0)



#define cceIsCharWhitespaceLike(ch) (((unsigned)ch) < 128u ? (cce__charType[(unsigned) ch] & CCE__CHAR_WHITESPACE_LIKE) == CCE__CHAR_WHITESPACE_LIKE : 0)
#define cceIsCharDelimiter(ch)      (((unsigned)ch) < 128u ? (cce__charType[(unsigned) ch] & CCE__CHAR_DELIMITER)       == CCE__CHAR_DELIMITER : 0)

#define CCE_LOWEST_BIT_INDEX(x) \
((sizeof(x) == 8) ? cce__debruijnToBitPosition64[(uint64_t)(((x) & -(x)) * cce__debruijnNumberLS6) >> 58] : cce__debruijnToBitPosition32[(uint32_t)((x & -x) * cce__debruijnNumberLS5) >> 27])

#define CCE_HIGHEST_BIT_INDEX(x) \
((sizeof(x) == 8) ? cce__debruijnToBitPosition64[(uint64_t)(cceKeepHighBitInt64(x) * cce__debruijnNumberLS6) >> 58] : cce__debruijnToBitPosition32[(uint32_t)(cceKeepHighBitInt32(x) * cce__debruijnNumberLS5) >> 27])

#define CCE_GET_EXPONENT_ABOVE_NUMBER(x) \
((sizeof(x) == 8) ? cce__debruijnToBitPosition64[(uint64_t)(cceCeilToPowerOfTwoInt64(x) * cce__debruijnNumberLS6) >> 58] : cce__debruijnToBitPosition32[(uint32_t)(cceCeilToPowerOfTwoInt32(x) * cce__debruijnNumberLS5) >> 27])

#define  CCE_DO_ARGS1(action, fn, a) fn(a)
#define  CCE_DO_ARGS2(action, fn, a, b) (fn(a) action fn(b))
#define  CCE_DO_ARGS3(action, fn, a, ...) (fn(a) action  CCE_DO_ARGS2(action, fn, __VA_ARGS__))
#define  CCE_DO_ARGS4(action, fn, a, ...) (fn(a) action  CCE_DO_ARGS3(action, fn, __VA_ARGS__))
#define  CCE_DO_ARGS5(action, fn, a, ...) (fn(a) action  CCE_DO_ARGS4(action, fn, __VA_ARGS__))
#define  CCE_DO_ARGS6(action, fn, a, ...) (fn(a) action  CCE_DO_ARGS5(action, fn, __VA_ARGS__))
#define  CCE_DO_ARGS7(action, fn, a, ...) (fn(a) action  CCE_DO_ARGS6(action, fn, __VA_ARGS__))
#define  CCE_DO_ARGS8(action, fn, a, ...) (fn(a) action  CCE_DO_ARGS7(action, fn, __VA_ARGS__))
#define  CCE_DO_ARGS9(action, fn, a, ...) (fn(a) action  CCE_DO_ARGS8(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS10(action, fn, a, ...) (fn(a) action  CCE_DO_ARGS9(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS11(action, fn, a, ...) (fn(a) action CCE_DO_ARGS10(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS12(action, fn, a, ...) (fn(a) action CCE_DO_ARGS11(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS13(action, fn, a, ...) (fn(a) action CCE_DO_ARGS12(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS14(action, fn, a, ...) (fn(a) action CCE_DO_ARGS13(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS15(action, fn, a, ...) (fn(a) action CCE_DO_ARGS14(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS16(action, fn, a, ...) (fn(a) action CCE_DO_ARGS15(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS17(action, fn, a, ...) (fn(a) action CCE_DO_ARGS16(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS18(action, fn, a, ...) (fn(a) action CCE_DO_ARGS17(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS19(action, fn, a, ...) (fn(a) action CCE_DO_ARGS18(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS20(action, fn, a, ...) (fn(a) action CCE_DO_ARGS19(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS21(action, fn, a, ...) (fn(a) action CCE_DO_ARGS20(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS22(action, fn, a, ...) (fn(a) action CCE_DO_ARGS21(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS23(action, fn, a, ...) (fn(a) action CCE_DO_ARGS22(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS24(action, fn, a, ...) (fn(a) action CCE_DO_ARGS23(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS25(action, fn, a, ...) (fn(a) action CCE_DO_ARGS24(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS26(action, fn, a, ...) (fn(a) action CCE_DO_ARGS25(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS27(action, fn, a, ...) (fn(a) action CCE_DO_ARGS26(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS28(action, fn, a, ...) (fn(a) action CCE_DO_ARGS27(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS29(action, fn, a, ...) (fn(a) action CCE_DO_ARGS28(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS30(action, fn, a, ...) (fn(a) action CCE_DO_ARGS29(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS31(action, fn, a, ...) (fn(a) action CCE_DO_ARGS30(action, fn, __VA_ARGS__))
#define CCE_DO_ARGS32(action, fn, a, ...) (fn(a) action CCE_DO_ARGS31(action, fn, __VA_ARGS__))

#define CCE_DO_ARGS(action, fn, ...) CCE_DO_ARGS ## CCE_COUNT_ARGS(__VA_ARGS__)(action, fn, __VA_ARGS__)

#define CCE_COMPLIT_TO_MACRO(type, ...) type, ((type){__VA_ARGS__})

#define CCE_STATIC_ARRAY_LENGTH(x) sizeof(x)/sizeof(x[0])

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

CCE_API CCE_NOALIAS_FN int cce__uint32PtrCompare (const void *_a, const void *_b);

#define CCE_INSERT_INTO_UID_ARRAY(_elem, _arr, _sarrp, _len) \
do \
{ \
   (_arr)[_len] = _elem; \
   uint32_t *_ptr = &(_arr)[_len]; \
   uint32_t **_uidPosition = (uint32_t**)cceBinarySearchFirst((void*)&(_ptr), (void*)(_sarrp), _len, sizeof(uint32_t*), cce__uint32PtrCompare); \
   if (_uidPosition != _sarrp + _len) \
      memmove((_uidPosition) + 1, _uidPosition, ((_len) - (*_uidPosition - (_arr))) * sizeof(uint32_t*)); \
   *(_uidPosition) = &(_arr)[_len]; \
} \
while (0)
   
#define CCE_FIND_FROM_UID_ARRAY(_elem, _arr, _sarrp, _len, _result, _err) CCE_FIND_UID_FROM_ARRAY(_elem, _arr, _sarrp, _len, uint32_t, cce__uint32PtrCompare, _result, _err)
   
#define CCE_FIND_UID_FROM_ARRAY(_elem, _arr, _sarrp, _len, _elemtype, _cmp, _result, _err) \
do \
{ \
   _elemtype *_key = &(_elem); \
   _elemtype **_position = (_elemtype**)bsearch((void*)&_key, (void*)(_sarrp), _len, sizeof(_elemtype*), _cmp); \
   if (_position == NULL) \
   { \
      _err; \
   } \
   _result = *_position - (_arr); \
} \
while (0)

#define CCE_REALLOC_UID_ARRAY(_arr, _sarrp, _len, _newSize) \
uint32_t *_oldPtr = _arr; \
_arr = realloc(_arr, _newSize * sizeof(uint32_t)); \
_sarrp = realloc(_sarrp, _newSize * sizeof(uint32_t*)); \
intptr_t _diff = _arr - _oldPtr; \
/* Pointers became invalid, update 'em! (Workaround) */ \
for (uint32_t **_it = _sarrp, **_end = _sarrp + _len; _it < _end; ++_it) \
   *_it += _diff


struct UnicodeCharWithSize
{
   uint32_t ch;
   uint32_t size;
};

typedef int cce_cmpfun(const void *a, const void *b);

/* Binary search a sorted array. If array contains multiple elements matching the key, the first one is returned. If array does not contain the key, returns the element instead of which key should be inserted. */
CCE_API CCE_NOALIAS_FN void*    cceBinarySearchFirst (const void *key, const void *arr, size_t arr_len, size_t elem_size, cce_cmpfun cmp);
/* Binary search a sorted array. If array contains multiple elements matching the key, the last one is returned. If array does not contain the key, returns the element instead of which key should be inserted. */
CCE_API CCE_NOALIAS_FN void*    cceBinarySearchLast (const void *key, const void *arr, size_t arr_len, size_t elem_size, cce_cmpfun cmp);
/* Lineary search an array from the first to the last element. If array does not contain the key, returns lastElement + 1 */
CCE_API CCE_NOALIAS_FN void*    cceLinearSearch (const void *key, const void *arr, size_t arr_len, size_t elem_size, cce_cmpfun cmp);
CCE_API void*    cceReverseMemory (void *memory, size_t size);
CCE_API CCE_NOALIAS_FN uint32_t cceGetCharSizeUTF8 (const unsigned char *ch);
CCE_API CCE_NOALIAS_FN uint32_t cceGetCharUTF8 (const unsigned char *ch);
CCE_API CCE_NOALIAS_FN struct UnicodeCharWithSize cceGetCharWithSizeUTF8 (const unsigned char *ch);
CCE_API CCE_NOALIAS_FN uint32_t cceGetCharFromStringUTF8 (const char *string, size_t position);
CCE_API CCE_CONST_FN   uint8_t  cceCeilToPowerOfTwoInt8 (uint8_t x);
CCE_API CCE_CONST_FN   uint16_t cceCeilToPowerOfTwoInt16 (uint16_t x);
CCE_API CCE_CONST_FN   uint32_t cceCeilToPowerOfTwoInt32 (uint32_t x);
CCE_API CCE_CONST_FN   uint64_t cceCeilToPowerOfTwoInt64 (uint64_t x);
CCE_API CCE_CONST_FN   uint8_t  cceU8Pow  (uint8_t  base, uint8_t  exponent);
CCE_API CCE_CONST_FN   uint16_t cceU16Pow (uint16_t base, uint16_t exponent);
CCE_API CCE_CONST_FN   uint32_t cceU32Pow (uint32_t base, uint32_t exponent);
CCE_API CCE_CONST_FN   uint64_t cceU64Pow (uint64_t base, uint64_t exponent);
CCE_API CCE_CONST_FN   uint8_t  cceKeepHighBitInt8 (uint8_t x);
CCE_API CCE_CONST_FN   uint16_t cceKeepHighBitInt16 (uint16_t x);
CCE_API CCE_CONST_FN   uint32_t cceKeepHighBitInt32 (uint32_t x);
CCE_API CCE_CONST_FN   uint64_t cceKeepHighBitInt64 (uint64_t x);
CCE_API CCE_CONST_FN   float    cceFastSinInt8 (uint8_t x);
CCE_API char* cceConvertIntToBase64String (uintmax_t number, char *buffer, uint8_t symbolsQuantity);

/* Accepts a name with a length of at most 7 characters (not necessarily '\0'-terminated), consisting of:
 * latin letters (case-insensitive), JQXZ are not differentiated between!,
 * numbers (not encoded separately from letters!),
 * any other ASCII character is encoded as space (shared with letter K!)
 */
CCE_API CCE_NOALIAS_FN uint32_t cceNameToUID (const char *name);
CCE_API CCE_CONST_FN char* cceUIDToName (uint32_t uid);
CCE_API CCE_CONST_FN uint32_t cceUIDToHash (uint32_t uid, uint32_t powerOfTwoSize);

CCE_API size_t                 cceStringToLowercase (char *str);
CCE_API void                   cceMemoryToLowercase (char *str, size_t size);
CCE_API size_t                 cceStringToUppercase (char *str);
CCE_API void                   cceMemoryToUppercase (char *str, size_t size);
CCE_API CCE_NOALIAS_FN uint8_t cceStringToBool      (const char *str);

#define cceFastCosInt8(x) cceFastSinInt8(x + 64u)

#define CCE__STRING_TO_SXVECY(sign, signUpper, uIfUnsigned, bits, comp) \
CCE_API CCE_NOALIAS_FN struct cce_ ## sign ## bits ## vec ## comp cceStringTo ## signUpper ## bits ## Vec ## comp (const char *string)

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
