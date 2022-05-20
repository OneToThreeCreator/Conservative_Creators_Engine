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

#ifndef PATH_GETTERS_H
#define PATH_GETTERS_H
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#ifdef __cplusplus
extern C:
{
#endif // __cplusplus
#if defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__TOS_WIN__) || defined(__WINDOWS__) || \
    defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN__)
#include "cce_exports.h"
#define CCE_PUBLIC_OPTIONS CCE_EXPORTS
#else
#define CCE_PUBLIC_OPTIONS
#endif // Windows

CCE_PUBLIC_OPTIONS char* cceGetDirectory (char *path, size_t bufferSize);
CCE_PUBLIC_OPTIONS char* cceGetCurrentPath (size_t spaceToLeave);
CCE_PUBLIC_OPTIONS void  cceDeleteDirectory (const char *path);
CCE_PUBLIC_OPTIONS char* cceGetAppDataPath (const char *restrict folderName, size_t spaceToLeave);
CCE_PUBLIC_OPTIONS char* cceAppendPath (char *const buffer, size_t bufferSize, const char *const append);
CCE_PUBLIC_OPTIONS char* cceGetTemporaryDirectory (size_t spaceToLeave);
CCE_PUBLIC_OPTIONS void  cceTerminateTemporaryDirectory (void);
CCE_PUBLIC_OPTIONS char* cceConvertIntToBase64String (size_t number, char *restrict buffer, uint8_t symbolsQuantity);

static inline void shortToString (char *str, const unsigned short number, const char *strEnd)
{
   size_t lengthEnd = strlen(strEnd);
   str += strlen(str);
   if (number > 9)
   {
      if (number > 99)
      {
         if (number > 999)
         {
            if (number > 9999)
            {
               *(str++) = number / 10000 + '0';
            }
            *(str++) = number % 10000 / 1000 + '0';
         }
         *(str++) = number % 1000 / 100 + '0';
      }
      *(str++) = number % 100 / 10 + '0';
   }
   *(str++) = number % 10 + '0';
   memcpy(str, strEnd, lengthEnd + 1u);
}
#ifdef __cplusplus
}
#endif // __cplusplus
#endif //PATH_GETTERS_H
