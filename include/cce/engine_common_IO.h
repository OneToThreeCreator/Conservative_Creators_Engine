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
#define CCE_API CCE_EXPORTS

struct cce_buffer
{
   uint8_t sectionsQuantity;
   uint8_t flags;
   uint16_t loadingFunctionBlockID;
   uint32_t __pad; // Pad to uint64_t (to avoid misaligned reads down the line)
};

typedef int     (*cce_freadfun)(void *buffer, uint8_t sectionSize, struct cce_buffer *info, FILE *file);
typedef uint8_t (*cce_fwritefun)(void *buffer, struct cce_buffer *info, FILE *file);
typedef void    (*cce_dataparsefun)(void *buffer, struct cce_buffer *info);

CCE_API FILE* cceMoveFileContent (FILE *file, long offset, int position, size_t size);
CCE_API uint16_t cceGetFileIOfunctionSet (void);
CCE_API ptrdiff_t cceGetFunctionBufferOffset (uint8_t functionID, uint16_t functionSetID);
CCE_API uint8_t cceRegisterFileIOcallbacks (uint16_t functionSet, cce_freadfun onLoad, cce_dataparsefun onFree, cce_dataparsefun onCreate, cce_fwritefun onWrite, size_t bufferSize);
CCE_API struct cce_buffer* cceSetBufferSectionQuantity (struct cce_buffer *buffer, uint8_t newSectionsQuantity);
CCE_API struct cce_buffer* cceCreateBuffer (uint8_t sectionsQuantity, uint16_t functionSetID);
CCE_API void cceFreeBuffer (struct cce_buffer *buffer);
CCE_API struct cce_buffer* cceLoadBinaryCCF (char *path, uint16_t functionSetID);
CCE_API int cceWriteBinaryCCF (struct cce_buffer *buffer, char *path);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // ENGINE_COMMON_IO_H
