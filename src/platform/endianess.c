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

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "../engine_common.h"
#include "endianess.h"
#include <stdio.h>

CCE_PUBLIC_OPTIONS uint16_t (*cceLittleEndianConversionInt16) (uint16_t) = NULL;
CCE_PUBLIC_OPTIONS uint32_t (*cceLittleEndianConversionInt32) (uint32_t) = NULL;
CCE_PUBLIC_OPTIONS uint64_t (*cceLittleEndianConversionInt64) (uint64_t) = NULL;
CCE_PUBLIC_OPTIONS void* (*cceLittleEndianConversionArrayIntN)    (void*, size_t, size_t)        = NULL;
CCE_PUBLIC_OPTIONS void* (*cceLittleEndianConversionNewArrayIntN) (void*, const void*, size_t, size_t) = NULL;
CCE_PUBLIC_OPTIONS uint16_t (*cceBigEndianConversionInt16) (uint16_t) = NULL;
CCE_PUBLIC_OPTIONS uint32_t (*cceBigEndianConversionInt32) (uint32_t) = NULL;
CCE_PUBLIC_OPTIONS uint64_t (*cceBigEndianConversionInt64) (uint64_t) = NULL;
CCE_PUBLIC_OPTIONS void* (*cceBigEndianConversionArrayIntN)    (void*, size_t, size_t)        = NULL;
CCE_PUBLIC_OPTIONS void* (*cceBigEndianConversionNewArrayIntN) (void*, const void*, size_t, size_t) = NULL;

// 0 - Big Endian, 1 - Little Endian
static cce_endianess endianess;
CCE_PUBLIC_OPTIONS const cce_endianess *const g_endianess = &endianess;

CCE_PUBLIC_OPTIONS uint16_t cceSwapEndianInt16 (uint16_t value)
{
   uint8_t *bytes = (uint8_t*) &value;
   register uint8_t buffer;
   buffer   = bytes[0];
   bytes[0] = bytes[1];
   bytes[1] = buffer;
   return value;
}

CCE_PUBLIC_OPTIONS uint32_t cceSwapEndianInt32 (uint32_t value)
{
   uint8_t *bytes = (uint8_t*) &value;
   register uint8_t buffer;
   buffer   = bytes[0];
   bytes[0] = bytes[3];
   bytes[3] = buffer;
   buffer   = bytes[1];
   bytes[1] = bytes[2];
   bytes[2] = buffer;
   return value;
}

CCE_PUBLIC_OPTIONS uint64_t cceSwapEndianInt64 (uint64_t value)
{
   uint8_t *bytes = (uint8_t*) &value;
   register uint8_t buffer;
   buffer   = bytes[0];
   bytes[0] = bytes[7];
   bytes[7] = buffer;
   buffer   = bytes[1];
   bytes[1] = bytes[6];
   bytes[6] = buffer;
   buffer   = bytes[2];
   bytes[2] = bytes[5];
   bytes[5] = buffer;
   buffer   = bytes[3];
   bytes[3] = bytes[4];
   bytes[4] = buffer;
   return value;
}

CCE_PUBLIC_OPTIONS void* cceSwapEndianArrayIntN (void *array, size_t arraySize, size_t n)
{
   register uint8_t buffer;
   for (uint8_t *iterator = (uint8_t*) array, *end = ((uint8_t*) array) + arraySize * n; iterator < end; iterator += n)
   {
      for (size_t i = 0; i < (n / 2); ++i)
      {
         buffer              = iterator[i];
         iterator[i]         = iterator[n - i - 1];
         iterator[n - i - 1] = buffer;
      }
   }
   return array;
}

CCE_PUBLIC_OPTIONS void* cceSwapEndianNewArrayIntN (void *newArray, const void *array, size_t arraySize, size_t n)
{
   for (uint8_t *iterator = (uint8_t*) array, *jiterator = (uint8_t*) newArray, *end = ((uint8_t*) array) + arraySize * n;
        iterator < end; iterator += n)
   {
      for (size_t i = 0; i < (n / 2); ++i)
      {
         jiterator[n - i - 1] = iterator[i];
         jiterator[i]         = iterator[n - i - 1];
      }
   }
   return newArray;
}

static uint16_t ccePreserveEndianInt16 (uint16_t value)
{
   return value;
}

static uint32_t ccePreserveEndianInt32 (uint32_t value)
{
   return value;
}

static uint64_t ccePreserveEndianInt64 (uint64_t value)
{
   return value;
}

CCE_PUBLIC_OPTIONS void* ccePreserveEndianArrayIntN (void *array, size_t arraySize, size_t n)
{
   return array;
}

CCE_PUBLIC_OPTIONS void* ccePreserveEndianNewArrayIntN (void *newArray, const void *array, size_t arraySize, size_t n)
{
   memcpy(newArray, array, arraySize * n);
   return newArray;
}

static inline uint8_t getEndianess (void)
{
   uint16_t a = 1;
   uint8_t *b = (uint8_t*) &a;
   return a == *b;
}

void cce__initEndianConversion (void)
{
   endianess = getEndianess();
   if (endianess == CCE_BIG_ENDIAN)
   {
      cceLittleEndianConversionInt16 = cceSwapEndianInt16;
      cceLittleEndianConversionInt32 = cceSwapEndianInt32;
      cceLittleEndianConversionInt64 = cceSwapEndianInt64;
      cceLittleEndianConversionArrayIntN = cceSwapEndianArrayIntN;
      cceLittleEndianConversionNewArrayIntN = cceSwapEndianNewArrayIntN;
      cceBigEndianConversionInt16 = ccePreserveEndianInt16;
      cceBigEndianConversionInt32 = ccePreserveEndianInt32;
      cceBigEndianConversionInt64 = ccePreserveEndianInt64;
      cceBigEndianConversionArrayIntN = ccePreserveEndianArrayIntN;
      cceBigEndianConversionNewArrayIntN = ccePreserveEndianNewArrayIntN;
   }
   else
   {
      cceLittleEndianConversionInt16 = ccePreserveEndianInt16;
      cceLittleEndianConversionInt32 = ccePreserveEndianInt32;
      cceLittleEndianConversionInt64 = ccePreserveEndianInt64;
      cceLittleEndianConversionArrayIntN = ccePreserveEndianArrayIntN;
      cceLittleEndianConversionNewArrayIntN = ccePreserveEndianNewArrayIntN;
      cceBigEndianConversionInt16 = cceSwapEndianInt16;
      cceBigEndianConversionInt32 = cceSwapEndianInt32;
      cceBigEndianConversionInt64 = cceSwapEndianInt64;
      cceBigEndianConversionArrayIntN = cceSwapEndianArrayIntN;
      cceBigEndianConversionNewArrayIntN = cceSwapEndianNewArrayIntN;
   }
}
