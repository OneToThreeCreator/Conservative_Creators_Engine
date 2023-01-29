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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cce/map2D/actions.h>
#include <cce/engine_common.h>
#include <cce/map2D/map2D.h>
#include <cce/os_interaction.h>

uint32_t frames = 0;

void axisCallback (int8_t horizontal, int8_t vertical)
{
   printf("Left stick position changed to %i %i\n", horizontal, vertical);
}

void buttonCallback (uint16_t buttonsSet, uint16_t diff)
{
   printf("Set buttons 0x%x (diff 0x%x)\n", buttonsSet, diff);
}

struct cce_buffer* createMap (void)
{
   struct cce_buffer *map = cceCreateMap2Ddynamic();
   struct cce_usedtexinfo *textures = cceGetResource(CCE_RESOURCE_TEXTURE, map);
   CCE_ALLOC_ARRAY(textures->texturesMapDependsOn, 1);
   textures->texturesMapDependsOnQuantity = 1;
   uint16_t texture = cceLoadTexture("test.png", 1);
   textures->texturesMapDependsOn[0] = texture;
   struct cce_elementposition positions[] = 
   {
      {{0,  0}, 1, 0, 0},
      {{0,  0}, 2, 0, 0},
      {{0,  0}, 3, 0, 0},
      {{5,  0}, 4, 0, 0},
   };
   struct cce_element elements[] = 
   {
      {{-16, -16}, { 0, 0}, {11,  4}, 0, 0, texture, 0,   0},
      {{-16,   0}, { 0, 4}, { 5,  3}, 0, 0, texture, 0,   0},
      {{-16,   8}, { 0, 7}, { 8,  8}, 0, 0, texture, 0,   0},
      {{  0, -16}, {10, 3}, { 5, 13}, 0, 0, texture, 192, 0},
   };
   memcpy(cceGetElementsPosition(0, 0, 4, map), positions, 4 * sizeof(struct cce_elementposition));
   memcpy(cceGetElements(0, 4, map),            elements,  4 * sizeof(struct cce_element));
   struct cce_renderinginfo *info = cceGetRenderingInfo(map);
   return map;
}

void alterMapFrame (struct cce_buffer *map)
{
   struct cce_element *element = cceGetElements(0, 3, map);
   uint8_t updateFrame = (frames & 31) == 0;
   uint8_t offsets[] = {0, 1};
   uint8_t flags[] = {0, 0, CCE_ELEMENT_FLIP_HORIZONTALLY, CCE_ELEMENT_FLIP_VERTICALLY};
   element[1].data.texturePosition.x = offsets[(frames >> 5) & 1];
   element[1].flags = flags[(frames >> 5) & 3];
   element[2].rotation -= 1;
   element[3].flags ^= CCE_ELEMENT_FLIP_HORIZONTALLY & -(updateFrame);
   element[3].rotation += 128 & -(updateFrame);
   if (frames >= 192)
      cceSetEngineShouldTerminate(1);
   cceSetElementsUpdated(cceGetRenderingInfo(map));
}

int main (int argc, char **argv)
{
   char *path;
   size_t pathLength;
   if (argc >= 2)
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
   struct cce_buffer *map = createMap();
   
   char *tmp = cceGetTemporaryDirectory(8u);
   strcat(tmp, "test.c2m");
   cceWriteMap2Ddynamic(map, tmp);
   cceFreeMap2Ddynamic(map);
   map = cceLoadMap2D(tmp);
   free(tmp);
   cceSetRenderingLayerMap2D(0, 0, map);
   cceSetAxisChangeCallback(axisCallback, CCE_AXISPAIR_LSTICK);
   cceSetButtonCallback(buttonCallback);
   printf("Initialization complete\n");
   while (cceEngineShouldTerminate() == 0)
   {
      cceRenderMap2D();
      cceUpdateEngineMap2D();
      ++frames;
      alterMapFrame(map);
   }
   cceFreeMap2D(map);
   cceTerminateEngine();
   return 0;
}
