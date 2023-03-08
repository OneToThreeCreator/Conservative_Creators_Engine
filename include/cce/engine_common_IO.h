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

#ifndef ENGINE_COMMON_IO_H
#define ENGINE_COMMON_IO_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>

#include "cce_exports.h"

struct cce_buffer
{
   uint8_t sectionsQuantity;
   uint8_t flags;
   uint16_t loadingFunctionBlockID;
   uint32_t __pad; // Pad to uint64_t (to avoid misaligned reads down the line)
};

typedef int     (*cce_freadfun)(void *buffer, uint16_t sectionSize, struct cce_buffer *info, FILE *file);
typedef uint16_t (*cce_fwritefun)(void *buffer, struct cce_buffer *info, FILE *file);
typedef void    (*cce_dataparsefun)(void *buffer, struct cce_buffer *info);

CCE_API FILE* cceMoveFileContent (FILE *file, long offset, int position, size_t size);
CCE_API uint16_t cceGetFileIOfunctionSet (void);
CCE_API ptrdiff_t cceGetFunctionBufferOffset (uint16_t functionID, uint16_t functionSetID);
CCE_API uint8_t cceRegisterFileIOcallbacks (uint16_t functionSet, char sectionName[8], cce_freadfun onLoad, cce_dataparsefun onFree, cce_dataparsefun onCreate, cce_fwritefun onWrite, size_t bufferSize);
CCE_API struct cce_buffer* cceSetBufferSectionQuantity (struct cce_buffer *buffer, uint8_t newSectionsQuantity);
CCE_API struct cce_buffer* cceCreateBuffer (uint8_t sectionsQuantity, uint16_t functionSetID);
CCE_API void cceFreeBuffer (struct cce_buffer *buffer);
CCE_API struct cce_buffer* cceLoadBinaryCCF (char *path, uint16_t functionSetID);
CCE_API int cceWriteBinaryCCF (struct cce_buffer *buffer, char *path);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ENGINE_COMMON_IO_H
