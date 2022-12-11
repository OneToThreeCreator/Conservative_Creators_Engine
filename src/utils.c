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

#include <ctype.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#include "../include/cce/engine_common.h"
#include "../include/cce/endianess.h"
#include "../include/cce/utils.h"


const uint8_t cce__charType[128] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    CCE__CHAR_WHITESPACE_LIKE, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CCE__CHAR_DELIMITER, CCE__CHAR_WHITESPACE_LIKE, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CCE__CHAR_DELIMITER, CCE__CHAR_DELIMITER, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, CCE__CHAR_WHITESPACE_LIKE,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
// Generated
const uint8_t cce__debruijnToBitPosition64[64] = {63, 0,  47, 1,  56, 48, 27, 2,  60, 57, 49, 41, 37, 28, 16, 3,
                                                  61, 54, 58, 35, 52, 50, 42, 21, 44, 38, 32, 29, 23, 17, 11, 4,
                                                  62, 46, 55, 26, 59, 40, 36, 15, 53, 34, 51, 20, 43, 31, 22, 10,
                                                  45, 25, 39, 14, 33, 19, 30, 9,  24, 13, 18, 8,  12, 7,  6,  5};

const uint8_t cce__debruijnToBitPosition32[32] = {31, 0,  22, 1,  28, 23, 13, 2,  29, 26, 24, 17, 19, 14, 9,  3,
                                                  30, 21, 27, 12, 25, 16, 18, 8,  20, 11, 15, 7,  10, 6,  5,  4};

const uint64_t cce__debruijnNumberLS6 = 0x7ef3ae369961512;
const uint32_t cce__debruijnNumberLS5 = 0xfb9ac52;

CCE_PUBLIC_OPTIONS char* cceReverseMemory (char *memory, size_t size)
{
   size_t wordHalfQuantity = size / (sizeof(size_t) * 2);
   size_t *iterator = (size_t*) memory, *jiterator = (size_t*) (memory + size), *end = ((size_t*) memory) + wordHalfQuantity;
   while (iterator < end)
   {
      size_t tmp = *iterator;
      if (sizeof(size_t) == sizeof(uint64_t))
      {
         *iterator = cceSwapEndianInt64(*jiterator);
         *jiterator = cceSwapEndianInt64(tmp);
      }
      else
      {
         *iterator = cceSwapEndianInt32(*jiterator);
         *jiterator = cceSwapEndianInt32(tmp);
      }
      ++iterator;
      --jiterator;
   }
   for (unsigned char *it = (unsigned char*) iterator, *jit = (unsigned char*) jiterator; it < jit; ++it, --jit)
   {
      unsigned char tmp = *it;
      *it = *jit;
      *jit = tmp;
   }
   return memory;
}

#define CCE_POW(base, exponent, type) \
{ \
   type result = 1; \
   for (;;) \
   { \
      if (exponent & 0x1) \
         result *= base; \
      exponent >>= 1; \
      if (exponent == 0) \
         return result; \
      base *= base; \
   } \
}

CCE_PUBLIC_OPTIONS uint8_t cceU8Pow (uint8_t base, uint8_t exponent)
   CCE_POW(base, exponent, uint8_t)
   
CCE_PUBLIC_OPTIONS uint16_t cceU16Pow (uint16_t base, uint16_t exponent)
   CCE_POW(base, exponent, uint16_t)

CCE_PUBLIC_OPTIONS uint32_t cceU32Pow (uint32_t base, uint32_t exponent)
   CCE_POW(base, exponent, uint32_t)
   
CCE_PUBLIC_OPTIONS uint64_t cceU64Pow (uint64_t base, uint64_t exponent)
   CCE_POW(base, exponent, uint64_t)

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

CCE_PUBLIC_OPTIONS char* cceConvertIntToBase64String (size_t number, char *buffer, uint8_t symbolsQuantity)
{
   static const char
   dictionary[64] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                     'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                     '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'};
   for (uint8_t i = 0u; i < symbolsQuantity; ++i)
   {
      *(buffer + symbolsQuantity - i - 1u) = *(dictionary + ((number >> (i * 6)) & 63));
   }
   return buffer;
}

CCE_PUBLIC_OPTIONS size_t cceStringToLowercase (char *str)
{
   unsigned char *iterator = (unsigned char*) str;
   for (; *iterator != '\0'; ++iterator)
   {
      *iterator = tolower(*iterator);
   }
   return (char*)iterator - str;
}

CCE_PUBLIC_OPTIONS void cceMemoryToLowercase (char *str, size_t size)
{
   unsigned char *iterator = (unsigned char*) str;
   for (unsigned char *end = iterator + size; iterator < end; ++iterator)
   {
      *iterator = tolower(*iterator);
   }
}

CCE_PUBLIC_OPTIONS size_t cceStringToUppercase (char *str)
{
   unsigned char *iterator = (unsigned char*) str;
   for (; *iterator != '\0'; ++iterator)
   {
      *iterator = toupper(*iterator);
   }
   return (char*)iterator - str;
}

CCE_PUBLIC_OPTIONS void cceMemoryToUppercase (char *str, size_t size)
{
   unsigned char *iterator = (unsigned char*) str;
   for (unsigned char *end = iterator + size; iterator < end; ++iterator)
   {
      *iterator = toupper(*iterator);
   }
}

CCE_PUBLIC_OPTIONS uint8_t cceStringToBool (const char *str)
{
   if (str == NULL)
      return 0;
   char buf[8] = {0};
   strncpy(buf, str, 8);
   cceMemoryToLowercase(buf, 7);
   
   if (CCE_STREQ(buf, "true") || CCE_STREQ(buf, "yes") || CCE_STREQ(buf, "enable") || CCE_STREQ(buf, "allow") || CCE_STREQ(buf, "+"))
      return 1;
   
   if (CCE_STREQ(buf, "false") || CCE_STREQ(buf, "no") || CCE_STREQ(buf, "disable") || CCE_STREQ(buf, "deny") || CCE_STREQ(buf, "-"))
      return 0;
   
   return atoi(str) > 0;
}

CCE_PUBLIC_OPTIONS uint8_t cceCeilToPowerOfTwoInt8 (uint8_t x)
{
   return CCE_CEIL_TO_POWER_OF_TWO(x, x);
}
CCE_PUBLIC_OPTIONS uint16_t cceCeilToPowerOfTwoInt16 (uint16_t x)
{
   return CCE_CEIL_TO_POWER_OF_TWO(x, x);
}
CCE_PUBLIC_OPTIONS uint32_t cceCeilToPowerOfTwoInt32 (uint32_t x)
{
   return CCE_CEIL_TO_POWER_OF_TWO(x, x);
}
CCE_PUBLIC_OPTIONS uint64_t cceCeilToPowerOfTwoInt64 (uint64_t x)
{
   return CCE_CEIL_TO_POWER_OF_TWO(x, x);
}
CCE_PUBLIC_OPTIONS uint8_t  cceKeepHighBitInt8 (uint8_t x)
{
   return CCE_KEEP_HIGH_BIT(x, x);
}
CCE_PUBLIC_OPTIONS uint16_t cceKeepHighBitInt16 (uint16_t x)
{
   return CCE_KEEP_HIGH_BIT(x, x);
}
CCE_PUBLIC_OPTIONS uint32_t cceKeepHighBitInt32 (uint32_t x)
{
   return CCE_KEEP_HIGH_BIT(x, x);
}
CCE_PUBLIC_OPTIONS uint64_t cceKeepHighBitInt64 (uint64_t x)
{
   return CCE_KEEP_HIGH_BIT(x, x);
}

float sinLookup[18] = {0.0f,        0.09801714f, 0.19509032f, 0.29028467f, 0.382683432f, 0.47139674f, 0.55557023f, 0.63439328f,
                       0.70710678f, 0.77301045f, 0.8314696f,  0.88192127f, 0.9238795f,   0.95694033f, 0.98078528f, 0.9951847f, 1.0f, 0.9951847f};

CCE_PUBLIC_OPTIONS float cceFastSinInt8 (uint8_t x)
{
   uint8_t isNegative = (x <= 128) * 2 - 1;
   x *= isNegative;
   x = (x > 64) ? 128 - x : x;
   float distance = (x&3) * 0.25f;
   x >>= 4;
   return (sinLookup[x] * (1 - distance) + sinLookup[x + 1] * distance) * isNegative;
}

#define STRTOVEC(st, type, comp, strtoXl) \
type vector[comp] = {0}; \
for (type *it = vector, *end = vector + comp; it < end; ++it) \
{ \
   for (;; ++st) \
   { \
      if (isdigit(*st) || *st == '+' || *st == '-') \
         break; \
      if (*st == '\0' || *st == '\n') \
         goto RETURN; \
   } \
   *it = strtoXl(st, (char**)&st, 10); \
} \
RETURN:

#if LLONG_MAX == INT64_MAX
#define STR_CONV_SUFFIX ll
#elif LONG_MAX == INT64_MAX
#define STR_CONV_SUFFIX l
#else
#error "long and long long aren't 64-bit types"
#endif

#define ARRAY_TO_INITIALIZER_LIST1(arr)arr[0]
#define ARRAY_TO_INITIALIZER_LIST2(arr)ARRAY_TO_INITIALIZER_LIST1(arr), arr[1]
#define ARRAY_TO_INITIALIZER_LIST3(arr)ARRAY_TO_INITIALIZER_LIST2(arr), arr[2]
#define ARRAY_TO_INITIALIZER_LIST4(arr)ARRAY_TO_INITIALIZER_LIST3(arr), arr[3]

#define STRING_TO_SXVECY(sign, signUpper, uIfUnsigned, bits, comp) \
CCE_PUBLIC_OPTIONS struct cce_ ## sign ## bits ## vec ## comp cceStringTo ## signUpper ## bits ## Vec ## comp (const char *string) \
{ \
   STRTOVEC(string, uIfUnsigned ## int ## bits ## _t, comp, CCE_MACRO_CONCAT(strto ## uIfUnsigned, STR_CONV_SUFFIX)) \
   return (struct cce_ ## sign ## bits ## vec ## comp) {ARRAY_TO_INITIALIZER_LIST ## comp(vector)}; \
}

#define STRING_TO_XVECY(bits, comp) \
STRING_TO_SXVECY(i, I,  , bits, comp) \
STRING_TO_SXVECY(u, U, u, bits, comp)

#define STRING_TO_XVEC(bits) \
STRING_TO_XVECY(bits, 1) \
STRING_TO_XVECY(bits, 2) \
STRING_TO_XVECY(bits, 3) \
STRING_TO_XVECY(bits, 4)

STRING_TO_XVEC(8)
STRING_TO_XVEC(16)
STRING_TO_XVEC(32)
STRING_TO_XVEC(64)
