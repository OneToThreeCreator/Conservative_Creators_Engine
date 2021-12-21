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
#define CCE_OPTIONS CCE_EXPORTS
#else
#define CCE_OPTIONS
#endif // Windows

CCE_OPTIONS char*       getCurrentPath (char *pathBuffer, size_t pathBufferLength);
CCE_OPTIONS char*       getAppDataPath (const char *restrict folderName, size_t spaceToLeave);
CCE_OPTIONS char*       createNewPathFromOldPath (const char *const oldPath, const char *const appendPath, size_t leaveFreeSpace);
CCE_OPTIONS char*       appendPath (char *const buffer, size_t bufferSize, const char *const append);
CCE_OPTIONS char*       getTemporaryDirectory (size_t spaceToLeave);
CCE_OPTIONS void        terminateTemporaryDirectory (void);

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
