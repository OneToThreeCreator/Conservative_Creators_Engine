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

#include "platforms.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "../../include/coffeechain/path_getters.h"
#include "../../include/coffeechain/utils.h"

#ifdef __OPTIMIZE_SIZE__
typedef uint32_t cce_uint;
#else
typedef uint_fast32_t cce_uint;
#endif // __OPTIMIZE_SIZE__

#define CCE_TMPDIR_NAME_TEMPLATE "CoffeeChain-tmpdir-XXXXXX"
#define CCE_TMPDIR_NAME_TEMPLATE_SIZE 25u

static char *tmpPath = NULL;
static size_t tmpPathLength;

CCE_PUBLIC_OPTIONS char* cceCreateNewPathFromOldPath (const char *const oldPath, const char *const appendPath, size_t freeSpaceToLeave)
{
   char *newPath;
   size_t oldPathSize = strlen(oldPath);
   size_t appendPathSize = strlen(appendPath);
   if (*(oldPath + oldPathSize - 1u) == '/' || *(oldPath + oldPathSize - 1u) == '\\')
   {
      newPath = malloc(oldPathSize + appendPathSize + freeSpaceToLeave + 1u); // \0
      memcpy(newPath, oldPath, oldPathSize);
   }
   else
   {
      newPath = malloc(oldPathSize + appendPathSize + freeSpaceToLeave + 2u); // \0 and /
      memcpy(newPath, oldPath, oldPathSize);
      #ifdef WINDOWS_SYSTEM
      *(newPath + oldPathSize) = '\\';
      #else
      *(newPath + oldPathSize) = '/';
      #endif // WINDOWS_SYSTEM
      ++oldPathSize;
   }
   memcpy(newPath + oldPathSize, appendPath, appendPathSize);
   *(newPath + oldPathSize + appendPathSize) = '\0';
   return newPath;
}

CCE_PUBLIC_OPTIONS char* cceAppendPath (char *const buffer, size_t bufferSize, const char *const append)
{
   size_t oldPathLength = strnlen(buffer, bufferSize);
   size_t appendPathLength = strlen(append);
   if (bufferSize <= oldPathLength + appendPathLength + 1u)
   {
      return NULL;
   }
   if (!(*(buffer + oldPathLength - 1u) == '/' || *(buffer + oldPathLength - 1u) == '\\'))
   {
      #ifdef WINDOWS_SYSTEM
      *(buffer + oldPathLength) = '\\';
      #else
      *(buffer + oldPathLength) = '/';
      #endif // WINDOWS_SYSTEM
      ++oldPathLength;
   }
   memcpy(buffer + oldPathLength, append, appendPathLength);
   *(buffer + oldPathLength + appendPathLength) = '\0';
   return buffer;
}

void cce__shortToString (char *str, const unsigned short number, const char *strEnd)
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

CCE_PUBLIC_OPTIONS char* cceConvertIntToBase64String (size_t number, char *restrict buffer, uint8_t symbolsQuantity)
{
   static const char
   dictionary[64] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                     'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                     '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '-', '_'};
   for (uint8_t i = 0u; i < symbolsQuantity; ++i)
   {
      *(buffer + symbolsQuantity - i - 1u) = *(dictionary + ((number >> (i * 6)) & 63));
   }
   return buffer;
}

CCE_PUBLIC_OPTIONS void cceTerminateTemporaryDirectory (void)
{
   if (!tmpPath)
   {
      return;
   }
   cceDeleteDirectory(tmpPath);
   free(tmpPath);
   tmpPath = NULL;
}

#if defined(POSIX_SYSTEM)

#include <errno.h>
#include <pwd.h>
#include <unistd.h>
#include <ftw.h>

#define DEFAULT_PATH_LENGTH 256

CCE_PUBLIC_OPTIONS char* cceGetCurrentPath (size_t spaceToLeave)
{
   char *path;
   size_t pathLength;
   for (size_t i = 1;; ++i)
   {
      pathLength = DEFAULT_PATH_LENGTH * i;
      path = malloc(pathLength);
      if (getcwd(path, pathLength) != NULL)
         break;

      free(path);
      switch (errno)
      {
         case ERANGE:
            errno = 0;
            continue;
         default:
            return NULL;
      }
   }
   return realloc(path, strnlen(path, pathLength) + 1 + spaceToLeave);
}

CCE_PUBLIC_OPTIONS char* cceGetDirectory (char *path, size_t bufferSize)
{
   struct stat st;
   int mkdirState, statState = stat(path, &st);
   if (statState != 0)
   {
      switch (errno)
      {
         case ENOENT:
         {
            errno = 0u;
            mkdirState = mkdir(path, S_IRWXU | S_IRWXG);
            if (mkdirState != 0)
            {
               if (errno == EEXIST)
               {
                  // Dangling pointer found
                  errno = 0u;
                  break;
               }
            }
            else
            {
               return path;
            }
         }
         // fallthrough
         default:
         {
            fprintf(stderr, "DIRECTORY::FAILED_TO_GET:\n%s - %s\n", path, strerror(errno));
            return NULL;
         }
      }
   }
   else if (S_ISDIR(st.st_mode))
   {
      return path;
   }
   size_t length = strnlen(path, bufferSize);
   if (length + 1u >= bufferSize)
   {
      return NULL;
   }
   size_t symbolsRemaining = bufferSize - (length + 1u);
   for (size_t symbolsQuantity = 1u; symbolsQuantity <= symbolsRemaining; ++symbolsQuantity)
   {
      for (size_t number = 0u; number < (((size_t) 1) << (symbolsQuantity * 6u)); ++number)
      {
         cceConvertIntToBase64String(number, path + length, symbolsQuantity);
         *(path + length + symbolsQuantity) = '\0';
         statState = stat(path, &st);
         if (statState != 0)
         {
            switch (errno)
            {
               case ENOENT:
               {
                  errno = 0u;
                  mkdirState = mkdir(path, S_IRWXU | S_IRWXG);
                  if (mkdirState != 0)
                  {
                     if (errno == EEXIST)
                     {
                        // Dangling pointer found
                        errno = 0u;
                        break;
                     }
                  }
                  else
                  {
                     return path;
                  }
               }
               // fallthrough
               default:
               {
                  *(path + length) = '\0';
                  fprintf(stderr, "DIRECTORY::FAILED_TO_GET:\n%s - %s\n", path, strerror(errno));
                  return NULL;
               }
            }
         }
         else if (S_ISDIR(st.st_mode))
         {
            return path;
         }
      }
   }
   *(path + length) = '\0';
   fprintf(stderr, "DIRECTORY::FAILED_TO_GET:\n%s - %s\n", path, strerror(errno));
   return NULL;
}

#if (defined(__APPLE__) && defined(__MACH__))
#define DEFAULT_APPDATA_FOLDER "/Library/Application Support/"
#define APPDATA_APPEND_SIZE 29u // strlen("/Library/Application Support/") == 29
#else 
#define DEFAULT_APPDATA_FOLDER "/.local/share/"
#define APPDATA_APPEND_SIZE 14u // strlen("/.local/share/") == 14
#endif

CCE_PUBLIC_OPTIONS char* cceGetAppDataPath (const char *restrict folderName, size_t spaceToLeave)
{
   struct stat st;
   char *restrict appDataPath;
   const char *restrict path = getenv("XDG_DATA_HOME");
   size_t pathLength, folderNameLength = strlen(folderName);
   if ((path != NULL) && (*path != '\0') && (stat(path, &st) != 0) && !S_ISDIR(st.st_mode))
   {
      pathLength = strlen(path) + 1u; // '/'
      appDataPath = malloc((pathLength + folderNameLength + spaceToLeave + 1u + 1u) * sizeof(char)); /* One symbol just in case */
      memcpy(appDataPath, path, pathLength - 1u);
      if (*(appDataPath + pathLength - 2u) == '/')
      {
         (*(appDataPath + pathLength - 1u)) = '/';
      }
      else
      {
         --pathLength;
      }
   }
   else
   {
      path = getenv("HOME");
      if ((path == NULL) || (*path == '\0') || (stat(path, &st) != 0) || !S_ISDIR(st.st_mode))
      {
         errno = 0u;
         struct passwd *pw = getpwuid(getuid());
         path = pw->pw_dir;
         if ((path == NULL) || (stat(path, &st) != 0) || !S_ISDIR(st.st_mode))
         {
            perror("DIRECTORY::HOME::CANNOT_GET");
            return NULL;
         }
      }
      pathLength = strlen(path);
      appDataPath = malloc((pathLength + folderNameLength + APPDATA_APPEND_SIZE + spaceToLeave + 1u + 1u) * sizeof(char)); /* One symbol just in case */
      memcpy(appDataPath, path, pathLength);
      memcpy(appDataPath + pathLength, DEFAULT_APPDATA_FOLDER, APPDATA_APPEND_SIZE + 1u);
      if((stat(appDataPath, &st) != 0) || !(S_ISDIR(st.st_mode)))
      {
         errno = 0u;
         *(appDataPath + pathLength + 0u) = '/';
         *(appDataPath + pathLength + 1u) = '.';
         pathLength += 2u;
      }
      else
      {
         pathLength += APPDATA_APPEND_SIZE;
      }
   }
   memcpy(appDataPath + pathLength, folderName, folderNameLength + 1u);
   pathLength += folderNameLength;
   char *result = cceGetDirectory(appDataPath, pathLength + 1u + 1u + spaceToLeave);
   if (!result)
   {
      free(appDataPath);
      return NULL;
   }
   result = realloc(appDataPath, strnlen(result, pathLength + 1u + 1u + spaceToLeave) + 1u + spaceToLeave);
   return result;
}

CCE_PUBLIC_OPTIONS char* cceGetTemporaryDirectory (size_t spaceToLeave)
{
   if (!tmpPath)
   {
      char *path = getenv("TMPDIR");
      struct stat st;
      if (!path || *path == '\0' || (stat(path, &st) != 0u) || !S_ISDIR(st.st_mode))
      {
         path = "/tmp";
      }
      tmpPathLength = strlen(path);
      size_t isMissingSlash = *(path + tmpPathLength - 1u) != '/';
      tmpPath = malloc((tmpPathLength + CCE_TMPDIR_NAME_TEMPLATE_SIZE + 2u + isMissingSlash) * sizeof(char));
      memcpy(tmpPath, path, tmpPathLength);
      *(tmpPath + tmpPathLength) = '/';
      memcpy(tmpPath + tmpPathLength + isMissingSlash, CCE_TMPDIR_NAME_TEMPLATE, CCE_TMPDIR_NAME_TEMPLATE_SIZE + 1u);
      tmpPathLength += CCE_TMPDIR_NAME_TEMPLATE_SIZE + isMissingSlash;
      path = mkdtemp(tmpPath);
      
      if (!path)
      {
         free(tmpPath);
         perror("DIRECTORY::TEMPORARY::FAILED_TO_CREATE");
         return NULL;
      }
      tmpPath = path;
      *(tmpPath + tmpPathLength) = '\0';
   }
   char *buffer = malloc((tmpPathLength + spaceToLeave + 1u) * sizeof(char));
   return memcpy(buffer, tmpPath, tmpPathLength + 1u);
}

static int removeCallback (const char *path, const struct stat *st, int type, struct FTW *info)
{
   CCE_UNUSED(st);
   CCE_UNUSED(info);
   switch (type)
   {
      case FTW_F:
      case FTW_SL:
      {
         return remove(path);
      }
      case FTW_D:
      case FTW_DP:
      {
         return rmdir(path);
      }
      default:
      {
         return -1;
      }
   }
}

CCE_PUBLIC_OPTIONS void cceDeleteDirectory (const char *path)
{
   int nftwState = nftw(path, removeCallback, FOPEN_MAX, FTW_DEPTH | FTW_MOUNT | FTW_PHYS);
   if (nftwState != 0)
   {
      fprintf(stderr, "DIRECTORY::FAILED_TO_REMOVE:\n%s path cannot be removed - %s", path, strerror(errno));
   }
}

#elif defined(WINDOWS_SYSTEM)

#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <time.h>

static void printSystemError (char *message)
{
   DWORD error = GetLastError();
   char *errorString;
   FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, error, 0u, (LPTSTR) &errorString, 0u, NULL);
   if (errorString == NULL)
   {
      fprintf(stderr, "%s: error code %u", message, error);
   }
   else
   {
      fprintf(stderr, "%s: %s", message, errorString);
      LocalFree(errorString);
   }
}

static char* getErrorMessage (DWORD error)
{
   static char errorString[256u];
   FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, error, 0u, (LPTSTR) errorString, 256u, NULL);
   return errorString;
}

CCE_PUBLIC_OPTIONS char* cceGetCurrentPath (size_t spaceToLeave)
{
   size_t size = (spaceToLeave + MAX_PATH) * sizeof(char);
   char *path = malloc(size);
   GetCurrentDirectoryA(size, path);
   return realloc(path, strnlen(path, size) + 1 + spaceToLeave);
}

CCE_PUBLIC_OPTIONS char* cceGetDirectory (char *path, size_t bufferSize)
{
   DWORD attributes = GetFileAttributesA(path);
   DWORD error;
   if (attributes == INVALID_FILE_ATTRIBUTES)
   {
      error = GetLastError();
      switch (error)
      {
         case ERROR_FILE_NOT_FOUND:
         {
            SetLastError(ERROR_SUCCESS);
            if (CreateDirectoryA(path, NULL) == 0u) // CreateDirectoryA returns zero if failed
            {
               error = GetLastError();
            }
            else
            {
               return path;
            }
         }
         // fallthrough
         default:
         {
            fprintf(stderr, "DIRECTORY::FAILED_TO_GET:\n%s - %s", path, getErrorMessage(error));
            return NULL;
         }
      }
   }
   if (attributes & FILE_ATTRIBUTE_DIRECTORY)
   {
      return path;
   }
   
   size_t length = strnlen(path, bufferSize);
   if ((*(path + length - 1) == '\\') || (*(path + length - 1) == '/'))
       --length;
   uint8_t symbolsRemaining = bufferSize - (length + 1u);
   for (uint8_t symbolsQuantity = 1u; symbolsQuantity <= symbolsRemaining; ++symbolsQuantity)
   {
      for (size_t number = 0u; number < (((size_t) 1) << (symbolsQuantity * 6u)); ++number)
      {
         cceConvertIntToBase64String(number, path + length, symbolsQuantity);
         *(path + length + symbolsQuantity) = '\0';
         attributes = GetFileAttributesA(path);
         if (attributes == INVALID_FILE_ATTRIBUTES)
         {
            error = GetLastError();
            switch (error)
            {
               case ERROR_FILE_NOT_FOUND:
               {
                  SetLastError(ERROR_SUCCESS);
                  if (CreateDirectoryA(path, NULL) == 0u) // CreateDirectoryA returns zero if failed
                  {
                     error = GetLastError();
                  }
                  else
                  {
                     return path;
                  }
               }
               // fallthrough
               default:
               {
                  *(path + length) = '\0';
                  fprintf(stderr, "DIRECTORY::FAILED_TO_GET:\n%s - %s\n", path, getErrorMessage(error));
                  return NULL;
               }
            }
         }
         else if (attributes & FILE_ATTRIBUTE_DIRECTORY)
         {
            return path;
         }
      }
   }
   *(path + length) = '\0';
   fprintf(stderr, "DIRECTORY::FAILED_TO_GET:\n%s - %s\n", path, getErrorMessage(error));
   return NULL;
}

CCE_PUBLIC_OPTIONS char* cceGetAppDataPath (const char *restrict folderName, size_t spaceToLeave)
{
   char *restrict appDataPath;
   const char *restrict path = getenv("APPDATA");
   size_t pathLength, folderNameLength = strlen(folderName);
   if (path && (*path != '\0'))
   {
      pathLength = strlen(path);
      appDataPath = malloc((pathLength + folderNameLength + spaceToLeave + 1u + 1u) * sizeof(char)); // One symbol just in case
      memcpy(appDataPath, path, pathLength);
      (*(appDataPath + pathLength)) = '\\';
   }
   else
   {
      appDataPath = malloc(MAX_PATH * sizeof(char));
      if (SHGetFolderPathA(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, appDataPath) != S_OK)
      {
         printSystemError("DIRECTORY::APPDATA::FAILED_TO_RETRIEVE");
         free(appDataPath);
         return NULL;
      }
      pathLength = strlen(appDataPath);
      appDataPath = realloc(appDataPath, (pathLength + folderNameLength + spaceToLeave + 1u + 1u) * sizeof(char)); // One symbol just in case
      (*(appDataPath + pathLength)) = '\\';
   }
   memcpy(appDataPath + pathLength + 1u, folderName, folderNameLength + 1u);
   cceGetDirectory(appDataPath, pathLength + folderNameLength + spaceToLeave + 1u + 1u);
   return realloc(appDataPath, strnlen(appDataPath, pathLength + folderNameLength + spaceToLeave + 1u + 1u) + spaceToLeave + 1u);
}

CCE_PUBLIC_OPTIONS char* cceGetTemporaryDirectory (size_t spaceToLeave)
{
   if (!tmpPath)
   {
      tmpPath = malloc(MAX_PATH * sizeof(char));
      tmpPathLength = GetTempPathA(MAX_PATH, tmpPath);
      if (!tmpPathLength)
      {
         free(tmpPath);
         fprintf(stderr, "DIRECTORY::TEMPORARY::FAILED_TO_GET_TEMPORARY_PATH");
         return NULL;
      }
      char *path = malloc(MAX_PATH * sizeof(char));
      tmpPathLength = GetLongPathNameA(tmpPath, path, MAX_PATH);
      if (!tmpPathLength)
      {
         free(tmpPath);
         fprintf(stderr, "DIRECTORY::TEMPORARY::FAILED_TO_GET_LONG_PATH");
         return NULL;
      }
      
      if (tmpPathLength > MAX_PATH)
      {
         free(path);
         tmpPath = realloc(tmpPath, tmpPathLength + CCE_TMPDIR_NAME_TEMPLATE_SIZE + 2u);
         GetLongPathNameA(tmpPath, tmpPath, tmpPathLength + 1u);
      }
      else
      {
         free(tmpPath);
         tmpPath = realloc(path, tmpPathLength + CCE_TMPDIR_NAME_TEMPLATE_SIZE + 2u);
      }
      memcpy(tmpPath + tmpPathLength, CCE_TMPDIR_NAME_TEMPLATE, CCE_TMPDIR_NAME_TEMPLATE_SIZE);
      tmpPathLength += CCE_TMPDIR_NAME_TEMPLATE_SIZE;
      
      *(tmpPath + tmpPathLength) = '\\';
      ++tmpPathLength;
      *(tmpPath + tmpPathLength) = '\0';
      cceConvertIntToBase64String(time(0), tmpPath + (tmpPathLength - 6u - 1u - 2u), 6u);
      CreateDirectoryA(tmpPath, NULL);
   }
   char *buffer = malloc((tmpPathLength + spaceToLeave + 1u) * sizeof(char));
   memcpy(buffer, tmpPath, tmpPathLength + 1u);
   return buffer;
}

CCE_PUBLIC_OPTIONS void cceDeleteDirectory (const char *aPath)
{
   WIN32_FIND_DATA fileData;
   size_t bufferSize;
   size_t aPathLength = strlen(aPath);
   size_t pathLength = aPathLength;
   size_t fileNameLength;
   HANDLE file;
   char *path;
   if (pathLength > MAX_PATH)
   {
      bufferSize = pathLength + 23u; // +Null-terminator
   }
   else
   {
      bufferSize = MAX_PATH; // Null-terminator included by default
   }
   path = malloc(bufferSize * sizeof(char));
   memcpy(path, aPath, pathLength);
   if (*(path + pathLength - 1u) != '\\' && *(path + pathLength - 1u) != '/')
   {
      *(path + pathLength) = '\\';
      ++aPathLength;
      ++pathLength;
   }
   *(path + pathLength) = '*';
   *(path + pathLength + 1u) = '\0';
   for (;;)
   {
      file = FindFirstFileA(path, &fileData);
      for (;;)
      {
         fileNameLength = strlen(fileData.cFileName);
         if ((*(fileData.cFileName) != '.') || (fileNameLength > 2) || ((fileNameLength > 1) && (*(fileData.cFileName + 1) != '.')))
         {
            if ((pathLength + fileNameLength + 1u) > bufferSize)
            {
                bufferSize = pathLength + fileNameLength + 23u;
                path = realloc(path, bufferSize * sizeof(char));
            }

            memcpy(path + pathLength, fileData.cFileName, fileNameLength + 1);

            if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
            {
                pathLength += fileNameLength + 1u;
                *(path + pathLength - 1u) = '\\';
                *(path + pathLength) = '*';
                *(path + pathLength + 1u) = '\0';
                FindClose(file);
                break;
            }
            remove(path);
            *(path + pathLength) = '\0';
         }
         if (!FindNextFileA(file, &fileData))
         {
            RemoveDirectoryA(path);
            FindClose(file);
            if (pathLength <= aPathLength)
            {
               free(path);
               return;
            }
            pathLength -= 1;
            while (*(path + pathLength - 1) != '\\')
            {
               --pathLength;
            }
            *(path + pathLength) = '*';
            *(path + pathLength + 1) = '\0';
            break;
         }
      }
   }
}

#endif
