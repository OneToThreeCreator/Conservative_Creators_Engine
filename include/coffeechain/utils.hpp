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


#ifndef UTILS_H
#define UTILS_H

#include <cstdlib>
#include <cstring>

#define CCE_UNUSED(x) (void)(x)

#define CCE_ARRAY(name, type, sizeType) \
type * name = NULL; \
sizeType name ## Quantity = 0; \
sizeType name ## QuantityAllocated = 0

#if !defined(CCE_ALLOCATION_STEP)
#define CCE_ALLOCATION_STEP 8u
#elif ((CCE_ALLOCATION_STEP) & ((CCE_ALLOCATION_STEP) - 1)) != 0
#error "CCE_ALLOCATION_STEP must be a power of 2"
#endif

#define CCE__MACRO_TO_STR(x) #x
#define CCE_MACRO_TO_STR(x) CCE__MACRO_TO_STR(x)
#define CCE_CEIL_SIZE_TO_ALLOCATION_STEP(s) (((s) & ~(CCE_ALLOCATION_STEP - 1u)) + CCE_ALLOCATION_STEP * (((s) & (CCE_ALLOCATION_STEP - 1u)) > 0))

template <typename T, typename ST>
static inline void cce__alloc_array(T **array, ST &array_size)
{
   *array = static_cast<T*>(std::malloc((array_size = CCE_ALLOCATION_STEP) * sizeof(T)));
}

#define CCE_ALLOC_ARRAY(name) cce__alloc_array(&name, name ## QuantityAllocated)

template <typename T, typename ST>
static inline void cce__alloc_array_zeroed(T **array, ST &array_size)
{
   *array = static_cast<T*>(std::calloc(array_size = CCE_ALLOCATION_STEP, sizeof(T)));
}
#define CCE_ALLOC_ARRAY_ZEROED(name) cce__alloc_array_zeroed(&name, name ## QuantityAllocated);

template <typename T, typename ST>
static inline void cce__realloc_array(T **array, ST &array_size, ST array_new_size)
{
   ST oldQuantityAllocated = array_size;
   array_size = CCE_CEIL_SIZE_TO_ALLOCATION_STEP(array_new_size);
   if (array_size == oldQuantityAllocated)
      return;
   *array = static_cast<T*>(std::realloc(array, array_size * sizeof(T)));
}
#define CCE_REALLOC_ARRAY(name, newQuantity) cce__realloc_array(&name, name ## QuantityAllocated, newQuantity)

template <typename T, typename ST>
static inline void cce__realloc_array_zeroed(T **array, ST &array_size, ST array_new_size)
{
   ST oldQuantityAllocated = array_size;
   array_size = CCE_CEIL_SIZE_TO_ALLOCATION_STEP(array_new_size);
   if (array_size == oldQuantityAllocated)
      return;
   *array = static_cast<T*>(std::realloc(array, array_size * sizeof(T)));
   std::memset((*array) + oldQuantityAllocated, 0, (array_size - oldQuantityAllocated) * sizeof(T));
}
#define CCE_REALLOC_ARRAY_ZEROED(name, newQuantity) cce__realloc_array_zeroed(&name, name ## QuantityAllocated, newQuantity)

#define CCE_FIT_ARRAY_TO_SIZE(name) CCE_REALLOC_ARRAY(name, name ## Quantity)

extern "C"
{
   #include <stdint.h>
   #include <stddef.h>
   
   #if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__TOS_WIN__) || defined(__WINDOWS__) || \
       defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN__)
   #include "cce_exports.h"
   #define CCE_PUBLIC_OPTIONS CCE_EXPORTS
   #else
   #define CCE_PUBLIC_OPTIONS
   #endif // Windows

   struct UnicodeCharWithSize
   {
      uint32_t ch;
      uint32_t size;
   };
   CCE_PUBLIC_OPTIONS size_t cceBinarySearch (const void *const array, size_t arraySize, size_t typeSize, size_t step, size_t value);
   CCE_PUBLIC_OPTIONS char*  cceReverseMemory (char *memory, size_t size);
   CCE_PUBLIC_OPTIONS uint32_t cceGetCharSizeUTF8 (const unsigned char *ch);
   CCE_PUBLIC_OPTIONS uint32_t cceGetCharUTF8 (const unsigned char *ch);
   CCE_PUBLIC_OPTIONS struct UnicodeCharWithSize cceGetCharWithSizeUTF8 (const unsigned char *ch);
   CCE_PUBLIC_OPTIONS uint32_t cceGetCharFromStringUTF8 (const char *string, size_t position);
}

#endif // UTILS_H
