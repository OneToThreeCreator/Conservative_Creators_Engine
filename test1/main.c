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
