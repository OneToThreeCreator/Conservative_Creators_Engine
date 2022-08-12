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


#ifndef CCE_TOOLS_H
#define CCE_TOOLS_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdint.h>
#include "engine_common.h"

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

#define CCE__REALLOC_ARRAY(name, newQuantity) \
size_t oldQuantityAllocated = (name ## QuantityAllocated); \
(name ## QuantityAllocated) = CCE_CEIL_SIZE_TO_ALLOCATION_STEP(newQuantity); \
if ((name ## QuantityAllocated) == oldQuantityAllocated) \
   break; \
name = realloc(name, (name ## QuantityAllocated) * sizeof(*name));

#define CCE_ALLOC_ARRAY(name)  (name) = malloc(((name ## QuantityAllocated) = CCE_ALLOCATION_STEP) * sizeof(*name))
#define CCE_ALLOC_ARRAY_ZEROED(name) (name) = calloc(((name ## QuantityAllocated) = CCE_ALLOCATION_STEP),  sizeof(*name))

#define CCE_REALLOC_ARRAY(name, newQuantity) \
do \
{ \
   CCE__REALLOC_ARRAY(name, newQuantity) \
} \
while(0)

#define CCE_REALLOC_ARRAY_ZEROED(name, newQuantity) \
do \
{ \
   CCE__REALLOC_ARRAY(name, newQuantity) \
   memset((name) + oldQuantityAllocated, 0, ((name ## QuantityAllocated) - oldQuantityAllocated) * sizeof(*name)); \
} \
while(0)

#define CCE_FIT_ARRAY_TO_SIZE(name) CCE_REALLOC_ARRAY(name, name ## Quantity)

struct UnicodeCharWithSize
{
   uint32_t ch;
   uint32_t size;
};

CCE_PUBLIC_OPTIONS size_t cceBinarySearch (const void *const array, const size_t arraySize, const size_t typeSize, const size_t step, const size_t value);
CCE_PUBLIC_OPTIONS char*  cceReverseMemory (char *memory, size_t size);
CCE_PUBLIC_OPTIONS uint32_t cceGetCharSizeUTF8 (const unsigned char *ch);
CCE_PUBLIC_OPTIONS uint32_t cceGetCharUTF8 (const unsigned char *ch);
CCE_PUBLIC_OPTIONS struct UnicodeCharWithSize cceGetCharWithSizeUTF8 (const unsigned char *ch);
CCE_PUBLIC_OPTIONS uint32_t cceGetCharFromStringUTF8 (const char *string, size_t position);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // CCE_TOOLS_H
