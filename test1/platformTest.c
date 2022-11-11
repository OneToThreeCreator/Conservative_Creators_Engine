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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cce/os_interaction.h>

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
