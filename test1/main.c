/*
   Conservative Creator's Engine - open source engine for making games.
   Copyright © 2020-2023 Andrey Gaivoronskiy

   This file is part of Conservative Creator's Engine.

   Copying and distribution of this file, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  This file is offered as-is,
   without any warranty.
*/

#define TESTS_QUANTITY 3lu

#include <stdint.h>
#include <stdio.h>

uint8_t tmpDirTest (void);
uint8_t appDataDirTest (void);
uint8_t utf8Test (void);
uint8_t test4 (void);

int main (int argc, char **argv)
{
   if (argc > 1)
   {
      printf("Usage: %s", argv[0]);
      return 1;
   }
   size_t testsPassed = 0u;
   testsPassed += tmpDirTest();
   testsPassed += appDataDirTest();
   testsPassed += utf8Test();
   return testsPassed != TESTS_QUANTITY;
}
