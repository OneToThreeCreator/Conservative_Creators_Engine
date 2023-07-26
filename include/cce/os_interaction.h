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

#ifndef OS_INTERACTION_H
#define OS_INTERACTION_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include "cce_exports.h"

#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__TOS_WIN__) || defined(__WINDOWS__) || \
    defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN__)
#define cceIsPathDelimiter(x) ((x) == '/' || (x) == '\\')
#define cceIsPathAbsolute(path) (path[1] == ':' ? (path[2] == '/' || path[2] == '\\') : (path[0] == '/' || path[0] == '\\'))
#define cceNativePathDelimiter '\\'
#else
#define cceIsPathDelimiter(x) ((x) == '/')
#define cceIsPathAbsolute(path) (path[0] == '/')
#define cceNativePathDelimiter '/'
#endif

CCE_API char*    cceCreateNewPathFromOldPath (const char *oldPath, const char *appendPath, size_t freeSpaceToLeave);
CCE_API void     cceTruncateFile (FILE *file, size_t size);
CCE_API char*    cceGetAbsolutePath (const char *path, size_t spaceToLeave);
CCE_API int      cceSetCurrentPath (const char *path);
CCE_API char*    cceGetDirectory (char *path, size_t bufferSize);
CCE_API char*    cceGetCurrentPath (size_t spaceToLeave);
CCE_API void     cceDeleteDirectory (const char *path);
CCE_API char*    cceGetAppDataPath (const char *folderName, size_t spaceToLeave);
CCE_API char*    cceAppendPath (char *buffer, size_t bufferSize, const char *append);
CCE_API char*    cceGetTemporaryDirectory (size_t spaceToLeave);
CCE_API void     cceTerminateTemporaryDirectory (void);
CCE_API uint8_t  cceIsDirectory (char *path);
CCE_API int      cceGetRandomSeed (void *buffer, size_t bufferSize);
/* Has millisecond precision, overflows every 49.7 days. */
CCE_API uint32_t cceGetMonotonicTime (void);
int              cce__iniOsInteraction ();

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // OS_INTERACTION_H
