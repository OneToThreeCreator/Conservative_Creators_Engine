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

#include <stdio.h>
#include <string.h>
#include <coffeechain/engine_common.h>
#include <coffeechain/utils.h>

uint8_t truthTableTest (void)
{
   char *strings[5] = {"a", "!a", "a & b", "a & b & !c", "a & b | c"};
   uint64_t correctAnswers[5] = {0xAAAAAAAAAAAAAAAA, 0x5555555555555555, 0x8888888888888888, 0x4040404040404040, 0xEAEAEAEAEAEAEAEA};
   uint_fast16_t *operations;
   uint8_t i = 0;
   while (i < 5)
   {
      operations = cceParseStringToLogicOperations(strings[i], NULL);
      if (*operations != correctAnswers[i])
      {
         printf("Operations for %s\n Expected: 0x%lx\nGot: 0x%lx\n", strings[i], correctAnswers[i], *operations);
         break;
      }
      free(operations);
      ++i;
   }
   return i / 5;
}

#define POUND_UNICODE 0xa3
#define EURO_UNICODE 0x20ac
#define SMILE_UNICODE 0x1f600

uint8_t utf8Test (void)
{
   unsigned char dollar = '$';
   uint32_t result1 = cceGetCharUTF8(&dollar);
   if (result1 != dollar)
   {
      printf("Expected: 0x%xu\nGot: 0x%xu\n", dollar, result1);
      return 0;
   }
   // £ (U+00A3)
   unsigned char poundLE[2] = {0xc2, 0xa3}, poundBE[2] = {0xa3, 0xc2};
   result1 = cceGetCharSizeUTF8(poundLE);
   uint32_t result2 = cceGetCharSizeUTF8(poundBE);
   if (result1 != 2 || result2 != 2)
   {
      printf("Symbol 0x%x length:\nExpected: %u\nGot: (LE:) %u (BE:) %u\n", POUND_UNICODE, 2, result1, result2);
   }
   result1 = cceGetCharUTF8((unsigned char*) &poundLE);
   result2 = cceGetCharUTF8((unsigned char*) &poundBE);
   if (result1 != POUND_UNICODE || result2 != POUND_UNICODE)
   {
      printf("Expected: 0x%x\nGot: (LE:) 0x%x (BE:) 0x%x\n", POUND_UNICODE, result1, result2);
      return 0;
   }
   // € (U+20AC)
   unsigned char euroLE[3] = {0xe2, 0x82, 0xac}, euroBE[3] = {0xac, 0x82, 0xe2};
   result1 = cceGetCharSizeUTF8((unsigned char*) &euroLE);
   result2 = cceGetCharSizeUTF8((unsigned char*) &euroBE);
   if (result1 != 3 || result2 != 3)
   {
      printf("Symbol 0x%x length:\nExpected: %u\nGot: (LE:) %u (BE:) %u\n", EURO_UNICODE, 3, result1, result2);
   }
   result1 = cceGetCharUTF8((unsigned char*) &euroLE);
   result2 = cceGetCharUTF8((unsigned char*) &euroBE);
   if (result1 != EURO_UNICODE || result2 != EURO_UNICODE)
   {
      printf("Expected: 0x%x\nGot: (LE:) 0x%x (BE:) 0x%x\n", EURO_UNICODE, result1, result2);
      return 0;
   }
   // (U+1F600)
   unsigned char emojiLE[4] = {0xf0, 0x9f, 0x98, 0x80}, emojiBE[4] = {0x80, 0x98, 0x9f, 0xf0};
   result1 = cceGetCharSizeUTF8((unsigned char*) &emojiLE);
   result2 = cceGetCharSizeUTF8((unsigned char*) &emojiBE);
   if (result1 != 4 || result2 != 4)
   {
      printf("Symbol 0x%x length:\nExpected: %u\nGot: (LE:) %u (BE:) %u\n", SMILE_UNICODE, 4, result1, result2);
   }
   result1 = cceGetCharUTF8((unsigned char*) &emojiLE);
   result2 = cceGetCharUTF8((unsigned char*) &emojiBE);
   if (result1 != SMILE_UNICODE || result2 != SMILE_UNICODE)
   {
      printf("Expected: 0x%x\nGot: (LE:) 0x%x (BE:) 0x%x\n", SMILE_UNICODE, result1, result2);
      return 0;
   }
   
   char stringLE[14];
   memcpy(stringLE, &dollar, 1);
   memcpy(stringLE + 1, euroLE, 3);
   memcpy(stringLE + 4, &dollar, 1);
   memcpy(stringLE + 5, emojiLE, 4);
   memcpy(stringLE + 9, poundLE, 2);
   memcpy(stringLE + 11, poundLE, 2);
   stringLE[13] = '\0';
   char stringBE[14];
   memcpy(stringBE, &dollar, 1);
   memcpy(stringBE + 1, euroBE, 3);
   memcpy(stringBE + 4, &dollar, 1);
   memcpy(stringBE + 5, emojiBE, 4);
   memcpy(stringBE + 9, poundBE, 2);
   memcpy(stringBE + 11, poundBE, 2);
   stringBE[13] = '\0';
   
   result1 = cceGetCharFromStringUTF8(stringLE, 2);
   result2 = cceGetCharFromStringUTF8(stringBE, 2);
   if (result1 != dollar || result2 != dollar)
   {
      printf("String processing error!\nExpected: 0x%x\nGot: (LE:) 0x%x (BE:) 0x%x\n", dollar, result1, result2);
      return 0;
   }
   
   result1 = cceGetCharFromStringUTF8(stringLE, 5);
   result2 = cceGetCharFromStringUTF8(stringBE, 5);
   if (result1 != POUND_UNICODE || result2 != POUND_UNICODE)
   {
      printf("String processing error!\nExpected: 0x%x\nGot: (LE:) 0x%x (BE:) 0x%x\n", POUND_UNICODE, result1, result2);
      return 0;
   }
   
   result1 = cceGetCharFromStringUTF8(stringLE, 6);
   result2 = cceGetCharFromStringUTF8(stringBE, 6);
   if (result1 != '\0' || result2 != '\0')
   {
      printf("String processing error!\nExpected: 0x%x\nGot: (LE:) 0x%x (BE:) 0x%x\n", '\0', result1, result2);
      return 0;
   }
   return 1;
}

#define CHECK_SYMBOL(str, strlen, c, exp, cor) \
do \
{ \
   size_t pos = cceBinarySearch(str, strlen, sizeof(char), sizeof(char), c); \
   if (pos == exp) \
      ++cor; \
   else \
      printf("Binary search error!\nIn string: \"%.*s\" symbol %c should be inserted at position %lu (returned position %lu)\n", strlen, str, c, exp, pos); \
} \
while (0)

uint8_t binarySearchTest (void)
{
   size_t passed = 0;
   char alphabet[26];
   for (char *iterator = alphabet, i = 'A'; i <= 'Z'; ++i, ++iterator)
   {
      *iterator = i;
   }
   CHECK_SYMBOL(alphabet, 26, '+', 0l, passed);
   CHECK_SYMBOL(alphabet, 26, 'H', 7l, passed);
   CHECK_SYMBOL(alphabet, 26, '[', 26l, passed);
   CHECK_SYMBOL(alphabet, 26, '~', 26l, passed);
   char badArray[8];
   for (char *iterator = badArray, i = 'A', j = 0; i <= 'Z'; i += j, ++iterator, ++j)
   {
      *iterator = i;
   }
   CHECK_SYMBOL(badArray, 8, '+', 0l, passed);
   CHECK_SYMBOL(badArray, 8, 'A', 0l, passed);
   CHECK_SYMBOL(badArray, 8, 'B', 2l, passed);
   CHECK_SYMBOL(badArray, 8, 'C', 3l, passed);
   CHECK_SYMBOL(badArray, 8, 'S', 7l, passed);
   CHECK_SYMBOL(badArray, 8, '~', 8l, passed);
   char asciiPrintable[95];
   for (char *iterator = asciiPrintable, i = ' '; i <= '~'; ++i, ++iterator)
   {
      *iterator = i;
   }
   for (unsigned char i = 0, j = 0; i <= 127; j += (i >= ' '), ++i)
   {
      CHECK_SYMBOL(asciiPrintable, 95, i, (size_t) j, passed);
   }
   char small[1] = {'G'};
   CHECK_SYMBOL(small, 1, 'A', 0l, passed);
   CHECK_SYMBOL(small, 1, 'G', 0l, passed);
   CHECK_SYMBOL(small, 1, 'S', 1l, passed);
   CHECK_SYMBOL(small, 1, '~', 1l, passed);
   char *empty = NULL;
   CHECK_SYMBOL(empty, 0, 'A', 0l, passed);
   CHECK_SYMBOL(empty, 0, '~', 0l, passed);
   return passed / (4 + 6 + 128 + 4 + 2);
}

#define TESTS_QUANTITY 3

int main (int argc, char **argv)
{
   if (argc > 1)
   {
      printf("Usage: %s", argv[0]);
      exit(1);
   }
   size_t testsPassed = 0u;
   testsPassed += truthTableTest();
   testsPassed += utf8Test();
   testsPassed += binarySearchTest();
   return testsPassed < TESTS_QUANTITY;
}
