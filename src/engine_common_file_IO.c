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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../include/coffeechain/engine_common.h"
#include "../include/coffeechain/endianess.h"

#include "engine_common_internal.h"


struct ElementGroup* cce__loadGroups (uint16_t groupsQuantity, FILE *map_f)
{
   if (!groupsQuantity)
   {
      return NULL;
   }
   struct ElementGroup *groups = (struct ElementGroup*) malloc(groupsQuantity * sizeof(struct ElementGroup));
   for (struct ElementGroup *iterator = groups, *end = (groups + groupsQuantity); iterator < end; ++iterator)
   {
      fread(&(iterator->elementsQuantity), 2u/*uint16_t*/, 1u, map_f);
      iterator->elementsQuantity = cceLittleEndianToHostEndianInt16(iterator->elementsQuantity);
      if (iterator->elementsQuantity)
      {
         iterator->elements = (uint32_t*) malloc(iterator->elementsQuantity * sizeof(uint32_t));
         fread(iterator->elements, 4u/*uint32_t*/, iterator->elementsQuantity, map_f);
         cceLittleEndianToHostEndianArrayInt32(iterator->elements, iterator->elementsQuantity);
      }
      else
      {
         iterator->elements = NULL;
      }
   }
   return groups;
}

void cce__writeGroups (uint16_t groupsQuantity, struct ElementGroup *groups, FILE *map_f)
{
   for (struct ElementGroup *iterator = groups, *end = (groups + groupsQuantity); iterator < end; ++iterator)
   {
      uint16_t elementsQuantity = cceHostEndianToLittleEndianInt16(iterator->elementsQuantity);
      fwrite(&elementsQuantity, 2u/*uint16_t*/, 1u, map_f);
      if (!iterator->elementsQuantity)
         continue;

      if (*g_endianess == CCE_BIG_ENDIAN)
      {
         uint32_t elementID;
         for (uint32_t *jiterator = iterator->elements, *jend = iterator->elements + iterator->elementsQuantity; jiterator < jend; ++jiterator)
         {
            elementID = cceBigEndianToLittleEndianInt32(*jiterator);
            fwrite(&elementID, 4u/*uint32_t*/, 1, map_f);
         }
      }
      else
      {
         fwrite(iterator->elements, 4u/*uint32_t*/, iterator->elementsQuantity, map_f);
      }
   }
}

struct ElementLogic* cce__loadLogic (uint32_t logicQuantity, FILE *map_f, void (**endianConvertAction)(void*))
{
   if (!logicQuantity)
   {
      return NULL;
   }
   struct ElementLogic *logic = (struct ElementLogic*) malloc(logicQuantity * sizeof(struct ElementLogic));
   struct ElementLogic *end = (logic + logicQuantity);

   uint_fast32_t operationsQuantityInBytes;
   uint8_t isLogicQuantityHigherThanThree;
   for (struct ElementLogic *iterator = logic; iterator < end; ++iterator)
   {
      fread(&(iterator->logicElementsQuantity), 1u/*uint8_t*/,   1u,                                                        map_f);
      (iterator->logicElements) = (uint16_t *) malloc((iterator->logicElementsQuantity) * sizeof(uint16_t));
      fread( (iterator->logicElements),         2u/*uint16_t*/,  (iterator->logicElementsQuantity),                         map_f);
      cceLittleEndianToHostEndianArrayInt16(iterator->logicElements, iterator->logicElementsQuantity);

      isLogicQuantityHigherThanThree = iterator->logicElementsQuantity > 3u;
      operationsQuantityInBytes = ((0x01 << ((iterator->logicElementsQuantity) - 3u)) * isLogicQuantityHigherThanThree) + (!isLogicQuantityHigherThanThree);
      (iterator->operations) = (uint_fast16_t*) calloc((operationsQuantityInBytes > sizeof(uint_fast16_t)) ? operationsQuantityInBytes : sizeof(uint_fast16_t), 1u);
      if (operationsQuantityInBytes > sizeof(uint_fast16_t))
      {
         fread( (iterator->operations),         sizeof(uint_fast16_t),     operationsQuantityInBytes >> SHIFT_OF_FAST_SIZE, map_f);
         cceLittleEndianToHostEndianArrayIntN(iterator->operations, operationsQuantityInBytes >> SHIFT_OF_FAST_SIZE, sizeof(uint_fast16_t));
      }
      else
      {
         fread(iterator->operations,            operationsQuantityInBytes, 1u,                                              map_f);
         cceLittleEndianToHostEndianArrayIntN(iterator->operations, 1, sizeof(uint_fast16_t));
      }
      fread(&(iterator->elementType),           8u/*uint64_t*/,  1u,                                                        map_f);
      iterator->elementType = cceLittleEndianToHostEndianInt64(iterator->elementType);
      fread(&(iterator->actionsQuantity),       1u/*uint8_t*/,   1u,                                                        map_f);
      (iterator->actionIDs) = (uint32_t *) malloc((iterator->actionsQuantity) * sizeof(uint32_t));
      fread( (iterator->actionIDs),             4u/*uint32_t*/,  (iterator->actionsQuantity),                               map_f);
      cceLittleEndianToBigEndianArrayInt32(iterator->actionIDs, iterator->actionsQuantity);
      (iterator->actionsArgOffsets) = (uint32_t *) malloc((iterator->actionsQuantity + 1u) * sizeof(uint32_t));
      *(iterator->actionsArgOffsets) = 0u;
      fread( (iterator->actionsArgOffsets + 1), 4u/*uint32_t*/,  (iterator->actionsQuantity),                               map_f);
      cceLittleEndianToBigEndianArrayInt32((iterator->actionsArgOffsets + 1), iterator->actionsQuantity);
      (iterator->actionsArg) = (cce_void *) malloc(*(iterator->actionsArgOffsets + iterator->actionsQuantity)/* sizeof(cce_void) */);
      fread( (iterator->actionsArg),            1u/*cce_void*/, *(iterator->actionsArgOffsets + iterator->actionsQuantity), map_f);
      if (*g_endianess == CCE_BIG_ENDIAN)
      {
         cce__callActions(endianConvertAction, iterator->actionsQuantity, iterator->actionIDs, iterator->actionsArgOffsets, iterator->actionsArg);
      }
   }
   return logic;
}

void cce__writeLogic (uint32_t logicQuantity, struct ElementLogic *logic, FILE *map_f, void (**endianConvertAction)(void*))
{
   struct ElementLogic *end = (logic + logicQuantity - 1u);
   uint_fast16_t *bufferfast16array = NULL;
   uint64_t buffer64;
   uint_fast32_t operationsQuantityInBytes;
   uint_fast16_t bufferfast16;
   uint32_t buffer32;
   uint16_t buffer16;
   uint8_t isLogicQuantityHigherThanThree;
   for (struct ElementLogic *iterator = logic; iterator <= end; ++iterator)
   {
      fwrite(&(iterator->logicElementsQuantity), 1u/*uint8_t*/,   1u,                                                        map_f);
      if (*g_endianess == CCE_BIG_ENDIAN)
      {
         for (uint16_t *jiterator = iterator->logicElements, *jend = iterator->logicElements + iterator->logicElementsQuantity; jiterator < jend; ++jiterator)
         {
            buffer16 = cceBigEndianToLittleEndianInt16(*jiterator);
            fwrite(&buffer16,                    2u/*uint16_t*/,  1u,                                                        map_f);
         }
      }
      else
      {
         fwrite( (iterator->logicElements),      2u/*uint16_t*/,  (iterator->logicElementsQuantity),                         map_f);
      }

      isLogicQuantityHigherThanThree = iterator->logicElementsQuantity > 3u;
      operationsQuantityInBytes = ((0x01 << ((iterator->logicElementsQuantity) - 3u)) * isLogicQuantityHigherThanThree) + (!isLogicQuantityHigherThanThree);
      if (operationsQuantityInBytes > sizeof(uint_fast16_t))
      {

         if (*g_endianess == CCE_BIG_ENDIAN)
         {
            bufferfast16array = realloc(bufferfast16array, operationsQuantityInBytes);
            cceBigEndianToLittleEndianNewArrayIntN(bufferfast16array, iterator->operations, operationsQuantityInBytes >> SHIFT_OF_FAST_SIZE, sizeof(uint_fast16_t));
            fwrite(bufferfast16array,            sizeof(uint_fast16_t), operationsQuantityInBytes >> SHIFT_OF_FAST_SIZE,     map_f);
         }
         else
         {
            fwrite( (iterator->operations),      sizeof(uint_fast16_t), operationsQuantityInBytes >> SHIFT_OF_FAST_SIZE,     map_f);
         }
      }
      else
      {
         bufferfast16 = *(iterator->operations);
         cceHostEndianToLittleEndianArrayIntN(&bufferfast16, 1, sizeof(uint_fast16_t));
         fwrite( (iterator->operations),         operationsQuantityInBytes, 1u,                                              map_f);
      }
      buffer64 = cceHostEndianToLittleEndianInt64(iterator->elementType);
      fwrite(&buffer64,                          8u/*uint64_t*/,  1u,                                                        map_f);
      fwrite(&(iterator->actionsQuantity),       1u/*uint8_t*/,   1u,                                                        map_f);
      if (*g_endianess == CCE_BIG_ENDIAN)
      {
         for (uint32_t *jiterator = iterator->actionIDs, *jend = iterator->actionIDs + iterator->actionsQuantity; jiterator < jend; ++jiterator)
         {
            buffer32 = cceBigEndianToLittleEndianInt32(*jiterator);
            fwrite(&buffer32,                    4u/*uint32_t*/,  1u,                                                        map_f);
         }
         for (uint32_t *jiterator = iterator->actionsArgOffsets + 1, *jend = iterator->actionsArgOffsets + 1 + iterator->actionsQuantity;
              jiterator < jend; ++jiterator)
         {
            buffer32 = cceBigEndianToLittleEndianInt32(*jiterator);
            fwrite(&buffer32,                    4u/*uint32_t*/,  1u,                                                        map_f);
         }
         for (uint32_t *jiterator = iterator->actionsArgOffsets, *action = iterator->actionIDs, *jend = iterator->actionsArgOffsets + iterator->actionsQuantity;
              jiterator < jend; ++jiterator, ++action)
         {
            uint32_t size = *(jiterator + 1) - *jiterator;
            uint32_t *buffer;
            if (size > operationsQuantityInBytes)
            {
                buffer = malloc(size);
            }
            else
            {
                buffer = (uint32_t*) bufferfast16array;
            }
            memcpy(buffer, iterator->actionsArg + *jiterator, size);
            (*(endianConvertAction + *action))(buffer);
            fwrite(buffer, size, 1, map_f);
            if (size > operationsQuantityInBytes)
            {
                free(buffer);
            }
         }
      }
      else
      {
         fwrite( (iterator->actionIDs),          4u/*uint32_t*/,  (iterator->actionsQuantity),                               map_f);
         fwrite(iterator->actionsArgOffsets + 1, 4u/*uint32_t*/,  (iterator->actionsQuantity),                               map_f);
         fwrite( (iterator->actionsArg),         1u/*cce_void*/, *(iterator->actionsArgOffsets + iterator->actionsQuantity), map_f);
      }
      if (*g_endianess == CCE_BIG_ENDIAN)
      {
         cce__callActions(endianConvertAction, iterator->actionsQuantity, iterator->actionIDs, iterator->actionsArgOffsets, iterator->actionsArg);
      }
   }
   if (*g_endianess == CCE_BIG_ENDIAN)
   {
      free(bufferfast16array);
   }
}
