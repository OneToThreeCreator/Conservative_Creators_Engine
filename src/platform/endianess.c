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

#include <string.h>
#include <stdlib.h>
#include <stdint.h>

#include "../../include/cce/endianess.h"

CCE_API void cceSwapEndianArrayIntN (void *array, size_t arraySize, size_t n)
{
   n >>= 3;
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
}

CCE_API void cceSwapEndianNewArrayIntN (void *newArray, const void *array, size_t arraySize, size_t n)
{
   n >>= 3;
   for (uint8_t *iterator = (uint8_t*) array, *jiterator = (uint8_t*) newArray, *end = ((uint8_t*) array) + arraySize * n;
        iterator < end; iterator += n)
   {
      for (size_t i = 0; i < (n / 2); ++i)
      {
         jiterator[n - i - 1] = iterator[i];
         jiterator[i]         = iterator[n - i - 1];
      }
   }
}
