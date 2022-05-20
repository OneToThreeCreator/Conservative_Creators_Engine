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

#include "engine_common.h"

CCE_PUBLIC_OPTIONS size_t cceBinarySearch (const void *const array, const size_t arraySize, const size_t typeSize, const size_t step, const size_t value)
{
   if (!arraySize)
      return -1;
      
   const uint8_t *iterator = (uint8_t*) array;
   const uint8_t *end = ((uint8_t*) array) + arraySize * step;
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
      if (iterator + typeRemain + step >= end)
         continue;
      iterator += (typeRemain + step) * (((*((size_t*) (iterator + typeRemain))) & typeMask) < value);
   }
   while (remain > 0u); /* Checking for last valid value, because we cannot detect underflow of unsigned variable */
   return (iterator - (uint8_t*) array) / step;
}

CCE_PUBLIC_OPTIONS 
