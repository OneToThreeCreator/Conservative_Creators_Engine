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

void axisCallback (int8_t horizontal, int8_t vertical)
{
   printf("Left stick position changed to %i %i\n", horizontal, vertical);
}

void buttonCallback (uint16_t buttonsSet, uint16_t diff)
{
   printf("Set buttons 0x%x (diff 0x%x)\n", buttonsSet, diff);
}

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
      cceSetCurrentPath(argv[0]);
   }
   if (cceInitEngine2D("test2/game.ini") != 0)
   {
      free(path);
      printf("Initialization failure\n");
      return -1;
   }
   struct cce_buffer *map = cceLoadMap2Ddynamic("/NULL.c2m");
   cceRenderingLayerSetMap2D(0, 0, map);
   cceSetAxisChangeCallback(axisCallback, CCE_AXISPAIR_LSTICK);
   cceSetButtonCallback(buttonCallback);
   printf("Initialization complete\n");
   while (cceEngineShouldTerminate() == 0)
   {
      cceRenderMap2D();
      cceUpdateEngineMap2D();
   }
   return 0;
}
