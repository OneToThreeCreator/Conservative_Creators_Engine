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

#include <cce/plugins/actions.h>
#include <cce/engine_common.h>
#include <cce/plugins/map2D/map2D.h>
#include <cce/os_interaction.h>

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
      {{-16, -16}, { 0, 0}, {11,  4}, texture, 0,   0},
      {{-16,   0}, { 0, 4}, { 5,  3}, texture, 0,   0},
      {{-16,   8}, { 0, 7}, { 8,  8}, texture, 0,   0},
      {{  0, -16}, {10, 3}, { 5, 13}, texture, 192, 0},
   };
   memcpy(cceGetElementsPosition(0, 0, 4, map), positions, 4 * sizeof(struct cce_elementposition));
   memcpy(cceGetElements(0, 4, map),            elements,  4 * sizeof(struct cce_element));
   struct cce_renderinginfo *info = cceGetRenderingInfo(map);
   return map;
}

struct cce_buffer *g_map;

struct alterMapSt
{
   uint32_t actionID;
   uint8_t counter;
};

void alterMapFrame (const void *data, uint16_t repeats)
{
   struct alterMapSt *map = (void*) data;
   map->counter += repeats;
   struct cce_element *element = cceGetElements(1, 3, g_map);
   uint8_t offsets[] = {0, 1};
   uint8_t flags[] = {0, 0, CCE_ELEMENT_FLIP_HORIZONTALLY, CCE_ELEMENT_FLIP_VERTICALLY};
   element[0].data.texturePosition.x = offsets[map->counter & 1];
   element[0].flags = flags[map->counter & 3];
   element[2].flags ^= CCE_ELEMENT_FLIP_HORIZONTALLY;
   element[2].rotation += 128;
   cceSetElementsUpdated(cceGetRenderingInfo(g_map));
}

void delayedAction (const void *data, uint16_t repeats)
{
   CCE_UNUSED(data);
   if ((repeats & 1) == 0)
      return;
   struct cce_element *element = cceGetElements(0, 1, g_map);
   element->flags ^= CCE_ELEMENT_FLIP_VERTICALLY;
   cceSetElementsUpdated(cceGetRenderingInfo(g_map));
}

void rotate (const void *data, uint16_t repeats)
{
   struct cce_element *element = cceGetElements(2, 1, g_map);
   element->rotation -= repeats;
   cceSetElementsUpdated(cceGetRenderingInfo(g_map));
}

int main (int argc, char **argv)
{
   if (argc >= 2)
   {
      if (argc > 2 || (argc == 2 && (!strcmp(argv[1], "-h") || !strcmp(argv[1], "--help"))))
      {
         printf("Usage: %s [PATH_TO_ENGINE_RESOURCES]\nWhen PATH_TO_ENGINE_RESOURCES is not provided, current directory is assumed.", argv[0]);
         return -(argc > 2);
      }
      cceSetCurrentPath(argv[1]);
   }
   cceLoadActionsPlugin();
   cceLoadMap2Dplugin();
   if (cceInit("test2/game.ini") != 0)
   {
      printf("Initialization failure\n");
      return -1;
   }
   struct cce_buffer *map = createMap();
   
   char *tmp = cceGetTemporaryDirectory(9u);
   strcat(tmp, "/test.c2m");
   cceWriteMap2Ddynamic(map, tmp);
   cceFreeMap2Ddynamic(map);
   map = cceLoadMap2D(tmp);
   free(tmp);
   cceSetRenderingLayerMap2D(0, 0, map);
   cceSetAxisChangeCallback(axisCallback, CCE_AXISPAIR_LSTICK);
   cceSetButtonCallback(buttonCallback);
   cceRegisterAction(16, delayedAction, NULL);
   cceRegisterAction(17, alterMapFrame, NULL);
   cceRegisterAction(18, rotate, NULL);
   g_map = map;
   uint32_t actionID = 16;
   struct alterMapSt st = {17, 0};
   struct cceSetEngineShouldTerminate st2 = {CCE_ACTION_SETENGINESHOULDTERMINATE, 1, CCE_ACTION_SET};
   cceDelayAction(6,  800000, sizeof(uint32_t),                           &actionID, CCE_DEFAULT);
   cceDelayAction(1,  330000, sizeof(struct alterMapSt),                  &st,       CCE_DELAYACTION_NEVER_END);
   cceDelayAction(1, 3000000, sizeof(struct cceSetEngineShouldTerminate), &st2,      CCE_DEFAULT);
   actionID = 18;
   cceDelayAction(1,    3000, sizeof(uint32_t),                           &actionID, CCE_DELAYACTION_NEVER_END);
   while (cceEngineShouldTerminate() == 0)
   {
      cceRenderMap2D();
      cceUpdate();
      cceScreenUpdate();
   }
   cceFreeMap2D(map);
   cceTerminate();
   return 0;
}
