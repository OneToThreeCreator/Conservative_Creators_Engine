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

#ifndef ENDIANESS_H
#define ENDIANESS_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "cce_exports.h"

#define CCE_BIG_ENDIAN 0
#define CCE_LITTLE_ENDIAN 1

#define cceEndianess (((union {uint16_t n; uint8_t a[2];}) {1}).a[0])

#define cceSwapEndianInt16(value) (((value) << 8)  | ((value) >> 8))
#define cceSwapEndianInt32(value) (((value) << 24) | ((value) >> 24) | (((value) << 8) & 0xFF0000) | (((value) >> 8) & 0xFF00))
#define cceSwapEndianInt64(value) \
(((value) << 56) | (((value) << 40) & 0xFF000000000000ULL) | (((value) << 24) & 0xFF0000000000ULL) | (((value) << 8) & 0xFF00000000ULL) | \
 ((value) >> 56) | (((value) >> 40) & 0xFF00)              | (((value) >> 24) & 0xFF0000)          | (((value) >> 8) & 0xFF000000))
#define cce__swapEndianNewArrayIntN(newarray, array, arraySize, n) \
do \
{ \
   for (size_t IT = 0; IT < arraySize; ++IT) \
   { \
      (newarray)[IT] = cceSwapEndianInt ## n((array)[IT]); \
   } \
} \
while (0)

#define cceSwapEndianArrayInt16(array, arraySize) cce__swapEndianNewArrayIntN(array, array, arraySize, 16)
#define cceSwapEndianArrayInt32(array, arraySize) cce__swapEndianNewArrayIntN(array, array, arraySize, 32)
#define cceSwapEndianArrayInt64(array, arraySize) cce__swapEndianNewArrayIntN(array, array, arraySize, 64)

#define cceSwapEndianNewArrayInt16(newarray, array, arraySize) cce__swapEndianNewArrayIntN(newarray, array, arraySize, 16)
#define cceSwapEndianNewArrayInt32(newarray, array, arraySize) cce__swapEndianNewArrayIntN(newarray, array, arraySize, 32)
#define cceSwapEndianNewArrayInt64(newarray, array, arraySize) cce__swapEndianNewArrayIntN(newarray, array, arraySize, 64)

CCE_API void cceSwapEndianArrayIntN (void *array, size_t arraySize, size_t n);
CCE_API void cceSwapEndianNewArrayIntN (void *newArray, const void *array, size_t arraySize, size_t n);

#define cce__littleEndianConversionIntN(value, n) ((cceEndianess == CCE_BIG_ENDIAN)    ? cceSwapEndianInt ## n(value) : value)
#define cce__bigEndianConversionIntN(value, n)    ((cceEndianess == CCE_LITTLE_ENDIAN) ? cceSwapEndianInt ## n(value) : value)

#define cce__littleEndianConversionArrayIntN(array, arraySize, n) \
if (cceEndianess == CCE_BIG_ENDIAN) \
   cce__swapEndianNewArrayIntN(array, array, arraySize, n)

#define cce__bigEndianConversionArrayIntN(array, arraySize, n) \
if (cceEndianess == CCE_LITTLE_ENDIAN) \
   cce__swapEndianNewArrayIntN(array, array, arraySize, n)

#define cce__littleEndianConversionNewArrayIntN(newarray, array, arraySize, n) \
if (cceEndianess == CCE_BIG_ENDIAN) \
   cce__swapEndianNewArrayIntN(newarray, array, arraySize, n); \
else \
   memcpy(newarray, array, arraySize * (n >> 3))

#define cce__bigEndianConversionNewArrayIntN(newarray, array, arraySize, n) \
if (cceEndianess == CCE_LITTLE_ENDIAN) \
   cce__swapEndianNewArrayIntN(newarray, array, arraySize, n); \
else \
   memcpy(newarray, array, arraySize * (n >> 3))

#define cceLittleEndianConversionArrayIntN(array, arraySize, n) \
if (cceEndianess == CCE_BIG_ENDIAN) \
   cceSwapEndianArrayIntN(array, arraySize, n)

#define cceLittleEndianConversionNewArrayIntN(newarray, array, arraySize, n) \
if (cceEndianess == CCE_BIG_ENDIAN) \
   cceSwapEndianNewArrayIntN(newarray, array, arraySize, n); \
else \
   memcpy(newarray, array, arraySize * (n >> 3))

#define cceBigEndianConversionArrayIntN(array, arraySize, n) \
if (cceEndianess == CCE_LITTLE_ENDIAN) \
   cceSwapEndianArrayIntN(array, arraySize, n)

#define cceBigEndianConversionNewArrayIntN(newarray, array, arraySize, n) \
if (cceEndianess == CCE_LITTLE_ENDIAN) \
   cceSwapEndianNewArrayIntN(newarray, array, arraySize, n); \
else \
   memcpy(newarray, array, arraySize * (n >> 3))

#define cceBigEndianToHostEndianInt16(value) cce__bigEndianConversionIntN(value, 16)
#define cceHostEndianToBigEndianInt16(value) cce__bigEndianConversionIntN(value, 16)
#define cceBigEndianToHostEndianInt32(value) cce__bigEndianConversionIntN(value, 32)
#define cceHostEndianToBigEndianInt32(value) cce__bigEndianConversionIntN(value, 32)
#define cceBigEndianToHostEndianInt64(value) cce__bigEndianConversionIntN(value, 64)
#define cceHostEndianToBigEndianInt64(value) cce__bigEndianConversionIntN(value, 64)

#define cceBigEndianToHostEndianArrayInt16(array, size)   cce__bigEndianConversionArrayIntN(array, size, 16)
#define cceHostEndianToBigEndianArrayInt16(array, size)   cce__bigEndianConversionArrayIntN(array, size, 16)
#define cceBigEndianToHostEndianArrayInt32(array, size)   cce__bigEndianConversionArrayIntN(array, size, 32)
#define cceHostEndianToBigEndianArrayInt32(array, size)   cce__bigEndianConversionArrayIntN(array, size, 32)
#define cceBigEndianToHostEndianArrayInt64(array, size)   cce__bigEndianConversionArrayIntN(array, size, 64)
#define cceHostEndianToBigEndianArrayInt64(array, size)   cce__bigEndianConversionArrayIntN(array, size, 64)
#define cceBigEndianToHostEndianArrayIntN(array, size, n) cceBigEndianConversionArrayIntN(array, size, n)
#define cceHostEndianToBigEndianArrayIntN(array, size, n) cceBigEndianConversionArrayIntN(array, size, n)

#define cceBigEndianToHostEndianNewArrayInt16(dest, src, size)   cce__bigEndianConversionNewArrayIntN(dest, src, size, 16)
#define cceHostEndianToBigEndianNewArrayInt16(dest, src, size)   cce__bigEndianConversionNewArrayIntN(dest, src, size, 16)
#define cceBigEndianToHostEndianNewArrayInt32(dest, src, size)   cce__bigEndianConversionNewArrayIntN(dest, src, size, 32)
#define cceHostEndianToBigEndianNewArrayInt32(dest, src, size)   cce__bigEndianConversionNewArrayIntN(dest, src, size, 32)
#define cceBigEndianToHostEndianNewArrayInt64(dest, src, size)   cce__bigEndianConversionNewArrayIntN(dest, src, size, 64)
#define cceHostEndianToBigEndianNewArrayInt64(dest, src, size)   cce__bigEndianConversionNewArrayIntN(dest, src, size, 64)
#define cceBigEndianToHostEndianNewArrayIntN(dest, src, size, n) cceBigEndianConversionNewArrayIntN(dest, src, size, n)
#define cceHostEndianToBigEndianNewArrayIntN(dest, src, size, n) cceBigEndianConversionNewArrayIntN(dest, src, size, n)

#define cceLittleEndianToHostEndianInt16(value) cce__littleEndianConversionIntN(value, 16)
#define cceHostEndianToLittleEndianInt16(value) cce__littleEndianConversionIntN(value, 16)
#define cceLittleEndianToHostEndianInt32(value) cce__littleEndianConversionIntN(value, 32)
#define cceHostEndianToLittleEndianInt32(value) cce__littleEndianConversionIntN(value, 32)
#define cceLittleEndianToHostEndianInt64(value) cce__littleEndianConversionIntN(value, 64)
#define cceHostEndianToLittleEndianInt64(value) cce__littleEndianConversionIntN(value, 64)

#define cceHostEndianToLittleEndianArrayInt16(array, size)   cce__littleEndianConversionArrayIntN(array, size, 16)
#define cceLittleEndianToHostEndianArrayInt16(array, size)   cce__littleEndianConversionArrayIntN(array, size, 16)
#define cceHostEndianToLittleEndianArrayInt32(array, size)   cce__littleEndianConversionArrayIntN(array, size, 32)
#define cceLittleEndianToHostEndianArrayInt32(array, size)   cce__littleEndianConversionArrayIntN(array, size, 32)
#define cceHostEndianToLittleEndianArrayInt64(array, size)   cce__littleEndianConversionArrayIntN(array, size, 64)
#define cceLittleEndianToHostEndianArrayInt64(array, size)   cce__littleEndianConversionArrayIntN(array, size, 64)
#define cceHostEndianToLittleEndianArrayIntN(array, size, n) cceLittleEndianConversionArrayIntN(array, size, n)
#define cceLittleEndianToHostEndianArrayIntN(array, size, n) cceLittleEndianConversionArrayIntN(array, size, n)

#define cceHostEndianToLittleEndianNewArrayInt16(dest, src, size)   cce__littleEndianConversionNewArrayIntN(dest, src, size, 16)
#define cceLittleEndianToHostEndianNewArrayInt16(dest, src, size)   cce__littleEndianConversionNewArrayIntN(dest, src, size, 16)
#define cceHostEndianToLittleEndianNewArrayInt32(dest, src, size)   cce__littleEndianConversionNewArrayIntN(dest, src, size, 32)
#define cceLittleEndianToHostEndianNewArrayInt32(dest, src, size)   cce__littleEndianConversionNewArrayIntN(dest, src, size, 32)
#define cceHostEndianToLittleEndianNewArrayInt64(dest, src, size)   cce__littleEndianConversionNewArrayIntN(dest, src, size, 64)
#define cceLittleEndianToHostEndianNewArrayInt64(dest, src, size)   cce__littleEndianConversionNewArrayIntN(dest, src, size, 64)
#define cceHostEndianToLittleEndianNewArrayIntN(dest, src, size, n) cceLittleEndianConversionNewArrayIntN(dest, src, size, n)
#define cceLittleEndianToHostEndianNewArrayIntN(dest, src, size, n) cceLittleEndianConversionNewArrayIntN(dest, src, size, n)

#define cceBigEndianToLittleEndianInt16(value) cceSwapEndianInt16(value)
#define cceLittleEndianToBigEndianInt16(value) cceSwapEndianInt16(value)
#define cceBigEndianToLittleEndianInt32(value) cceSwapEndianInt32(value)
#define cceLittleEndianToBigEndianInt32(value) cceSwapEndianInt32(value)
#define cceBigEndianToLittleEndianInt64(value) cceSwapEndianInt64(value)
#define cceLittleEndianToBigEndianInt64(value) cceSwapEndianInt64(value)

#define cceBigEndianToLittleEndianArrayInt16(array, size)   cceSwapEndianArrayInt16(array, size)
#define cceLittleEndianToBigEndianArrayInt16(array, size)   cceSwapEndianArrayInt16(array, size)
#define cceBigEndianToLittleEndianArrayInt32(array, size)   cceSwapEndianArrayInt32(array, size)
#define cceLittleEndianToBigEndianArrayInt32(array, size)   cceSwapEndianArrayInt32(array, size)
#define cceBigEndianToLittleEndianArrayInt64(array, size)   cceSwapEndianArrayInt64(array, size)
#define cceLittleEndianToBigEndianArrayInt64(array, size)   cceSwapEndianArrayInt64(array, size)
#define cceBigEndianToLittleEndianArrayIntN(array, size, n) cceSwapEndianArrayIntN(array, size, n)
#define cceLittleEndianToBigEndianArrayIntN(array, size, n) cceSwapEndianArrayIntN(array, size, n)

#define cceBigEndianToLittleEndianNewArrayInt16(dest, src, size)   cceSwapEndianNewArrayInt16(dest, src, size)
#define cceLittleEndianToBigEndianNewArrayInt16(dest, src, size)   cceSwapEndianNewArrayInt16(dest, src, size)
#define cceBigEndianToLittleEndianNewArrayInt32(dest, src, size)   cceSwapEndianNewArrayInt32(dest, src, size)
#define cceLittleEndianToBigEndianNewArrayInt32(dest, src, size)   cceSwapEndianNewArrayInt32(dest, src, size)
#define cceBigEndianToLittleEndianNewArrayInt64(dest, src, size)   cceSwapEndianNewArrayInt64(dest, src, size)
#define cceLittleEndianToBigEndianNewArrayInt64(dest, src, size)   cceSwapEndianNewArrayInt64(dest, src, size)
#define cceBigEndianToLittleEndianNewArrayIntN(dest, src, size, n) cceSwapEndianNewArrayIntN(dest, src, size, n)
#define cceLittleEndianToBigEndianNewArrayIntN(dest, src, size, n) cceSwapEndianNewArrayIntN(dest, src, size, n)

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ENDIANESS_H
