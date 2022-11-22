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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cce/map2D/actions.h>
#include <cce/engine_common.h>
#include <cce/map2D/map2D.h>
#include <cce/os_interaction.h>

int main (int argc, char **argv)
{
   char *path;
   size_t pathLength;
   if (argc < 2)
   {
      path = cceGetCurrentPath(15);
      pathLength = strlen(path);
   }
   else
   {
      if (argc > 2 || (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))))
      {
         printf("Usage: %s [PATH_TO_ENGINE_RESOURCES]\nWhen PATH_TO_ENGINE_RESOURCES is not provided, current directory is assumed.", argv[0]);
         exit(argc > 2);
      }
      if (argv[1][0] == '/')
      {
         pathLength = strlen(argv[1]);
         path = malloc(pathLength + 16);
         memcpy(path, argv[1], pathLength + 1);
      }
      else
      {
         char *appendString = argv[1];
         appendString += (*appendString == '.');
         size_t appendLength = strlen(appendString);
         path = cceGetCurrentPath(appendLength + 16);
         pathLength = strlen(path);
         if (*appendString == '.')
         {
            appendString += 2;
            appendLength -= 2;
            pathLength -= 2 - (pathLength < 2);
            for (char *iterator = (path + pathLength), *end = path; iterator >= end && *iterator != '/' && *iterator != '\\'; --iterator, --pathLength) {}
         }
         memcpy(path + pathLength + 1, appendString, appendLength + 1);
         pathLength += appendLength;
      }
   }
   if (cceInitEngine2D(48, 48, "CCE TEST", path, 1, 0, CCE_DEFAULT) != 0)
   {
      free(path);
      printf("Initialization failure\n");
      return -1;
   }
   struct cce_buffer *map = cceLoadMap2Ddynamic("/NULL.c2m");
   cceWriteMap2Ddynamic(map, "/tmp/test.c2m");
   cceFreeMap2D(map);
   map = cceLoadMap2D("/tmp/test.c2m");
   remove("/tmp/test.c2m");
   printf("cceLoadMap2D returned %s\n", (map == NULL) ? "NULL" : "Valid map");
   cceLayerSetMap2D(0, 0, map);
   printf("Initialization complete\n");
   for (;;)
   {
      cceRenderMap2D();
      cceUpdateEngineMap2D();
   }
   return 0;
}
