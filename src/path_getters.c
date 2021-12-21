#if defined(unix) || defined(__unix__) || defined(__unix) || \
   (defined(__APPLE__) && defined(__MACH__)) || \
    defined(linux) || defined(__linux__) || defined(__linux) || \
    defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
    defined(__bsdi__) || defined(__DragonFly__) || defined(__FreeBSD_kernel__) || \
    defined(__GNU__) || defined(__gnu_hurd__) || \
    defined(sun) || defined(__sun) || defined (sinux) || defined(__minix)

#define POSIX_SYSTEM
#define _POSIX_C_SOURCE 200809L
#define _XOPEN_SOURCE 500L

#elif defined(_WIN32) || defined(__WIN32__) || defined(_WIN64) || defined(__TOS_WIN__) || defined(__WINDOWS__) || \
      defined(__MINGW32__) || defined(__MINGW64__) || defined(__CYGWIN__)
      
#define WINDOWS_SYSTEM

#else

#error "Target platform is not supported"

#endif

#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include "log.h"
#include "path_getters.h"

#ifdef __OPTIMIZE_SIZE__
typedef uint32_t cce_uint;
#else
typedef uint_fast32_t cce_uint;
#endif // __OPTIMIZE_SIZE__

#define CCE_TMPDIR_NAME_TEMPLATE "CoffeeChain-tmpdir-XXXXXX"
#define CCE_TMPDIR_NAME_TEMPLATE_SIZE 25u

static char *tmpPath = NULL;
static size_t tmpPathLength;

CCE_OPTIONS char* createNewPathFromOldPath (const char *const oldPath, const char *const appendPath, size_t freeSpaceToLeave)
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

CCE_OPTIONS char* appendPath (char *const buffer, size_t bufferSize, const char *const append)
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
   memcpy(buffer + oldPathLength, appendPath, appendPathLength);
   *(buffer + oldPathLength + appendPathLength) = '\0';
   return buffer;
}

CCE_OPTIONS char* convertIntToBase64String (size_t number, char *restrict buffer, uint8_t symbolsQuantity)
{
   static const char
   dictionary[64] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 
                     'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z',
                     'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z',
                     '-', '_'};
   for (uint8_t i = 0u; i < symbolsQuantity; ++i)
   {
      *(buffer + symbolsQuantity - i - 1u) = *(dictionary + ((number >> (i * 6)) & 63));
   }
   return buffer;
}

#if defined(POSIX_SYSTEM)

#include <errno.h>
#include <pwd.h>
#include <unistd.h>
#include <ftw.h>

CCE_OPTIONS char* getCurrentPath (char *pathBuffer, size_t pathBufferLength)
{
   return getcwd(pathBuffer, pathBufferLength);
}

uint8_t checkDirectoryExistance (const char *restrict path)
{
   struct stat st;
   int st_state = stat(path, &st);
   if (st_state)
   {
      switch (errno)
      {
         case ENOENT:
         {
            errno = 0u;
            return 1u;
         }
         case EFAULT:
         case ELOOP:
         case ENOMEM:
         {
            return 3u;
         }
         default:
         {
            errno = 0u;
            return 2u;
         }
      }
   }
   return !S_ISDIR(st.st_mode) * 2u;
}

#if (defined(__APPLE__) && defined(__MACH__))
#define DEFAULT_APPDATA_FOLDER "/Library/Application Support/"
#define APPDATA_APPEND_SIZE 29u // strlen("/Library/Application Support/") == 29
#else 
#define DEFAULT_APPDATA_FOLDER "/.local/share/"
#define APPDATA_APPEND_SIZE 14u // strlen("/.local/share/") == 14
#endif

CCE_OPTIONS char* getAppDataPath (const char *restrict folderName, size_t spaceToLeave)
{
   struct stat st;
   char *restrict appDataPath;
   const char *restrict path = getenv("XDG_DATA_HOME");
   size_t pathLength, folderNameLength = strlen(folderName);
   if (path && checkDirectoryExistance(path))
   {
      pathLength = strlen(path) + 1u; // '/'
      appDataPath = malloc((pathLength + folderNameLength + spaceToLeave + 1u) * sizeof(char));
      memcpy(appDataPath, path, pathLength - 1u);
      if (*(appDataPath + pathLength - 2u) == '/')
      {
         (*(appDataPath + pathLength - 1u)) = '/';
      }
      else
      {
         --pathLength;
         appDataPath = realloc(appDataPath, (pathLength + folderNameLength + spaceToLeave + 1u) * sizeof(char));
      }
   }
   else
   {
      path = getenv("HOME");
      if (!path || !checkDirectoryExistance(path))
      {
         struct passwd *pw = getpwuid(getuid());
         path = pw->pw_dir;
      }
      pathLength = strlen(path);
      appDataPath = malloc((pathLength + folderNameLength + APPDATA_APPEND_SIZE + spaceToLeave + 1u) * sizeof(char)); 
      memcpy(appDataPath, path, pathLength);
      memcpy(appDataPath + pathLength, DEFAULT_APPDATA_FOLDER, APPDATA_APPEND_SIZE + 1u);
      stat(appDataPath, &st);
      if(!(S_ISDIR(st.st_mode)))
      {
         appDataPath = realloc(appDataPath, (pathLength + 2u + 1u + spaceToLeave) * sizeof(char)); // strlen("/.") == 2
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
   switch (checkDirectoryExistance(appDataPath))
   {
      case 1u:
      {
         mkdir(path, S_IRWXU | S_IRWXG);
         break;
      }
      case 2u:
      {
         criticalErrorPrint("DIRECTORY::FAILED_TO_CREATE:\n%s - Cannot create directory when there is file with same name.", path);
         break;
      }
   }
   return appDataPath;
}

CCE_OPTIONS char* getTemporaryDirectory (size_t spaceToLeave)
{
   if (!tmpPath)
   {
      char *path = getenv("TMPDIR");
      if (!path || *path == '\0' || !checkDirectoryExistance(path))
      {
         path = "/tmp";
      }
      tmpPathLength = strlen(path);
      size_t isMissingSlash = *(path + tmpPathLength - 1u) != '/';
      tmpPath = malloc((tmpPathLength + CCE_TMPDIR_NAME_TEMPLATE_SIZE + 2u + isMissingSlash) * sizeof(char));
      memcpy(tmpPath, path, tmpPathLength);
      *(tmpPath + tmpPathLength) = '/';
      memcpy(tmpPath + tmpPathLength + isMissingSlash, CCE_TMPDIR_NAME_TEMPLATE, CCE_TMPDIR_NAME_TEMPLATE_SIZE + 1u);
      tmpPathLength += CCE_TMPDIR_NAME_TEMPLATE_SIZE + isMissingSlash + 1u;
      path = mkdtemp(tmpPath);
      
      if (!path)
      {
         free(tmpPath);
         criticalErrorPrint("DIRECTORY::TEMPORARY::FAILED_TO_CREATE:\nmkdtemp() returned NULL", NULL);
      }
      tmpPath = path;
      *(tmpPath + tmpPathLength - 1u) = '/';
      *(tmpPath + tmpPathLength) = '\0';
   }
   char *buffer = malloc((tmpPathLength + spaceToLeave + 1u) * sizeof(char));
   return memcpy(buffer, tmpPath, tmpPathLength + 1u);
}

static int removeCallback (const char *path, const struct stat *st, int type, struct FTW *info)
{
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

CCE_OPTIONS void terminateTemporaryDirectory (void)
{
   if (!tmpPath)
   {
      return;
   }
   int error = nftw(tmpPath, removeCallback, FOPEN_MAX, FTW_DEPTH | FTW_MOUNT | FTW_PHYS);
   free(tmpPath);
   tmpPath = NULL;
   if (error)
   {
      criticalErrorPrint("DIRECTORY::TEMPORARY::FAILED_TO_REMOVE:\nnftw indicates an error", NULL);
   }
}

#elif defined(WINDOWS_SYSTEM)

#include <windows.h>
#include <shlobj.h>
#include <shlobj_core.h>
#include <shellapi.h>
#include <time.h>

CCE_OPTIONS char* getCurrentPath (char *pathBuffer, size_t pathBufferLength)
{
   return GetCurrentDirectory(pathBufferLength, pathBuffer);
}

uint8_t checkDirectoryExistance (const char *restrict path)
{
   DWORD dwAttrib = GetFileAttributes(path);
   if (dwAttrib != INVALID_FILE_ATTRIBUTES)
   {
      return (GetLastError() != ERROR_PATH_NOT_FOUND) + 1u;
   }
   return ((!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY)) * 2u);
}

CCE_OPTIONS char* getAppDataPath (const char *restrict folderName, size_t spaceToLeave)
{
   char *restrict appDataPath;
   const char *restrict path = getenv("APPDATA");
   size_t pathLength, folderNameLength = strlen(folderName);
   if (path)
   {
      pathLength = strlen(path);
      appDataPath = malloc((pathLength + folderNameLength + spaceToLeave + 1u) * sizeof(char));
      memcpy(appDataPath, path, pathLength);
      (*(appDataPath + pathLength)) = '\\';
   }
   else
   {
      appDataPath = malloc(MAX_PATH * sizeof(char));
      if (!SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA | CSIDL_FLAG_CREATE, NULL, 0, appDataPath))
      {
         criticalErrorPrint("DIRECTORY::APPDATA::FAILED_TO_RETRIEVE:\nFunction SHGetFolderPath exited with an error", NULL);
      }
      pathLength = strlen(appDataPath);
      appDataPath = realloc(appDataPath, (pathLength + folderNameLength + spaceToLeave + 1u) * sizeof(char));
      (*(appDataPath + pathLength)) = '\\';
   }
   memcpy(appDataPath + pathLength + 1u, folderName, folderNameLength + 1u);
   switch (checkDirectoryExistance(appDataPath))
   {
      case 1u:
      {
         CreateDirectory(appDataPath, NULL);
         break;
      }
      case 2u:
      {
         criticalErrorPrint("DIRECTORY::FAILED_TO_CHECK\n%s - Cannot check directory existence (is there a file with same name?).", appDataPath);
         break;
      }
   }
   return appDataPath;
}

CCE_OPTIONS char* getTemporaryDirectory (size_t spaceToLeave);
{
   if (!tmpPath)
   {
      tmpPath = malloc(MAX_PATH * sizeof(char));
      tmpPathLength = GetTempPath(tmpPath, tmpPath, MAX_PATH);
      if (!tmpPathLength)
      {
         free(tmpPath);
         criticalErrorPrint("DIRECTORY::TEMPORARY::FAILED_TO_GET_TEMPORARY_PATH:\GetTempPath() returned length 0", NULL);
      }
      char *path = malloc(MAX_PATH * sizeof(char));
      tmpPathLength = GetLongPathName(tmpPath, path, MAX_PATH);
      if (!tmpPathLength)
      {
         free(tmpPath);
         criticalErrorPrint("DIRECTORY::TEMPORARY::FAILED_TO_GET_LONG_PATH:\GetLongPathName() returned length 0", NULL);
      }
      
      if (tmpPathLength > MAX_PATH)
      {
         free(path);
         tmpPath = realloc(tmpPath, tmpPathLength + CCE_TMPDIR_NAME_TEMPLATE_SIZE + 2u);
         GetLongPathName(tmpPath, tmpPath, tmpPathLength + 1u);
      }
      else
      {
         free(tmpPath);
         tmpPath = realloc(path, tmpPathLength + CCE_TMPDIR_NAME_TEMPLATE_SIZE + 2u);
      }
      memcpy(tmpPath + tmpPathLength, CCE_TMPDIR_NAME_TEMPLATE, CCE_TMPDIR_NAME_TEMPLATE_SIZE);
      tmpPathLength += CCE_TMPDIR_NAME_TEMPLATE_SIZE;
      
      *(tmpPath + tmpPathLength) = '\\';
      *(tmpPath + tmpPathLength + 1u) = '\0';
      convertIntToBase64String(time(0), tmpPath + (tmpPathLength - 6u - 1u), 6u);
   }
   char *buffer = malloc((tmpPathLength + spaceToLeave + 1u) * sizeof(char));
   return memcpy(buffer, tmpPath, tmpPathLength + 1u);
}

CCE_OPTIONS void terminateTemporaryDirectory (void)
{
   if (!tmpPath)
   {
      return;
   }
   WIN32_FIND_DATA fileData;
   size_t bufferSize = strlen(tmpPath);
   size_t pathLength;
   size_t fileNameLength;
   HANDLE file;
   char *path
   if (bufferSize > MAX_PATH)
   {
      path = createNewPathFromOldPath(tmpPath, "*", 22u);
      bufferSize += 23u; // +Null-terminator
   }
   else
   {
      path = createNewPathFromOldPath(tmpPath, "*", MAX_PATH - bufferSize);
      bufferSize = MAX_PATH; // +Null-terminator
   }
   pathLength = strlen(path) - 1u;
   for (;;)
   {
      file = FindFirstFile(path, &fileData);
      for (;;)
      {
         fileNameLength = strlen(fileData.cFileName);
         if ((*(fileData.cFileName) == '.') && (fileNameLength < 3u))
            continue;
         
         memcpy(path + pathLength, fileData.cFileName, fileNameLength + 1u);
         
         if (fileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
         {
            pathLength += fileNameLength + 1u;
            *(path + pathLength - 1u) = '\\';
            *(path + pathLength)      = '*';
            *(path + pathLength + 1u) = '\0';
            break;
         }
         else
         {
            remove(path);
         }
         
         if (!FindNextFile(file, &fileData))
         {
            RemoveDirectory(path);
            if (!(strcmp(path, tmpPath)))
            {
               free(path);
               free(tmpPath);
               tmpPath = NULL
               return;
            }
            while (*(path + pathLength - 1u) != '\\')
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