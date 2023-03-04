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

#include <assert.h>
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
   cce_freadfun     *readingFunctions;
   cce_dataparsefun *freeingFunctions;
   cce_dataparsefun *creatingFunctions;
   cce_fwritefun    *writingFunctions;
   size_t           *readingFunctionsDataBufferOffsets;
   char            (*sectionNames)[8];
   char            **sectionNamesSorted;
   uint16_t          readingFunctionsQuantity;
   uint16_t          readingFunctionsAllocated;
};

CCE_ARRAY(IOfunctionSet, static struct cce_IO_function_set, static uint16_t);

CCE_API FILE* cceMoveFileContent (FILE *file, long offset, int position, size_t size)
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

CCE_API uint16_t cceGetFileIOfunctionSet (void)
{
   if (IOfunctionSetQuantity >= IOfunctionSetAllocated)
      CCE_REALLOC_ARRAY(IOfunctionSet, IOfunctionSetQuantity + 1);

   CCE_ALLOC_ARRAY(IOfunctionSet[IOfunctionSetQuantity].readingFunctions, 1);
   IOfunctionSet[IOfunctionSetQuantity].readingFunctionsQuantity = 0;
   IOfunctionSet[IOfunctionSetQuantity].freeingFunctions  =                 malloc( IOfunctionSet[IOfunctionSetQuantity].readingFunctionsAllocated * sizeof(cce_dataparsefun*));
   IOfunctionSet[IOfunctionSetQuantity].creatingFunctions =                 malloc( IOfunctionSet[IOfunctionSetQuantity].readingFunctionsAllocated * sizeof(cce_dataparsefun*));
   IOfunctionSet[IOfunctionSetQuantity].writingFunctions  =                 malloc( IOfunctionSet[IOfunctionSetQuantity].readingFunctionsAllocated * sizeof(cce_fwritefun*));
   IOfunctionSet[IOfunctionSetQuantity].readingFunctionsDataBufferOffsets = malloc((IOfunctionSet[IOfunctionSetQuantity].readingFunctionsAllocated + 1) * sizeof(size_t*));
   IOfunctionSet[IOfunctionSetQuantity].sectionNames =                      malloc( IOfunctionSet[IOfunctionSetQuantity].readingFunctionsAllocated * 8 * sizeof(char));
   IOfunctionSet[IOfunctionSetQuantity].sectionNamesSorted =                malloc( IOfunctionSet[IOfunctionSetQuantity].readingFunctionsAllocated * sizeof(char*));
   IOfunctionSet[IOfunctionSetQuantity].readingFunctionsDataBufferOffsets[0] = 0;
   return IOfunctionSetQuantity++;
}

CCE_API ptrdiff_t cceGetFunctionBufferOffset (uint16_t functionID, uint16_t functionSetID)
{
   if (functionID >= IOfunctionSet[functionSetID].readingFunctionsQuantity)
      return 0;
   return sizeof(struct cce_buffer) + IOfunctionSet[functionSetID].readingFunctionsDataBufferOffsets[functionID];
}

static int str_comp (const void *_a, const void *_b)
{
   return memcmp(*(char**)_a, *(char**)_b, 8);
}

CCE_API uint8_t cceRegisterFileIOcallbacks (uint16_t functionSet, char sectionName[8], cce_freadfun onLoad, cce_dataparsefun onFree, cce_dataparsefun onCreate, cce_fwritefun onWrite, size_t bufferSize)
{
   assert(functionSet < IOfunctionSetQuantity);
   struct cce_IO_function_set *currentFunctions = IOfunctionSet + functionSet;
   if (currentFunctions->readingFunctionsQuantity >= currentFunctions->readingFunctionsAllocated)
   {
      CCE_REALLOC_ARRAY(currentFunctions->readingFunctions, currentFunctions->readingFunctionsQuantity + 1);
      currentFunctions->freeingFunctions  = realloc(currentFunctions->freeingFunctions,  currentFunctions->readingFunctionsAllocated * sizeof(cce_dataparsefun*));
      currentFunctions->creatingFunctions = realloc(currentFunctions->creatingFunctions, currentFunctions->readingFunctionsAllocated * sizeof(cce_dataparsefun*));
      currentFunctions->writingFunctions  = realloc(currentFunctions->writingFunctions,  currentFunctions->readingFunctionsAllocated * sizeof(cce_fwritefun*));
      currentFunctions->readingFunctionsDataBufferOffsets = realloc(currentFunctions->readingFunctionsDataBufferOffsets, (currentFunctions->readingFunctionsAllocated + 1) * sizeof(size_t));
      currentFunctions->sectionNames = realloc(currentFunctions->sectionNames, currentFunctions->readingFunctionsAllocated * 8 * sizeof(char));
      currentFunctions->sectionNamesSorted = realloc(currentFunctions->sectionNamesSorted, currentFunctions->readingFunctionsAllocated * sizeof(char*));
   }
   currentFunctions->readingFunctions[currentFunctions->readingFunctionsQuantity]  = onLoad;
   currentFunctions->freeingFunctions[currentFunctions->readingFunctionsQuantity]  = onFree;
   currentFunctions->creatingFunctions[currentFunctions->readingFunctionsQuantity] = onCreate;
   currentFunctions->writingFunctions[currentFunctions->readingFunctionsQuantity]  = onWrite;
   currentFunctions->readingFunctionsDataBufferOffsets[currentFunctions->readingFunctionsQuantity + 1] = bufferSize + currentFunctions->readingFunctionsDataBufferOffsets[currentFunctions->readingFunctionsQuantity];
   strncpy(currentFunctions->sectionNames[currentFunctions->readingFunctionsQuantity], sectionName, 8);
   char *ptr = currentFunctions->sectionNames[currentFunctions->readingFunctionsQuantity];
   char **namePosition = cceBinarySearchFirst(&ptr, currentFunctions->sectionNamesSorted, currentFunctions->readingFunctionsQuantity, sizeof(char*), str_comp);
   memmove(namePosition + 1, namePosition, (currentFunctions->readingFunctionsQuantity - (namePosition - currentFunctions->sectionNamesSorted)) * sizeof(char*));
   *namePosition = currentFunctions->sectionNames[currentFunctions->readingFunctionsQuantity];
   
   return currentFunctions->readingFunctionsQuantity++;
}

CCE_API struct cce_buffer* cceSetBufferSectionQuantity (struct cce_buffer *buffer, uint8_t newSectionsQuantity)
{
   uint8_t oldSectionsQuantity = buffer->sectionsQuantity;
   size_t size = IOfunctionSet[buffer->loadingFunctionBlockID].readingFunctionsDataBufferOffsets[newSectionsQuantity + 1];
   buffer = realloc(buffer, sizeof(struct cce_buffer) + size);
   if (newSectionsQuantity > oldSectionsQuantity)
   {
      size_t *offsets = IOfunctionSet[buffer->loadingFunctionBlockID].readingFunctionsDataBufferOffsets + oldSectionsQuantity;
      for (cce_dataparsefun *iterator = IOfunctionSet[buffer->loadingFunctionBlockID].creatingFunctions + oldSectionsQuantity,
           *end = IOfunctionSet[buffer->loadingFunctionBlockID].creatingFunctions + newSectionsQuantity; iterator < end; ++iterator, ++offsets)
      {
         if (*iterator != NULL)
            (*iterator)((cce_void*)(buffer + 1) + *offsets, buffer);
      }
   }
   return buffer;
}

CCE_API struct cce_buffer* cceCreateBuffer (uint8_t sectionsQuantity, uint16_t functionSetID)
{
   assert(functionSetID < IOfunctionSetQuantity);
   sectionsQuantity = CCE_MIN(sectionsQuantity, IOfunctionSet[functionSetID].readingFunctionsQuantity);
   size_t size = IOfunctionSet[functionSetID].readingFunctionsDataBufferOffsets[sectionsQuantity + 1];
   struct cce_buffer *buffer = malloc(sizeof(struct cce_buffer) + size);
   buffer->sectionsQuantity = sectionsQuantity;
   buffer->loadingFunctionBlockID = functionSetID;
   size_t *offsets = IOfunctionSet[functionSetID].readingFunctionsDataBufferOffsets;
   for (cce_dataparsefun *iterator = IOfunctionSet[functionSetID].creatingFunctions, *end = iterator + sectionsQuantity; iterator < end;
        ++iterator, ++offsets)
   {
      if (*iterator != NULL)
         (*iterator)((cce_void*)(buffer + 1) + *offsets, buffer);
   }
   return buffer;
}

CCE_API void cceFreeBuffer (struct cce_buffer *buffer)
{
   if (!buffer)
      return;
   struct cce_IO_function_set *currentFunctions = IOfunctionSet + buffer->loadingFunctionBlockID;
   size_t *offsets = currentFunctions->readingFunctionsDataBufferOffsets;
   for (cce_dataparsefun *fun = currentFunctions->freeingFunctions, *end = currentFunctions->freeingFunctions + buffer->sectionsQuantity;
        fun < end; ++fun, ++offsets)
   {
      
      (*fun)((cce_void*)(buffer + 1) + *offsets, buffer);
   }
   free(buffer);
}

CCE_API struct cce_buffer* cceLoadBinaryCCF (char *path, uint16_t functionSetID)
{
   assert(functionSetID < IOfunctionSetQuantity);
   FILE *file = fopen(path, "rb");
   if (file == NULL)
   {
      return NULL;
   }
   struct cce_IO_function_set *currentFunctions = IOfunctionSet + functionSetID;
   uint8_t headSize, loaders;
   uint8_t errorLoader;
   fread(&loaders, sizeof(uint8_t), 1, file);
   char names[255][8] = {0};
   uint16_t sectionSizes[255];
   uint8_t  sectionsInitialized[255] = {0};
   headSize = 0;
   for (unsigned i = 0; i < loaders; ++i)
   {
      fread(names[i], sizeof(char), 8, file);
      fread(sectionSizes + i, sizeof(uint16_t), 1, file);
      char *key = names[i];
      char **position = bsearch(&key, currentFunctions->sectionNamesSorted, currentFunctions->readingFunctionsQuantity, sizeof(char*), str_comp);
      if (position == NULL)
         goto ERROR;
      headSize = CCE_MAX(headSize, (*position - currentFunctions->sectionNames[0]) / 8 + 1);
   }
   size_t size = currentFunctions->readingFunctionsDataBufferOffsets[headSize + 1];
   if (headSize == 0)
   {
      fclose(file);
      struct cce_buffer *buffer = calloc(1, sizeof(struct cce_buffer));
      buffer->sectionsQuantity = 0;
      return buffer;
   }
   struct cce_buffer *buffer = calloc(1, sizeof(struct cce_buffer) + size);
   buffer->sectionsQuantity = headSize;
   buffer->loadingFunctionBlockID = functionSetID;
   size_t *offsets = currentFunctions->readingFunctionsDataBufferOffsets;
   cce_void *data = (cce_void*)(buffer + 1);
   
   for (uint8_t i = 0, j; i < loaders; ++i)
   {
      char *key = names[i];
      char **position = bsearch(&key, currentFunctions->sectionNamesSorted, currentFunctions->readingFunctionsQuantity, sizeof(char*), str_comp);
      j = (*position - currentFunctions->sectionNames[0]) / 8;
      sectionsInitialized[j] = 1;
      if ((currentFunctions->readingFunctions[j])(data + offsets[j], sectionSizes[i], buffer, file) == 0)
         continue;
      
      errorLoader = j;
      goto ERROR;
   }
   fclose(file);
   for (uint8_t i = 0; i < headSize; ++i)
   {
      if (sectionsInitialized[i] != 0 || currentFunctions->creatingFunctions[i] == NULL)
         continue;
      currentFunctions->creatingFunctions[i](data + offsets[i], buffer);
   }
   return buffer;
ERROR:
   fclose(file);
   data = (cce_void*)(buffer + 1);
   offsets = currentFunctions->readingFunctionsDataBufferOffsets;
   for (cce_dataparsefun *fun = currentFunctions->freeingFunctions, *end = currentFunctions->freeingFunctions + errorLoader;
        fun < end; ++fun, ++offsets)
   {
      (*fun)(data + *offsets, buffer);
   }
   free(buffer);
   return NULL;
}

CCE_API int cceWriteBinaryCCF (struct cce_buffer *buffer, char *path)
{
   FILE *file = fopen(path, "wb+");
   if (file == NULL)
   {
      return -1;
   }
   struct cce_IO_function_set *currentFunctions = IOfunctionSet + buffer->loadingFunctionBlockID;
   uint16_t sectionSizes[255] = {0};
   uint8_t headSize = buffer->sectionsQuantity;
   fseek(file, (headSize) * (sizeof(uint16_t) + 8 * sizeof(char)) + sizeof(uint8_t), SEEK_SET);
   size_t *offsets = currentFunctions->readingFunctionsDataBufferOffsets;
   size_t bytesWritten = 0;
   uint16_t *sectionSizesIt = sectionSizes;
   
   for (cce_fwritefun *fun = currentFunctions->writingFunctions, *end = currentFunctions->writingFunctions + buffer->sectionsQuantity;
        fun < end; ++fun, ++sectionSizesIt, ++offsets)
   {
      *sectionSizesIt = (*fun)((cce_void*)(buffer + 1) + *offsets, buffer, file);
   }
   bytesWritten = ftell(file) - headSize * (sizeof(uint16_t) + 8 * sizeof(char)) - sizeof(uint8_t);
   uint8_t newHeadSize = headSize;
   fseek(file, (headSize + 1) * sizeof(uint8_t), SEEK_SET);
   {
      uint16_t *iterator = sectionSizes + headSize;
      do
      {
         --iterator;
         newHeadSize -= (*iterator == 0);
      }
      while (iterator > sectionSizes);
      if (newHeadSize != headSize)
      {
         cceMoveFileContent(file, (newHeadSize - headSize) * (sizeof(uint16_t) + 8 * sizeof(char)), SEEK_CUR, bytesWritten);
         cceTruncateFile(file, bytesWritten + sizeof(uint8_t) + newHeadSize * (sizeof(uint16_t) + 8 * sizeof(char)));
      }
   }
   fseek(file, 0, SEEK_SET);
   fwrite(&newHeadSize, sizeof(uint8_t), 1, file);
   for (unsigned i = 0; i < headSize; ++i)
   {
      if (sectionSizes[i] != 0)
      {
         fwrite(currentFunctions->sectionNames[i], sizeof(char), 8, file);
         fwrite(sectionSizes + i, sizeof(uint16_t), 1, file);
      }
   }
   fclose(file);
   return 0;
}
