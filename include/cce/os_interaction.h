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
#define CCE_PUBLIC_OPTIONS CCE_EXPORTS

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

CCE_PUBLIC_OPTIONS char* cceCreateNewPathFromOldPath (const char *oldPath, const char *appendPath, size_t freeSpaceToLeave);
CCE_PUBLIC_OPTIONS void  cceTruncateFile (FILE *file, size_t size);
CCE_PUBLIC_OPTIONS char* cceGetAbsolutePath (const char *path, size_t spaceToLeave);
CCE_PUBLIC_OPTIONS int   cceSetCurrentPath (const char *path);
CCE_PUBLIC_OPTIONS char* cceGetDirectory (char *path, size_t bufferSize);
CCE_PUBLIC_OPTIONS char* cceGetCurrentPath (size_t spaceToLeave);
CCE_PUBLIC_OPTIONS void  cceDeleteDirectory (const char *path);
CCE_PUBLIC_OPTIONS char* cceGetAppDataPath (const char *folderName, size_t spaceToLeave);
CCE_PUBLIC_OPTIONS char* cceAppendPath (char *buffer, size_t bufferSize, const char *append);
CCE_PUBLIC_OPTIONS char* cceGetTemporaryDirectory (size_t spaceToLeave);
CCE_PUBLIC_OPTIONS void  cceTerminateTemporaryDirectory (void);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //OS_INTERACTION_H
