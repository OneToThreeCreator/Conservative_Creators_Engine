/*
   Conservative Creator's Engine - open source engine for making games.
   Copyright © 2020-2023 Andrey Gaivoronskiy

   This file is part of Conservative Creator's Engine.

   Copying and distribution of this file, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  This file is offered as-is,
   without any warranty.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cce/os_interaction.h>
#include <cce/utils.h>

static uint8_t createTestFile (char *path)
{
   FILE *file = fopen(path, "w");
   if (file == NULL)
   {
      return 0u;
   }
   fprintf(file, "Some string for testing purposes\n");
   fclose(file);
   return 1u;
}

uint8_t tmpDirTest (void)
{
   char *path = cceGetTemporaryDirectory(8u + 1u);
   size_t pathLength = strlen(path);
   cceAppendPath(path, pathLength + 8u + 1u + 1u, "test.txt");
   if (!createTestFile(path))
   {
       printf("TEST1::FAILED\nfile at path %s cannot be created\n", path);
       return 0;
   }
   cceTerminateTemporaryDirectory();
   return 1;
   /*
   size_t i = 0u;
   char **tmpPaths = malloc(iterationsQuantity * sizeof(char*));
   while (i < iterationsQuantity)
   {
      tmpPaths[i] = cceGetTemporaryDirectory(8u + 1u);
      size_t pathLength = strlen(tmpPaths[i]);
      char *path = cceAppendPath(tmpPaths[i], pathLength + 8u + 1u + 1u, "test.txt");
      if (!createTestFile(path))
      {
         printf("TEST1::FAILED\nfile at path %s cannot be created\n", path);
         break;
      }
      tmpPaths[i][pathLength] = '\0';
      cceTerminateTemporaryDirectory();
      if (!createTestFile(tmpPaths[i]))
      {
         printf("TEST1::FAILED\nfile at path %s cannot be created\n", path);
         break;
      }
      ++i;
   }
   for (size_t j = 0u; j <= i; ++j)
   {
      remove(tmpPaths[j]);
   }
   return (i == iterationsQuantity);
   */
}

uint8_t appDataDirTest (void)
{
   char *path = cceGetAppDataPath("TEST", 8u + 1u);
   size_t pathLength = strlen(path);
   if (!createTestFile(cceAppendPath(path, pathLength + 8u + 1u + 1u, "test.txt")))
   {
      printf("TEST2::FAILED:\nfile at path %s cannot be created\n", path);
      cceDeleteDirectory(path);
      return 0u;
   }
   path[pathLength] = '\0';
   cceDeleteDirectory(path);
   if (!createTestFile(path))
   {
      printf("TEST2::FAILED:\nfile at path %s cannot be created\n", path);
      cceDeleteDirectory(path);
      return 0u;
   }
   path[pathLength + 1u] = '\0';
   for (size_t i = 0u, end = 64u; i < end; ++i)
   {
      cceConvertIntToBase64String(i, path + pathLength, 1u);
      if (!createTestFile(path))
      {
         printf("TEST2::FAILED:\nfile at path %s cannot be created\n", path);
         do
         {
            --i;
            cceConvertIntToBase64String(i, path + pathLength, 1u);
            remove(path);
         }
         while (i != 0);
         return 0u;
      }
   }
   char *path2 = cceGetAppDataPath("TEST", 8u + 1u);
   for (size_t i = 0u, end = 64u; i < end; ++i)
   {
      cceConvertIntToBase64String(i, path + pathLength, 1u);
      remove(path);
   }
   path[pathLength] = '\0';
   remove(path);
   size_t path2Length = strlen(path2);
   uint8_t result = createTestFile(cceAppendPath(path2, path2Length + 8u + 1u + 1u, "test.txt"));
   if (!result)
   {
      printf("TEST2::FAILED:\nfile at path %s cannot be created\n", path2);
   }
   path2[path2Length] = '\0';
   cceDeleteDirectory(path2);
   return result;
}
