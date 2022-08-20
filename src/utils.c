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
#include <stdint.h>

#include "../include/coffeechain/engine_common.h"
#include "../include/coffeechain/endianess.h"

struct UnicodeCharWithSize
{
   uint32_t ch;
   uint32_t size;
};

CCE_PUBLIC_OPTIONS size_t cceBinarySearch (const void *const array, size_t arraySize, size_t typeSize, size_t step, size_t value)
{
   if (!arraySize)
      return 0;
      
   const uint8_t *iterator = (const uint8_t*) array;
   const uint8_t *end = ((const uint8_t*) array) + arraySize * step;
   size_t remain = arraySize;
   size_t typeRemain;
   size_t typeMask;
   if (typeSize >= sizeof(size_t))
   {
      typeMask = SIZE_MAX;
   }
   else
   {
      typeMask = (((size_t) 1u) << typeSize * 8u) - 1u;
   }
   do
   {
      remain >>= 1u;
      typeRemain = remain * step;
      if (iterator + typeRemain + step > end)
         continue;
      iterator += (typeRemain + step) * (((*((size_t*) (iterator + typeRemain))) & typeMask) < value);
   }
   while (remain > 0u); /* Checking for last valid value, because it is impossible to detect underflow of unsigned variable */
   return (iterator - (uint8_t*) array) / step;
}

#define SWAP_S(x, y) \
size_t tmp = (x); \
(x) = (y); \
(y) = tmp

CCE_PUBLIC_OPTIONS char* cceReverseMemory (char *memory, size_t size)
{
   size_t wordHalfQuantity = size / (sizeof(size_t) * 2);
   size_t *iterator = (size_t*) memory, *jiterator = (size_t*) (memory + size), *end = ((size_t*) memory) + wordHalfQuantity;
   while (iterator < end)
   {
      SWAP_S(*iterator, *jiterator);
      ++iterator;
      --jiterator;
   }
   for (char *it = (char*) iterator, *jit = (char*) jiterator; it < jit; ++it, --jit)
   {
       SWAP_S(*it, *jit);
   }
   return memory;
}

// First utf-8 byte encodes it's length. 110XXXXX - length 2, 1110XXXX - length 3, 11110XXX - length 4 (max). This is branchless way of extracting this bits
#define GET_CHAR_LENGTH_UTF8LE(ch) (((*(ch) & 0x30) >> 4) + ((*(ch) & 0x30) == 0) + 1)

// Assumes first byte is last (checked already). Max symbol length is 4. In utf-8, 7-th bit is set only on first byte of the unicode sequence (this byte also encodes sequence length). 11XXXXXX on first byte, 10XXXXXX on remaining bytes
// That all means we left to check 7-th bit of 2-nd and 3-rd byte in unicode sequence to get it's length. 
#define GET_CHAR_LENGTH_UTF8BE(ch) (((*((ch) + 1) & 0x40) == 0) + ((*((uint16_t*) ((ch) + 1)) & 0x4040) == 0) + 2)

CCE_PUBLIC_OPTIONS uint32_t cceGetCharSizeUTF8 (const unsigned char *ch)
{
   switch ((*ch & 0xC0))
   {
      case 0x0:
         return 1;
      
      case 0x80:
         return GET_CHAR_LENGTH_UTF8BE(ch);
      
      case 0xC0:
         return GET_CHAR_LENGTH_UTF8LE(ch);
      
      default:
         return 0;
   }
}

CCE_PUBLIC_OPTIONS struct UnicodeCharWithSize cceGetCharWithSizeUTF8 (const unsigned char *ch)
{
   uint32_t result = 0;
   switch ((*ch & 0xC0))
   {
      case 0x00:
      case 0x40:
         return (struct UnicodeCharWithSize) {(uint32_t) *ch, 1};
      
      case 0x80:
      {
         size_t size = GET_CHAR_LENGTH_UTF8BE(ch);
         for (size_t i = 0; i < size - 1; ++i, ++ch)
         {
            result |= (((uint32_t)(*ch)) & 0x3F) << (i * 6);
         }
         result |= (((uint32_t)(*ch)) & ((0x1 << (7 - size)) - 1)) << ((size - 1) * 6);
         return (struct UnicodeCharWithSize) {result, size};
      }
      
      case 0xC0:
      {
         size_t size = GET_CHAR_LENGTH_UTF8LE(ch);
         result |= (((uint32_t)(*ch)) & ((0x1 << (7 - size)) - 1)) << ((size - 1) * 6);
         size_t i = size - 1;
         do
         {
            ++ch, --i;
            result |= (((uint32_t)(*ch)) & 0x3F) << (i * 6);
         }
         while (i != 0);
         return (struct UnicodeCharWithSize) {result, size};
      }
      
      default:
         return (struct UnicodeCharWithSize) {0, 0};
   }
}

CCE_PUBLIC_OPTIONS uint32_t cceGetCharUTF8 (const unsigned char *ch)
{
   return cceGetCharWithSizeUTF8(ch).ch;
}

CCE_PUBLIC_OPTIONS uint32_t cceGetCharFromStringUTF8 (const char *string, size_t position)
{   
   const unsigned char *str = (const unsigned char*) string;
   for (size_t i = 0; i < position; ++i)
   {
      str += cceGetCharSizeUTF8(str);
   }
   return cceGetCharUTF8(str);
}
