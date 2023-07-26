/*
   Conservative Creator's Engine - open source engine for making games.
   Copyright © 2020-2023 Andrey Gaivoronskiy

   This file is part of Conservative Creator's Engine.

   Copying and distribution of this file, with or without modification,
   are permitted in any medium without royalty provided the copyright
   notice and this notice are preserved.  This file is offered as-is,
   without any warranty.
*/

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <cce/plugins/actions.h>
#include <cce/engine_common.h>
#include <cce/plugins/map2D/map2D.h>
#include <cce/os_interaction.h>

uint8_t success = 0;

void axisCallback (int8_t horizontal, int8_t vertical)
{
   printf("Left stick position changed to %i %i\n", horizontal, vertical);
}

void buttonCallback (uint16_t buttonsSet, uint16_t diff)
{
   printf("Set buttons 0x%x (diff 0x%x)\n", buttonsSet, diff);
}

struct alterMapSt
{
   uint32_t actionUID;
   uint8_t counter;
};

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
   //struct cce_renderinginfo *info = cceGetRenderingInfo(map);
   {
      CCEA_RUNACTIONS_CREATE_STATIC1(action, struct cceaDelayActionsRepeated, ((struct cceaDelayActionsRepeated){cceaBasicActionUIDs[CCEA_DELAY_ACTIONS_REPEATED], 0, 800, 9}),
                                             uint32_t, cceNameToUID("flip"));
      cceaRunAction((struct cceaAction*)action, 1, map);
   }
   {
      CCEA_RUNACTIONS_CREATE_STATIC1(action, struct cceaDelayActionsPeriodic, ((struct cceaDelayActionsPeriodic){cceaBasicActionUIDs[CCEA_DELAY_ACTIONS_PERIODIC], 0, 330}),
                                             struct alterMapSt, ((struct alterMapSt){cceNameToUID("altMap"), 0}));
      cceaRunAction((struct cceaAction*)action, 1, map);
   }
   {
      CCEA_RUNACTIONS_CREATE_STATIC2(action, struct cceaDelayActions, ((struct cceaDelayActions){cceaBasicActionUIDs[CCEA_DELAY_ACTIONS], 0, 6000}),
                                             struct cceaInvokeEvent,  ((struct cceaInvokeEvent){cceaBasicActionUIDs[CCEA_INVOKE_EVENT], cceaBasicEventsUIDs[CCEA_EVENT_LOAD]}),
                                             struct cceaTerminateEngine, ((struct cceaTerminateEngine){cceaBasicActionUIDs[CCEA_TERMINATE_ENGINE]}));
      cceaRunAction((struct cceaAction*)action, 1, map);
   }
   {
      CCEA_RUNACTIONS_CREATE_STATIC1(action, struct cceaDelayActionsPeriodic, ((struct cceaDelayActionsPeriodic){cceaBasicActionUIDs[CCEA_DELAY_ACTIONS_PERIODIC], 0, 3}),
                                             uint32_t, cceNameToUID("rotate"));
      cceaRunAction((struct cceaAction*)action, 1, map);
   }
   {
      CCEA_RUNACTIONS_CREATE_STATIC2(action, struct cceaAddActionsOnEvent, ((struct cceaAddActionsOnEvent){cceaBasicActionUIDs[CCEA_ADD_ACTIONS_ON_EVENT], 0,
                                                                             cceaBasicEventsUIDs[CCEA_EVENT_LOAD], cceNameToUID("test2")}),
                                             uint32_t, cceNameToUID("setbit"),
                                             struct cceaRemoveActionOnEvent, ((struct cceaRemoveActionOnEvent){cceaBasicActionUIDs[CCEA_REMOVE_ACTIONS_ON_EVENT],
                                             cceaBasicEventsUIDs[CCEA_EVENT_LOAD], cceNameToUID("test2")}));
      cceaRunAction((struct cceaAction*)action, 1, map);
   }
   return map;
}

void flip (void *data, uint32_t repeats, struct cce_buffer *state)
{
   CCE_UNUSED(data);
   if ((repeats & 1) == 0)
      return;
   struct cce_element *element = cceGetElements(0, 1, state);
   element->flags ^= CCE_ELEMENT_FLIP_VERTICALLY;
   cceSetElementsUpdated(cceGetRenderingInfo(state));
}

void rotate (void *data, uint32_t repeats, struct cce_buffer *state)
{
   struct cce_element *element = cceGetElements(2, 1, state);
   element->rotation -= repeats;
   cceSetElementsUpdated(cceGetRenderingInfo(state));
}

void alterMapFrame (void *data, uint32_t repeats, struct cce_buffer *state)
{
   struct alterMapSt *map = data;
   map->counter += repeats;
   struct cce_element *element = cceGetElements(1, 3, state);
   uint8_t offsets[] = {0, 1};
   uint8_t flags[] = {0, 0, CCE_ELEMENT_FLIP_HORIZONTALLY, CCE_ELEMENT_FLIP_VERTICALLY};
   element[0].data.texturePosition.x = offsets[map->counter & 1];
   element[0].flags = flags[map->counter & 3];
   element[2].flags ^= CCE_ELEMENT_FLIP_HORIZONTALLY;
   element[2].rotation += 128;
   cceSetElementsUpdated(cceGetRenderingInfo(state));
}

void setSuccessBit (void *data, uint32_t repeats, struct cce_buffer *state)
{
   CCE_UNUSED(data);
   CCE_UNUSED(repeats);
   CCE_UNUSED(state);
   success ^= 1;
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
   cceaLoadActionsPlugin();
   cceLoadMap2Dplugin();
   if (cceInit("test2/game.ini") != 0)
   {
      fputs("Initialization failure\n", stderr);
      return -1;
   }
   cceaRegisterAction(cceNameToUID("flip"), flip, NULL, sizeof(uint32_t));
   cceaRegisterAction(cceNameToUID("altMap"), alterMapFrame, NULL, sizeof(struct alterMapSt));
   cceaRegisterAction(cceNameToUID("rotate"), rotate, NULL, sizeof(uint32_t));
   cceaRegisterAction(cceNameToUID("setbit"), setSuccessBit, NULL, sizeof(uint32_t));
   
   struct cce_buffer *map = createMap();
   
   char *tmp = cceGetTemporaryDirectory(9u);
   strcat(tmp, "/test.c2m");
   cceWriteMap2Ddynamic(map, tmp);
   cceFreeMap2Ddynamic(map);
   map = cceLoadMap2D(tmp);
   free(tmp);
   if (map == NULL || map->sectionsQuantity == 0)
   {
      fputs("Loading failure\n", stderr);
      return -1;
   }
   cceSetRenderingLayerMap2D(0, 0, map);
   cceSetAxisChangeCallback(axisCallback, CCE_AXISPAIR_LSTICK);
   cceSetButtonCallback(buttonCallback);
   while (cceEngineShouldTerminate() == 0)
   {
      cceRenderMap2D();
      cceUpdate();
      cceaRunDelayedActions(map);
      cceScreenUpdate();
   }
   cceFreeMap2D(map);
   cceTerminate();
   return (success == 1) - 1;
}
