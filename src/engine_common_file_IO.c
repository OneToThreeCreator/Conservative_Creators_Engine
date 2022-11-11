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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "../include/cce/engine_common.h"
#include "../include/cce/engine_common_IO.h"
#include "../include/cce/endianess.h"
#include "../include/cce/os_interaction.h"
#include "../include/cce/utils.h"

#include "engine_common_internal.h"

struct cce_IO_function_set
{
   cce_fparsefun *fileParseFunctions;
   uint8_t fileParseFunctionsQuantity;
   uint8_t fileParseFunctionsAllocated;
   cce_onfreefun *freeingFunctions;
   cce_fstorefun *writingFunctions;
   size_t *fileParseFunctionsDataBufferSizes;
   size_t bufferSize;
};

struct cce_buffer
{
   uint8_t sectionsQuantity;
   uint16_t loadingFunctionBlockID;
#if UINT_FAST32_MAX == UINT64_MAX
   uint16_t __pad[2]; // Pad to uint64_t (if uint_fast32_t is uint32_t, it is inserted before this struct)
#endif
};

CCE_ARRAY(IOfunctionSet, static struct cce_IO_function_set, static uint16_t);

CCE_PUBLIC_OPTIONS FILE* cceMoveFileContent (FILE *file, long offset, int position, size_t size)
{
   char buffer[1024];
   long readOffset = ftell(file) + (size & 1023);
   fread(buffer, sizeof(char), size & 1023, file);
   fseek(file, offset, position);
   long writeOffset = ftell(file) + (size & 1023);
   fwrite(buffer, sizeof(char), size & 1023, file);
   for (size_t i = 0, end = size >> 10; i < end; ++i, readOffset += size, writeOffset += size)
   {
      fseek(file, readOffset, SEEK_SET);
      fread(buffer, sizeof(char), 1024, file);
      fseek(file, writeOffset, SEEK_SET);
      fwrite(buffer, sizeof(char), 1024, file);
   }
   return file;
}

CCE_PUBLIC_OPTIONS uint16_t cceGetFileIOfunctionSet (void)
{
   if (IOfunctionSetQuantity >= IOfunctionSetAllocated)
      CCE_REALLOC_ARRAY(IOfunctionSet, IOfunctionSetQuantity + 1);

   CCE_ALLOC_ARRAY(IOfunctionSet[IOfunctionSetQuantity].fileParseFunctions);
   IOfunctionSet[IOfunctionSetQuantity].fileParseFunctionsQuantity = 0;
   IOfunctionSet[IOfunctionSetQuantity].freeingFunctions = malloc(sizeof(cce_onfreefun*));
   IOfunctionSet[IOfunctionSetQuantity].writingFunctions = malloc(sizeof(cce_fstorefun*));
   IOfunctionSet[IOfunctionSetQuantity].fileParseFunctionsDataBufferSizes = malloc(sizeof(size_t*));
   IOfunctionSet[IOfunctionSetQuantity].bufferSize = 0;
   return IOfunctionSetQuantity++;
}

CCE_PUBLIC_OPTIONS ptrdiff_t cceGetFunctionBufferOffset (uint8_t functionID, uint16_t functionSetID)
{
   if (functionID >= IOfunctionSet[functionSetID].fileParseFunctionsQuantity)
      return 0;
   ptrdiff_t offset = sizeof(struct cce_buffer);
   for (size_t *iterator = IOfunctionSet[functionSetID].fileParseFunctionsDataBufferSizes, *end = iterator + functionID; iterator < end; ++iterator)
   {
      offset += *iterator;
   }
   return offset;
}

CCE_PUBLIC_OPTIONS uint8_t cceRegisterFileIOcallbacks (uint16_t functionSet, cce_fparsefun onLoad, cce_onfreefun onFree, cce_fstorefun onWrite, size_t bufferSize)
{
   struct cce_IO_function_set *currentFunctions = IOfunctionSet + functionSet;
   if (currentFunctions->fileParseFunctionsQuantity >= currentFunctions->fileParseFunctionsAllocated)
   {
      CCE_REALLOC_ARRAY(currentFunctions->fileParseFunctions, currentFunctions->fileParseFunctionsQuantity + 1);
      currentFunctions->freeingFunctions = realloc(currentFunctions->freeingFunctions, currentFunctions->fileParseFunctionsAllocated * sizeof(cce_onfreefun*));
      currentFunctions->writingFunctions = realloc(currentFunctions->writingFunctions, currentFunctions->fileParseFunctionsAllocated * sizeof(cce_fstorefun*));
      currentFunctions->fileParseFunctionsDataBufferSizes = realloc(currentFunctions->fileParseFunctionsDataBufferSizes, (currentFunctions->fileParseFunctionsAllocated + 1) * sizeof(size_t));
   }
   currentFunctions->fileParseFunctions[currentFunctions->fileParseFunctionsQuantity] = onLoad;
   currentFunctions->freeingFunctions[currentFunctions->fileParseFunctionsQuantity] = onFree;
   currentFunctions->writingFunctions[currentFunctions->fileParseFunctionsQuantity] = onWrite;
   currentFunctions->fileParseFunctionsDataBufferSizes[currentFunctions->fileParseFunctionsQuantity] = bufferSize;
   currentFunctions->bufferSize += bufferSize;
   return currentFunctions->fileParseFunctionsQuantity++;
}

CCE_PUBLIC_OPTIONS struct cce_buffer* cceSetBufferSectionQuantity (struct cce_buffer *buffer, uint8_t newSectionsQuantity)
{
   uint8_t oldSectionsUsedBitFieldSize = HEADSIZE_TO_BITFIELD_SIZE(buffer->sectionsQuantity), newSectionsUsedBitFieldSize = HEADSIZE_TO_BITFIELD_SIZE(newSectionsQuantity);
   uint_fast32_t *bits = ((uint_fast32_t*) buffer) - oldSectionsUsedBitFieldSize;
   size_t size = IOfunctionSet[buffer->loadingFunctionBlockID].bufferSize;
   for (size_t *iterator = IOfunctionSet[buffer->loadingFunctionBlockID].fileParseFunctionsDataBufferSizes + IOfunctionSet[buffer->loadingFunctionBlockID].fileParseFunctionsQuantity - 1,
        *end = iterator - newSectionsQuantity; iterator >= end; --iterator)
   {
      size -= *iterator;
   }
   if (oldSectionsUsedBitFieldSize < newSectionsUsedBitFieldSize)
   {
      bits = realloc(bits, sizeof(struct cce_buffer) + newSectionsUsedBitFieldSize * sizeof(uint_fast32_t) + size);
      buffer = (struct cce_buffer*)(bits + oldSectionsUsedBitFieldSize);
   }
   if (oldSectionsUsedBitFieldSize != newSectionsUsedBitFieldSize)
   {
      size_t oldSize = IOfunctionSet[buffer->loadingFunctionBlockID].bufferSize;
      for (size_t *iterator = IOfunctionSet[buffer->loadingFunctionBlockID].fileParseFunctionsDataBufferSizes + IOfunctionSet[buffer->loadingFunctionBlockID].fileParseFunctionsQuantity - 1,
           *end = iterator - buffer->sectionsQuantity; iterator >= end; --iterator)
      {
         oldSize -= *iterator;
      }
      memmove((uint_fast32_t*)buffer + ((ptrdiff_t)newSectionsUsedBitFieldSize - (ptrdiff_t)oldSectionsUsedBitFieldSize), buffer, oldSize);
      if (oldSectionsUsedBitFieldSize < newSectionsUsedBitFieldSize)
      {
         memset((cce_void*)(buffer + 1) + oldSize, 0, size - oldSize);
         buffer = (struct cce_buffer*)(bits + newSectionsUsedBitFieldSize);
      }
   }
   if (oldSectionsUsedBitFieldSize >= newSectionsUsedBitFieldSize)
   {
      bits = realloc(bits, sizeof(struct cce_buffer) + newSectionsUsedBitFieldSize * sizeof(uint_fast32_t) + size);
      buffer = (struct cce_buffer*)(bits + newSectionsUsedBitFieldSize);
   }
   return buffer;
}

CCE_PUBLIC_OPTIONS void cceSetBufferCallEveryFunction (struct cce_buffer *buffer)
{
   uint8_t sectionsUsedBitFieldSize = HEADSIZE_TO_BITFIELD_SIZE(buffer->sectionsQuantity);
   uint_fast32_t *bits = ((uint_fast32_t*) buffer) - sectionsUsedBitFieldSize;
   memset(bits, UINT8_MAX, sectionsUsedBitFieldSize * sizeof(uint_fast32_t));
}

CCE_PUBLIC_OPTIONS struct cce_buffer* cceCreateBuffer (uint8_t sectionsQuantity, uint16_t functionSetID)
{
   uint8_t sectionsUsedBitFieldSize = HEADSIZE_TO_BITFIELD_SIZE(sectionsQuantity);
   size_t size = IOfunctionSet[functionSetID].bufferSize;
   for (size_t *iterator = IOfunctionSet[functionSetID].fileParseFunctionsDataBufferSizes + IOfunctionSet[functionSetID].fileParseFunctionsQuantity - 1,
        *end = iterator - sectionsQuantity; iterator >= end; --iterator)
   {
      size -= *iterator;
   }
   uint_fast32_t *bits = calloc(1, sizeof(struct cce_buffer) + sectionsUsedBitFieldSize * sizeof(uint_fast32_t) + size);
   memset(bits, UINT8_MAX, sectionsUsedBitFieldSize * sizeof(uint_fast32_t));
   struct cce_buffer *buffer = (struct cce_buffer*)(bits + sectionsUsedBitFieldSize);
   buffer->sectionsQuantity = sectionsQuantity;
   buffer->loadingFunctionBlockID = functionSetID;
   return buffer;
}

CCE_PUBLIC_OPTIONS void cceFreeBuffer (struct cce_buffer *buffer)
{
   if (!buffer)
      return;
   struct cce_IO_function_set *currentFunctions = IOfunctionSet + buffer->loadingFunctionBlockID;
   size_t *sizes = currentFunctions->fileParseFunctionsDataBufferSizes;
   size_t bitCounter = 0;
   uint_fast32_t *bits = ((uint_fast32_t*) buffer) - HEADSIZE_TO_BITFIELD_SIZE(buffer->sectionsQuantity);
   cce_void *data = (cce_void*) (buffer + 1);
   for (cce_onfreefun *fun = currentFunctions->freeingFunctions, *end = currentFunctions->freeingFunctions + buffer->sectionsQuantity;
        fun < end; ++fun, ++bitCounter, data += *sizes, ++sizes)
   {
      if ((bits[bitCounter >> ((sizeof(uint_fast32_t) >> 2) + 4)] & (1 << (bitCounter & (sizeof(uint_fast32_t) * 8 - 1)))) == 0)
         continue;
      
      (*fun)(data, buffer);
   }
   free(bits);
}

CCE_PUBLIC_OPTIONS struct cce_buffer* cceLoadBinaryCCF (char *path, uint16_t functionSetID)
{
   FILE *file = fopen(path, "rb");
   if (file == NULL)
   {
      return NULL;
   }
   struct cce_IO_function_set *currentFunctions = IOfunctionSet + functionSetID;
   uint8_t headSize;
   uint8_t errorLoader;
   fread(&headSize, sizeof(uint8_t), 1, file);
   uint8_t sectionSizes[256];
   fread(sectionSizes + 1, sizeof(uint8_t), headSize, file);
   size_t size;
   {
      uint8_t *iterator = sectionSizes + headSize, *end = sectionSizes;
      size_t *bufferSizes = currentFunctions->fileParseFunctionsDataBufferSizes + headSize - 1;
      sectionSizes[0] = 1; // Workaround to avoid out-of-bounds check
      size = currentFunctions->bufferSize;
      while (*iterator != 0)
      {
         size -= *bufferSizes;
         --iterator;
         --bufferSizes;
      }
      headSize = (iterator - sectionSizes);
   }
   if (headSize == 0)
   {
      fclose(file);
      struct cce_buffer *buffer = calloc(1, sizeof(struct cce_buffer));
      buffer->sectionsQuantity = 0;
      return buffer;
   }
   size_t sectionsUsedBitFieldSize = HEADSIZE_TO_BITFIELD_SIZE(headSize);
   uint_fast32_t *bits = calloc(1, sizeof(struct cce_buffer) + sectionsUsedBitFieldSize * sizeof(uint_fast32_t) + size);
   struct cce_buffer *buffer = (struct cce_buffer*)(bits + sectionsUsedBitFieldSize);
   buffer->sectionsQuantity = headSize;
   buffer->loadingFunctionBlockID = functionSetID;
   size_t *sizes = currentFunctions->fileParseFunctionsDataBufferSizes;
   uint8_t *sectionSizesIt = sectionSizes + 1;
   size_t bitCounter = 0;
   cce_void *data = (cce_void*) (buffer + 1);
   for (cce_fparsefun *fun = currentFunctions->fileParseFunctions, *end = currentFunctions->fileParseFunctions + headSize; fun < end; ++fun, data += *sizes++, ++sectionSizesIt, ++bitCounter)
   {
      if (*sectionSizesIt == 0)
         continue;
      
      bits[bitCounter >> ((sizeof(uint_fast32_t) >> 2) + 4)] |= 1 << (bitCounter & (sizeof(uint_fast32_t) * 8 - 1));
      if ((*fun)(data, *sectionSizesIt, buffer, file) == 0)
         continue;
      
      errorLoader = fun - currentFunctions->fileParseFunctions;
      goto ERROR;
   }
   fclose(file);
   return buffer;
ERROR:
   fclose(file);
   bitCounter = 0;
   data = (cce_void*) (buffer + 1);
   for (cce_onfreefun *fun = currentFunctions->freeingFunctions, *end = currentFunctions->freeingFunctions + errorLoader; // Don't free what wasn't loaded
        fun < end; ++fun, ++bitCounter, data += *sizes++)
   {
      if ((bits[bitCounter >> ((sizeof(uint_fast32_t) >> 2) + 4)] & (1 << (bitCounter & (sizeof(uint_fast32_t) * 8 - 1)))) == 0)
         continue;
      
      (*fun)(data, buffer);
   }
   free(bits);
   return NULL;
}

CCE_PUBLIC_OPTIONS int cceWriteBinaryCCF (struct cce_buffer *buffer, char *path)
{
   FILE *file = fopen(path, "wb+");
   if (file == NULL)
   {
      return -1;
   }
   struct cce_IO_function_set *currentFunctions = IOfunctionSet + buffer->loadingFunctionBlockID;
   uint8_t sectionSizes[256] = {0};
   uint8_t headSize = buffer->sectionsQuantity;
   fseek(file, (headSize + 1) * sizeof(uint8_t), SEEK_SET);
   size_t *sizes = currentFunctions->fileParseFunctionsDataBufferSizes;
   size_t bytesWritten = 0;
   uint8_t *sectionSizesIt = sectionSizes + 1;
   size_t bitCounter = 0;
   uint_fast32_t *bits = ((uint_fast32_t*) buffer) - HEADSIZE_TO_BITFIELD_SIZE(buffer->sectionsQuantity);
   cce_void *data = (cce_void*) (buffer + 1);
   for (cce_fstorefun *fun = currentFunctions->writingFunctions, *end = currentFunctions->writingFunctions + buffer->sectionsQuantity;
        fun < end; ++fun, ++sectionSizesIt, ++bitCounter, data += *sizes, ++sizes)
   {
      if ((bits[bitCounter >> ((sizeof(uint_fast32_t) >> 2) + 4)] & (1 << (bitCounter & (sizeof(uint_fast32_t) * 8 - 1)))) == 0)
         continue;
      
      *sectionSizesIt = (*fun)(data, buffer, file);
   }
   bytesWritten = ftell(file) - (1 + headSize) * sizeof(uint8_t);
   fseek(file, (headSize + 1) * sizeof(uint8_t), SEEK_SET);
   {
      uint8_t *iterator = sectionSizes + headSize, *end = sectionSizes;
      sectionSizes[0] = 1; // Workaround to avoid out-of-bounds check
      while (*iterator != 0)
      {
         --iterator;
      }
      if (iterator - sectionSizes != headSize)
      {
         cceMoveFileContent(file, (iterator - sectionSizes) - headSize, SEEK_CUR, bytesWritten);
         headSize = (iterator - sectionSizes);
         cceTruncateFile(file, bytesWritten + (1 + headSize) * sizeof(uint8_t));
      }
   }
   fseek(file, 0, SEEK_SET);
   fwrite(&headSize, sizeof(uint8_t), 1, file);
   fwrite(sectionSizes, sizeof(uint8_t), buffer->sectionsQuantity, file);
   fclose(file);
   return 0;
}
