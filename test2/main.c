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
#include <coffeechain/engine_common.h>

uint8_t truthTableTest (void)
{
   char *strings[5] = {"a", "!a", "a & b", "a & b & !c", "a & b | c"};
   uint64_t correctAnswers[5] = {0xAAAAAAAAAAAAAAAA, 0x5555555555555555, 0x8888888888888888, 0x4040404040404040, 0xEAEAEAEAEAEAEAEA};
   uint_fast16_t *operations;
   uint8_t i = 0;
   while (i < 5)
   {
      operations = cceParseStringToLogicOperations(strings[i], NULL);
      printf("Operations for %s: 0x%lx\n", strings[i], *operations);
      free(operations);
      ++i;
   }
   return i / 4;
}

int main (void)
{
   truthTableTest();
   return 0;
}
