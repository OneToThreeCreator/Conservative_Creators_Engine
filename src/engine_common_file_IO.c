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
   cce_freadfun *readingFunctions;
   uint8_t readingFunctionsQuantity;
   uint8_t readingFunctionsAllocated;
   cce_dataparsefun *freeingFunctions;
   cce_dataparsefun *creatingFunctions;
   cce_fwritefun *writingFunctions;
   size_t *readingFunctionsDataBufferSizes;
   size_t bufferSize;
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

   CCE_ALLOC_ARRAY(IOfunctionSet[IOfunctionSetQuantity].readingFunctions);
   IOfunctionSet[IOfunctionSetQuantity].readingFunctionsQuantity = 0;
   IOfunctionSet[IOfunctionSetQuantity].freeingFunctions  = malloc(IOfunctionSet[IOfunctionSetQuantity].readingFunctionsAllocated * sizeof(cce_dataparsefun*));
   IOfunctionSet[IOfunctionSetQuantity].creatingFunctions = malloc(IOfunctionSet[IOfunctionSetQuantity].readingFunctionsAllocated * sizeof(cce_dataparsefun*));
   IOfunctionSet[IOfunctionSetQuantity].writingFunctions  = malloc(IOfunctionSet[IOfunctionSetQuantity].readingFunctionsAllocated * sizeof(cce_fwritefun*));
   IOfunctionSet[IOfunctionSetQuantity].readingFunctionsDataBufferSizes = malloc(IOfunctionSet[IOfunctionSetQuantity].readingFunctionsAllocated * sizeof(size_t*));
   IOfunctionSet[IOfunctionSetQuantity].bufferSize = 0;
   return IOfunctionSetQuantity++;
}

CCE_PUBLIC_OPTIONS ptrdiff_t cceGetFunctionBufferOffset (uint8_t functionID, uint16_t functionSetID)
{
   if (functionID >= IOfunctionSet[functionSetID].readingFunctionsQuantity)
      return 0;
   ptrdiff_t offset = sizeof(struct cce_buffer);
   for (size_t *iterator = IOfunctionSet[functionSetID].readingFunctionsDataBufferSizes, *end = iterator + functionID; iterator < end; ++iterator)
   {
      offset += *iterator;
   }
   return offset;
}

CCE_PUBLIC_OPTIONS uint8_t cceRegisterFileIOcallbacks (uint16_t functionSet, cce_freadfun onLoad, cce_dataparsefun onFree, cce_dataparsefun onCreate, cce_fwritefun onWrite, size_t bufferSize)
{
   struct cce_IO_function_set *currentFunctions = IOfunctionSet + functionSet;
   if (currentFunctions->readingFunctionsQuantity >= currentFunctions->readingFunctionsAllocated)
   {
      CCE_REALLOC_ARRAY(currentFunctions->readingFunctions, currentFunctions->readingFunctionsQuantity + 1);
      currentFunctions->freeingFunctions  = realloc(currentFunctions->freeingFunctions,  currentFunctions->readingFunctionsAllocated * sizeof(cce_dataparsefun*));
      currentFunctions->creatingFunctions = realloc(currentFunctions->creatingFunctions, currentFunctions->readingFunctionsAllocated * sizeof(cce_dataparsefun*));
      currentFunctions->writingFunctions  = realloc(currentFunctions->writingFunctions,  currentFunctions->readingFunctionsAllocated * sizeof(cce_fwritefun*));
      currentFunctions->readingFunctionsDataBufferSizes = realloc(currentFunctions->readingFunctionsDataBufferSizes, (currentFunctions->readingFunctionsAllocated + 1) * sizeof(size_t));
   }
   currentFunctions->readingFunctions[currentFunctions->readingFunctionsQuantity]  = onLoad;
   currentFunctions->freeingFunctions[currentFunctions->readingFunctionsQuantity]  = onFree;
   currentFunctions->creatingFunctions[currentFunctions->readingFunctionsQuantity] = onCreate;
   currentFunctions->writingFunctions[currentFunctions->readingFunctionsQuantity]  = onWrite;
   currentFunctions->readingFunctionsDataBufferSizes[currentFunctions->readingFunctionsQuantity] = bufferSize;
   currentFunctions->bufferSize += bufferSize;
   return currentFunctions->readingFunctionsQuantity++;
}

CCE_PUBLIC_OPTIONS struct cce_buffer* cceSetBufferSectionQuantity (struct cce_buffer *buffer, uint8_t newSectionsQuantity)
{
   uint8_t oldSectionsQuantity = buffer->sectionsQuantity;
   size_t size = IOfunctionSet[buffer->loadingFunctionBlockID].bufferSize;
   for (size_t *iterator = IOfunctionSet[buffer->loadingFunctionBlockID].readingFunctionsDataBufferSizes + IOfunctionSet[buffer->loadingFunctionBlockID].readingFunctionsQuantity - 1,
        *end = iterator - newSectionsQuantity; iterator >= end; --iterator)
   {
      size -= *iterator;
   }
   buffer = realloc(buffer, sizeof(struct cce_buffer) + size);
   if (newSectionsQuantity > oldSectionsQuantity)
   {
      size_t *sizes = IOfunctionSet[buffer->loadingFunctionBlockID].readingFunctionsDataBufferSizes + oldSectionsQuantity;
      cce_void *data = (cce_void*)(buffer + 1);
      for (cce_dataparsefun *iterator = IOfunctionSet[buffer->loadingFunctionBlockID].creatingFunctions + oldSectionsQuantity,
           *end = IOfunctionSet[buffer->loadingFunctionBlockID].creatingFunctions + newSectionsQuantity; iterator < end; ++iterator, data += *sizes++)
      {
         if (*iterator != NULL)
            (*iterator)(data, buffer);
      }
   }
   return buffer;
}

CCE_PUBLIC_OPTIONS struct cce_buffer* cceCreateBuffer (uint8_t sectionsQuantity, uint16_t functionSetID)
{
   size_t size = IOfunctionSet[functionSetID].bufferSize;
   for (size_t *iterator = IOfunctionSet[functionSetID].readingFunctionsDataBufferSizes + IOfunctionSet[functionSetID].readingFunctionsQuantity - 1,
        *end = IOfunctionSet[functionSetID].readingFunctionsDataBufferSizes + sectionsQuantity; iterator >= end; --iterator)
   {
      size -= *iterator;
   }
   struct cce_buffer *buffer = malloc(sizeof(struct cce_buffer) + size);
   buffer->sectionsQuantity = sectionsQuantity;
   buffer->loadingFunctionBlockID = functionSetID;
   cce_void *data = (cce_void*)(buffer + 1);
   size_t *sizes = IOfunctionSet[functionSetID].readingFunctionsDataBufferSizes;
   for (cce_dataparsefun *iterator = IOfunctionSet[functionSetID].creatingFunctions, *end = iterator + sectionsQuantity; iterator < end; ++iterator, data += *sizes++)
   {
      if (*iterator != NULL)
         (*iterator)(data, buffer);
   }
   return buffer;
}

CCE_PUBLIC_OPTIONS void cceFreeBuffer (struct cce_buffer *buffer)
{
   if (!buffer)
      return;
   struct cce_IO_function_set *currentFunctions = IOfunctionSet + buffer->loadingFunctionBlockID;
   size_t *sizes = currentFunctions->readingFunctionsDataBufferSizes;
   cce_void *data = (cce_void*) (buffer + 1);
   for (cce_dataparsefun *fun = currentFunctions->freeingFunctions, *end = currentFunctions->freeingFunctions + buffer->sectionsQuantity;
        fun < end; ++fun, data += *sizes, ++sizes)
   {
      
      (*fun)(data, buffer);
   }
   free(buffer);
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
      uint8_t *iterator = sectionSizes + headSize;
      size_t *bufferSizes = currentFunctions->readingFunctionsDataBufferSizes + headSize - 1;
      sectionSizes[0] = 1; // Workaround to avoid out-of-bounds check
      size = currentFunctions->bufferSize;
      while (*iterator == 0)
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
   struct cce_buffer *buffer = calloc(1, sizeof(struct cce_buffer) + size);
   buffer->sectionsQuantity = headSize;
   buffer->loadingFunctionBlockID = functionSetID;
   size_t *sizes = currentFunctions->readingFunctionsDataBufferSizes;
   uint8_t *sectionSizesIt = sectionSizes + 1;
   cce_void *data = (cce_void*) (buffer + 1);
   cce_dataparsefun *initFun = currentFunctions->creatingFunctions;
   
   int i = 0;
   for (cce_freadfun *fun = currentFunctions->readingFunctions, *end = currentFunctions->readingFunctions + headSize; fun < end; ++fun, ++initFun, data += *sizes++, ++sectionSizesIt, ++i)
   {
      if (*sectionSizesIt == 0)
      {
         if (*initFun != NULL)
            (*initFun)(data, buffer);
         continue;
      }
      if ((*fun)(data, *sectionSizesIt, buffer, file) == 0)
         continue;
      
      errorLoader = fun - currentFunctions->readingFunctions;
      goto ERROR;
   }
   fclose(file);
   return buffer;
ERROR:
   fclose(file);
   data = (cce_void*) (buffer + 1);
   for (cce_dataparsefun *fun = currentFunctions->freeingFunctions, *end = currentFunctions->freeingFunctions + errorLoader;
        fun < end; ++fun, data += *sizes++)
   {
      (*fun)(data, buffer);
   }
   free(buffer);
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
   size_t *sizes = currentFunctions->readingFunctionsDataBufferSizes;
   size_t bytesWritten = 0;
   uint8_t *sectionSizesIt = sectionSizes + 1;
   cce_void *data = (cce_void*) (buffer + 1);
   
   int i = 0;
   for (cce_fwritefun *fun = currentFunctions->writingFunctions, *end = currentFunctions->writingFunctions + buffer->sectionsQuantity;
        fun < end; ++fun, ++sectionSizesIt, data += *sizes, ++sizes, ++i)
   {
      *sectionSizesIt = (*fun)(data, buffer, file);
   }
   bytesWritten = ftell(file) - (1 + headSize) * sizeof(uint8_t);
   fseek(file, (headSize + 1) * sizeof(uint8_t), SEEK_SET);
   {
      uint8_t *iterator = sectionSizes + headSize;
      sectionSizes[0] = 1; // Workaround to avoid out-of-bounds check
      while (*iterator == 0)
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
   fwrite(sectionSizes + 1, sizeof(uint8_t), buffer->sectionsQuantity, file);
   fclose(file);
   return 0;
}
