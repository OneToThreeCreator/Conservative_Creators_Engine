/*
   Conservative Creator's Engine - open source engine for making games.
   Copyright © 2020-2023 Andrey Gaivoronskiy

   This file is part of Conservative Creator's Engine.

   Conservative Creator's Engine is free software: you can redistribute it and/or modify it under 
   the terms of the GNU Lesser General Public License as published by the Free Software Foundation,
   either version 2 of the License, or (at your option) any later version.

   Conservative Creator's Engine is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
   PURPOSE. See the GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License along
   with Conservative Creator's Engine. If not, see <https://www.gnu.org/licenses/>.
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

#include "../include/cce/engine_common_internal.h"

struct cce_IO_function_set
{
   cce_freadfun     *readingFunctions;
   cce_dataparsefun *freeingFunctions;
   cce_dataparsefun *creatingFunctions;
   cce_fwritefun    *writingFunctions;
   size_t           *readingFunctionsDataBufferOffsets;
   uint32_t        (*sectionUIDs);
   uint32_t        **sectionUIDsSorted;
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
   IOfunctionSet[IOfunctionSetQuantity].readingFunctionsDataBufferOffsets = malloc((IOfunctionSet[IOfunctionSetQuantity].readingFunctionsAllocated + 1) * sizeof(size_t));
   IOfunctionSet[IOfunctionSetQuantity].sectionUIDs =                       malloc( IOfunctionSet[IOfunctionSetQuantity].readingFunctionsAllocated * sizeof(uint64_t));
   IOfunctionSet[IOfunctionSetQuantity].sectionUIDsSorted =                 malloc( IOfunctionSet[IOfunctionSetQuantity].readingFunctionsAllocated * sizeof(uint64_t*));
   IOfunctionSet[IOfunctionSetQuantity].readingFunctionsDataBufferOffsets[0] = 0;
   return IOfunctionSetQuantity++;
}

CCE_API ptrdiff_t cceGetFunctionBufferOffset (uint32_t functionUID, uint16_t functionSetID)
{
   struct cce_IO_function_set *currentFunctions = IOfunctionSet + functionSetID;
   ptrdiff_t result;
   #ifdef NDEBUG
   #define ERROR_HANDLER return -1;
   #else
   #define ERROR_HANDLER fprintf(stderr, "cceGetFunctionBufferOffset was called with functionUID %llx which was not registered in functionSetID %u", \
                         (unsigned long long)functionUID, functionSetID); abort();
   #endif
   CCE_FIND_FROM_UID_ARRAY(functionUID, currentFunctions->sectionUIDs, currentFunctions->sectionUIDsSorted, currentFunctions->readingFunctionsQuantity, result, ERROR_HANDLER);
   #undef ERROR_HANDLER
   return sizeof(struct cce_buffer) + currentFunctions->readingFunctionsDataBufferOffsets[result];
}

CCE_API int cceRegisterFileIOcallbacks (uint16_t functionSet, uint32_t functionUID, cce_freadfun onLoad, cce_dataparsefun onFree, cce_dataparsefun onCreate, cce_fwritefun onWrite, size_t bufferSize)
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
      CCE_REALLOC_UID_ARRAY(currentFunctions->sectionUIDs, currentFunctions->sectionUIDsSorted, currentFunctions->readingFunctionsQuantity, currentFunctions->readingFunctionsAllocated);
   }
   currentFunctions->readingFunctions[currentFunctions->readingFunctionsQuantity]  = onLoad;
   currentFunctions->freeingFunctions[currentFunctions->readingFunctionsQuantity]  = onFree;
   currentFunctions->creatingFunctions[currentFunctions->readingFunctionsQuantity] = onCreate;
   currentFunctions->writingFunctions[currentFunctions->readingFunctionsQuantity]  = onWrite;
   currentFunctions->readingFunctionsDataBufferOffsets[currentFunctions->readingFunctionsQuantity + 1] = bufferSize + currentFunctions->readingFunctionsDataBufferOffsets[currentFunctions->readingFunctionsQuantity];
   CCE_INSERT_INTO_UID_ARRAY(functionUID, currentFunctions->sectionUIDs, currentFunctions->sectionUIDsSorted, currentFunctions->readingFunctionsQuantity);
   ++currentFunctions->readingFunctionsQuantity;
   return 0;
}

CCE_API struct cce_buffer* cceSetBufferSectionQuantity (struct cce_buffer *buffer, uint8_t newSectionsQuantity)
{
   uint8_t oldSectionsQuantity = buffer->sectionsQuantity;
   size_t size = IOfunctionSet[buffer->loadingFunctionBlockID].readingFunctionsDataBufferOffsets[newSectionsQuantity];
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
   size_t size = IOfunctionSet[functionSetID].readingFunctionsDataBufferOffsets[sectionsQuantity];
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
   uint8_t errorLoader = 0;
   struct cce_buffer *buffer = NULL;
   fread(&loaders, sizeof(uint8_t), 1, file);
   uint16_t sectionSizes[255];
   uint8_t  sectionsInitialized[255] = {0};
   uint16_t ids[255];
   headSize = 0;
   {
      uint32_t uids[255] = {0};
      fread(uids, sizeof(uint32_t), loaders, file);
      fread(sectionSizes, sizeof(uint16_t), loaders, file);
      for (unsigned i = 0; i < loaders; ++i)
      {
         CCE_FIND_FROM_UID_ARRAY(uids[i], currentFunctions->sectionUIDs, currentFunctions->sectionUIDsSorted, \
                                 currentFunctions->readingFunctionsQuantity, ids[i], fclose(file); return NULL);
         headSize = CCE_MAX(headSize, ids[i] + 1);
      }
   }
   size_t size = currentFunctions->readingFunctionsDataBufferOffsets[headSize];
   if (headSize == 0)
   {
      fclose(file);
      buffer = calloc(1, sizeof(struct cce_buffer));
      buffer->sectionsQuantity = 0;
      return buffer;
   }
   buffer = calloc(1, sizeof(struct cce_buffer) + size);
   buffer->sectionsQuantity = headSize;
   buffer->loadingFunctionBlockID = functionSetID;
   size_t *offsets = currentFunctions->readingFunctionsDataBufferOffsets;
   cce_void *data = (cce_void*)(buffer + 1);
   
   for (unsigned i = 0; i < loaders; ++i)
   {
      sectionsInitialized[ids[i]] = 1;
      if ((currentFunctions->readingFunctions[ids[i]])(data + offsets[ids[i]], sectionSizes[i], buffer, file) == 0)
         continue;
      
      errorLoader = ids[i];
      goto ERROR;
   }
   fclose(file);
   for (unsigned i = 0; i < headSize; ++i)
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
   fseek(file, (headSize) * (sizeof(uint16_t) + sizeof(uint32_t)) + sizeof(uint8_t), SEEK_SET);
   size_t *offsets = currentFunctions->readingFunctionsDataBufferOffsets;
   size_t bytesWritten = 0;
   uint16_t *sectionSizesIt = sectionSizes;
   
   for (cce_fwritefun *fun = currentFunctions->writingFunctions, *end = currentFunctions->writingFunctions + buffer->sectionsQuantity;
        fun < end; ++fun, ++sectionSizesIt, ++offsets)
   {
      *sectionSizesIt = (*fun)((cce_void*)(buffer + 1) + *offsets, buffer, file);
   }
   bytesWritten = ftell(file) - headSize * (sizeof(uint16_t) + 8 * sizeof(char)) - sizeof(uint8_t);
   fseek(file, (headSize + 1) * sizeof(uint8_t), SEEK_SET);
   uint32_t uids[255];
   {
      unsigned zeroSum = 0;
      for (unsigned i = 0; i < headSize; ++i)
      {
         uids[i - zeroSum] = currentFunctions->sectionUIDs[i];
         sectionSizes[i - zeroSum] = sectionSizes[i];
         zeroSum += (sectionSizes[i] == 0);
      }
      
      if (zeroSum > 0)
      {
         headSize -= zeroSum;
         cceMoveFileContent(file, zeroSum * (sizeof(uint16_t) + 8 * sizeof(char)), SEEK_CUR, bytesWritten);
         cceTruncateFile(file, bytesWritten + sizeof(uint8_t) + headSize * (sizeof(uint16_t) + 8 * sizeof(char)));
      }
   }
   fseek(file, 0, SEEK_SET);
   fwrite(&headSize, sizeof(uint8_t), 1, file);
   fwrite(uids, sizeof(uint32_t), headSize, file);
   fwrite(sectionSizes, sizeof(uint16_t), headSize, file);
   fclose(file);
   return 0;
}
