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

#include "platforms.h"

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include "../../include/cce/os_interaction.h"
#include "../../include/cce/utils.h"

#if defined(CCE_TMPDIR_NAME_TEMPLATE)

#ifndef CCE_TMPDIR_NAME_TEMPLATE_SIZE
#define CCE_TMPDIR_NAME_TEMPLATE_SIZE strlen(CCE_TMPDIR_NAME_TEMPLATE)
#endif // CCE_TMPDIR_NAME_TEMPLATE_SIZE

#else

#define CCE_TMPDIR_NAME_TEMPLATE "CCE-tmpdir-XXXXXX"
#define CCE_TMPDIR_NAME_TEMPLATE_SIZE 17u

#endif // defined(CCE_TMPDIR_NAME_TEMPLATE)

static char *tmpPath = NULL;
static size_t tmpPathLength;

CCE_API char* cceCreateNewPathFromOldPath (const char *oldPath, const char *appendPath, size_t freeSpaceToLeave)
{
   char *newPath;
   size_t oldPathSize = strlen(oldPath);
   size_t appendPathSize = strlen(appendPath);
   uint8_t hasPathDelimiter = oldPath[oldPathSize - 1u] == '/' || oldPath[oldPathSize - 1u] == '\\';
   newPath = malloc(oldPathSize + appendPathSize + freeSpaceToLeave + 1u + !hasPathDelimiter); // \0
   memcpy(newPath, oldPath, oldPathSize);
   #ifdef WINDOWS_SYSTEM
   newPath[oldPathSize] = hasPathDelimiter ? newPath[oldPathSize] : '\\';
   #else
   newPath[oldPathSize] = hasPathDelimiter ? newPath[oldPathSize] : '/';
   #endif // WINDOWS_SYSTEM
   oldPathSize += !hasPathDelimiter;
   memcpy(newPath + oldPathSize, appendPath, appendPathSize);
   *(newPath + oldPathSize + appendPathSize) = '\0';
   return newPath;
}

CCE_API char* cceAppendPath (char *buffer, size_t bufferSize, const char *append)
{
   size_t oldPathLength = strlen(buffer);
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

CCE_API void cceTerminateTemporaryDirectory (void)
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
#include <time.h>

time_t engineStartTimeSec = 0;

#if defined(linux) || defined(__linux) || defined(__linux__)

static uint8_t doClockCoarseHaveEnoughPrecision;

#include <sys/random.h>

CCE_API int cceGetRandomSeed (void *buffer, size_t bufferSize)
{
   return (getrandom(buffer, bufferSize, 0) == (ssize_t)bufferSize) - 1;
}

CCE_API uint32_t cceGetMonotonicTime (void)
{
   
   struct timespec tp;
   clock_gettime((doClockCoarseHaveEnoughPrecision) ? CLOCK_MONOTONIC_COARSE : CLOCK_MONOTONIC, &tp);
   return (tp.tv_sec - engineStartTimeSec) * 1000 + tp.tv_nsec / 1000000;
}

#define CCE_INI_TIME() \
do \
{ \
   struct timespec tp; \
   clock_getres(CLOCK_MONOTONIC_COARSE, &tp); \
   doClockCoarseHaveEnoughPrecision = tp.tv_nsec <= 1000000 && tp.tv_sec == 0; \
   engineStartTimeSec = cceGetMonotonicTime(); \
} \
while (0)

#elif defined(__APPLE__) && defined(__MACH__)

static mach_timebase_info_data_t timebase = {0, 0};

#include <mach/mach_time.h>
CCE_API uint32_t cceGetMonotonicTime (void)
{
   uint64_t time = mach_absolute_time();
   return (time - engineStartTimeSec) * timebase.numer / (timebase.denom * 1000000);
}

#define CCE_INI_TIME() \
(void) mach_timebase_info(&timebase); \
engineStartTimeSec = mach_absolute_time()

#elif defined(_POSIX_MONOTONIC_CLOCK) && _POSIX_MONOTONIC_CLOCK >= 0

CCE_API uint32_t cceGetMonotonicTime (void)
{
   struct timespec tp;
   clock_gettime(CLOCK_MONOTONIC, &tp);
   return (tp.tv_sec - engineStartTimeSec) * 1000 + tp.tv_nsec / 1000000;
}

#define CCE_INI_TIME() \
do \
{ \
   struct timespec tp; \
   if (clock_gettime(CLOCK_MONOTONIC, &tp) != 0) \
   {
      fputs("ENGINE::OS_INTERACTION::MONOTONIC_CLOCKS_UNAVAILABLE:\nThe system does not support monotonic clocks required for engine operation\n", stderr); \
      return -1; \
   } \
} \
while (0)

#endif // if LINUX elif MAC_OS elif defined(_POSIX_MONOTONIC_CLOCK) && _POSIX_MONOTONIC_CLOCK >= 0

#define DEFAULT_PATH_LENGTH 256

int cce__iniOsInteraction ()
{
   CCE_INI_TIME();
   return 0;
}

#if !defined(linux) || !defined(__linux) || !defined(__linux__)

#include <fcntl.h>

CCE_API int cceGetRandomSeed (void *buffer, size_t bufferSize)
{
   int fd = open("/dev/urandom", O_RDONLY);
   if (fd < 0)
      return -1;
   ssize_t bytesRead = read(fd, buffer, bufferSize);
   return (close(fd) == 0 && bytesRead == (ssize_t)bufferSize) - 1;
}

#endif // if !defined(linux) || !defined(__linux) || !defined(__linux__)

CCE_API void cceTruncateFile (FILE *file, size_t size)
{
   ftruncate(fileno(file), size);
}

CCE_API char* cceGetAbsolutePath (const char *path, size_t spaceToLeave)
{
   char *result = realpath(path, NULL);
   if (result == NULL)
      return NULL;
   return realloc(result, strlen(result) + 1 + spaceToLeave);
}

CCE_API int cceSetCurrentPath (const char *path)
{
   return chdir(path);
}

CCE_API char* cceGetCurrentPath (size_t spaceToLeave)
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

CCE_API char* cceGetDirectory (char *path, size_t bufferSize)
{
   struct stat st;
   size_t length = strnlen(path, bufferSize);
   length -= (path[length - 1] == '/'); 
   size_t symbolsRemaining = bufferSize - (length + 1u);
   for (size_t symbolsQuantity = 0u; symbolsQuantity <= symbolsRemaining; ++symbolsQuantity)
   {
      for (size_t number = 0u; number < (((size_t) 1) << (symbolsQuantity * 6u)); ++number)
      {
         cceConvertIntToBase64String(number, path + length, symbolsQuantity);
         *(path + length + symbolsQuantity) = '\0';
         if (stat(path, &st) != 0)
         {
            switch (errno)
            {
               case ENOENT:
               {
                  errno = 0u;
                  if (mkdir(path, S_IRWXU | S_IRWXG) != 0)
                  {
                     if (errno == EEXIST)
                     {
                        // Dangling link found
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

CCE_API char* cceGetAppDataPath (const char *folderName, size_t spaceToLeave)
{
   struct stat st;
   char *appDataPath;
   const char *path = getenv("XDG_DATA_HOME");
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

CCE_API char* cceGetTemporaryDirectory (size_t spaceToLeave)
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

CCE_API uint8_t cceIsDirectory (char *path)
{
   struct stat st;
   errno = 0;
   return stat(path, &st) != 0u && S_ISDIR(st.st_mode);
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

CCE_API void cceDeleteDirectory (const char *path)
{
   int nftwState = nftw(path, removeCallback, FOPEN_MAX, FTW_DEPTH | FTW_MOUNT | FTW_PHYS);
   if (nftwState != 0)
   {
      fprintf(stderr, "DIRECTORY::FAILED_TO_REMOVE:\n%s path cannot be removed - %s", path, strerror(errno));
   }
}

#elif defined(WINDOWS_SYSTEM)

#include <io.h>
#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <wincrypt.h>

LARGE_INTEGER performanceCounterFrequency;
LARGE_INTEGER engineStartTime;

static void printSystemError (char *message)
{
   DWORD error = GetLastError();
   char *errorString;
   FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                 NULL, error, 0u, (LPTSTR) &errorString, 0u, NULL);
   if (errorString == NULL)
   {
      fprintf(stderr, "%s: error code %u\n", message, error);
   }
   else
   {
      fprintf(stderr, "%s: %s\n", message, errorString);
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

CCE_API cceGetMonotonicTime (void)
{
   LARGE_INTEGER time;
   QueryPerformanceCounter(&time);
   #ifdef _WIN64
   return (time.QuadPart - engineStartTime.QuadPart) * 1000 / performanceCounterFrequency.QuadPart;
   #else
   DWORD ticks = time.LowPart - engineStartTime.LowPart;
   return ticks * 1000 / performanceCounterFrequency.LowPart | ticks / performanceCounterFrequency.LowPart * 1000;
   #endif
}

int cce__iniOsInteraction ()
{
   QueryPerformanceFrequency(&performanceCounterFrequency);
   QueryPerformanceCounter(&engineStartTime);
   return 0;
}

CCE_API int cceGetRandomSeed (void *buffer, size_t bufferSize)
{
   HCRYPTPROV cryptProvider = NULL;
   const char *containerName = "CCEKeyContainer";
   if (!CryptAcquireContext(&cryptProvider, containerName, NULL, PROV_RSA_FULL, 0) &&
       (GetLastError() != NTE_BAD_KEYSET ||
       !CryptAcquireContext(&cryptProvider, containerName, NULL, PROV_RSA_FULL, CRYPT_NEWKEYSET)))
   {
      return -1;
   }
   uint8_t result = CryptGenRandom(cryptProvider, bufferSize, buffer);
   return ((CryptReleaseContext(cryptProvider, 0) != 0) && result != 0) - 1;
}

CCE_API void cceTruncateFile (FILE *file, size_t size)
{
   long position = ftell(file);
   fseek(file, size, SEEK_SET);
   SetEndOfFile(_get_osfhandle(_fileno(file)));
   fseek(file, position, SEEK_SET);
}

CCE_API char* cceGetAbsolutePath (const char *path, size_t spaceToLeave)
{
   char *result = malloc(MAX_PATH * sizeof(char));
   size_t len = GetFullPathNameA(path, MAX_PATH, result, NULL);
   if (len == 0)
   {
      free(result);
      return NULL;
   }
   return realloc(result, len + 1 + spaceToLeave);
}

CCE_API int cceSetCurrentPath (const char *path)
{
   return -(!SetCurrentDirectoryA(path));
}

CCE_API char* cceGetCurrentPath (size_t spaceToLeave)
{
   char *path = malloc(MAX_PATH * sizeof(char));
   size_t len = GetCurrentDirectoryA(MAX_PATH, path);
   if (len == 0)
   {
      free(path);
      return NULL;
   }
   return realloc(path, len + 1 + spaceToLeave);
}

CCE_API char* cceGetDirectory (char *path, size_t bufferSize)
{
   DWORD attributes = 0;
   DWORD error = 0;
   
   size_t length = strnlen(path, bufferSize);
   if ((*(path + length - 1) == '\\') || (*(path + length - 1) == '/'))
       --length;
   uint8_t symbolsRemaining = bufferSize - (length + 1u);
   for (uint8_t symbolsQuantity = 0u; symbolsQuantity <= symbolsRemaining; ++symbolsQuantity)
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

CCE_API char* cceGetAppDataPath (const char *folderName, size_t spaceToLeave)
{
   char *appDataPath;
   const char *path = getenv("APPDATA");
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

CCE_API uint8_t cceIsDirectory (char *path)
{
   DWORD attributes = GetFileAttributesA(path);
   GetLastError();
   return attributes != INVALID_FILE_ATTRIBUTES && (attributes & FILE_ATTRIBUTE_DIRECTORY);
}

CCE_API char* cceGetTemporaryDirectory (size_t spaceToLeave)
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
      tmpPath = realloc(tmpPath, tmpPathLength + CCE_TMPDIR_NAME_TEMPLATE_SIZE + 2u);
      memcpy(tmpPath + tmpPathLength, CCE_TMPDIR_NAME_TEMPLATE, CCE_TMPDIR_NAME_TEMPLATE_SIZE);
      tmpPathLength += CCE_TMPDIR_NAME_TEMPLATE_SIZE;
      
      *(tmpPath + tmpPathLength) = '\\';
      ++tmpPathLength;
      *(tmpPath + tmpPathLength) = '\0';
      uint32_t ID;
      cceGetRandomSeed(&ID, 3);
      cceConvertIntToBase64String(ID, tmpPath + (tmpPathLength - 6u - 1u - 2u), 6u);
      CreateDirectoryA(tmpPath, NULL);
   }
   char *buffer = malloc((tmpPathLength + spaceToLeave + 1u) * sizeof(char));
   memcpy(buffer, tmpPath, tmpPathLength + 1u);
   return buffer;
}

CCE_API void cceDeleteDirectory (const char *aPath)
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

#endif // WINDOWS_SYSTEM
