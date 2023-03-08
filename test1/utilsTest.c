/*
   Conservative Creator's Engine - open source engine for making games.
   Copyright © 2020-2023 Andrey Gaivoronskiy

   This file is part of Conservative Creator's Engine.

   Copying and distribution of this file, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  This file is offered as-is,
   without any warranty.
*/

#include <stdio.h>
#include <string.h>
#include <cce/engine_common.h>
#include <cce/utils.h>

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
