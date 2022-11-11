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

#ifndef UTILS_HPP
#define UTILS_HPP
#ifndef UTILS_H
#include "utils.h"
#endif // UTILS_H

#include <algorithm>
#include <cstdlib>
#include <cstring>

#define CCE_MIN(x,y) std::min(x,y)
#define CCE_MAX(x,y) std::max(x,y)
#define CCE_ABS(x)   std::abs(x)

#define CCE_ARRAY(name, type, sizeType) \
type * name = NULL; \
sizeType name ## Quantity = 0; \
sizeType name ## Allocated = 0

#define CCE_ARRAY_STRUCT(name, type, sizeType) \
struct name \
{ \
   type * data; \
   sizeType dataQuantity; \
   sizeType dataAllocated; \
}

template <typename T, typename ST>
static inline void cce__alloc_array(T **array, ST &array_size)
{
   *array = static_cast<T*>(std::malloc((array_size = 1) * sizeof(T)));
}

#define CCE_ALLOC_ARRAY(name) cce__alloc_array(&name, name ## Allocated)

template <typename T, typename ST>
static inline void cce__alloc_array_zeroed(T **array, ST &array_size)
{
   *array = static_cast<T*>(std::calloc(array_size = 1, sizeof(T)));
}
#define CCE_ALLOC_ARRAY_ZEROED(name) cce__alloc_array_zeroed(&name, name ## Allocated);

template <typename T, typename ST>
static inline void cce__realloc_array(T **array, ST &array_size, ST array_new_size)
{
   ST oldAllocated = array_size;
   CCE_CEIL_TO_POWER_OF_TWO(array_new_size, array_size);
   if (array_size == oldAllocated)
      return;
   *array = static_cast<T*>(std::realloc(array, array_size * sizeof(T)));
}
#define CCE_REALLOC_ARRAY(name, newQuantity) cce__realloc_array(&name, name ## Allocated, newQuantity)

template <typename T, typename ST>
static inline void cce__realloc_array_zeroed(T **array, ST &array_size, ST array_new_size)
{
   ST oldAllocated = array_size;
   CCE_CEIL_TO_POWER_OF_TWO(array_new_size, array_size);
   if (array_size == oldAllocated)
      return;
   *array = static_cast<T*>(std::realloc(array, array_size * sizeof(T)));
   std::memset((*array) + oldAllocated, 0, (array_size - oldAllocated) * sizeof(T));
}
#define CCE_REALLOC_ARRAY_ZEROED(name, newQuantity) cce__realloc_array_zeroed(&name, name ## Allocated, newQuantity)

#endif // UTILS_HPP
