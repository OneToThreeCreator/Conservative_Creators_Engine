/*
    CoffeeChain - open source engine for making games.
    Copyright (C) 2020-2021 Andrey Givoronsky

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

#include <coffeechain/map2D/base_actions.h>
#include <coffeechain/engine_common.h>
#include <coffeechain/map2D/map2D.h>
#include <coffeechain/path_getters.h>

void initPlayer (uint32_t globalBoolsQuantity)
{
   struct Map2DElementDev player = {-1, -1, 1, 2, {0.5625f, 0.0f, 1.0f, 1.0f, 1}, 0, 0, 0, 0, 0, 0, 0};
   cceCreateMap2DElementDynamicMap2D(&player, 1);

   uint_fast16_t *aandb = cceParseStringToLogicOperations("a & b", NULL);
   uint16_t logicIDs[5];
   uint16_t timerID = cceCreateTimerDynamicMap2D(0.03f);
   uint16_t moveElements[2] = {globalBoolsQuantity - 12, timerID};
   cce_enum types[2] = {CCE_GLOBAL_BOOL_LOGIC_ELEMENT, CCE_TIMER_LOGIC_ELEMENT};
   for (uint16_t *iterator = logicIDs, *end = logicIDs + 4; iterator < end; ++iterator)
   {
      *iterator = cceCreateLogicDynamicMap2D();
      cceUpdateLogicElementsByTruthTableDynamicMap2D(*iterator, 2, moveElements, types, aandb);
      ++(moveElements[0]);
   }
   {
      uint16_t updateTimerElements[2] = {globalBoolsQuantity, timerID};
      logicIDs[4] = cceCreateLogicDynamicMap2D();
      cceUpdateLogicElementsByTruthTableDynamicMap2D(logicIDs[4], 2, updateTimerElements, types, aandb);
   }
   free(aandb);
   uint32_t moveAndSetBoolAction[2] = {0, 5};
   struct setBoolActionStruct setBoolSt = {globalBoolsQuantity, CCE_ENABLE_BOOL};
   struct moveActionStruct moveSt = {0, -1, 0, CCE_SHIFT, CCE_DYNAMIC_MAP2D};
   const void *moveVerticallyActionArgs[2] = {&moveSt, &setBoolSt};
   uint32_t moveVerticallyActionArgSizes[2] = {sizeof(struct moveActionStruct), sizeof(struct setBoolActionStruct)};
   cceUpdateLogicActionsDynamicMap2D(logicIDs[0], 2, moveAndSetBoolAction, moveVerticallyActionArgs, moveVerticallyActionArgSizes);
   moveSt.y = 1;
   cceUpdateLogicActionsDynamicMap2D(logicIDs[1], 2, moveAndSetBoolAction, moveVerticallyActionArgs, moveVerticallyActionArgSizes);
   moveSt.x = 1;
   moveSt.y = 0;
   setBoolSt.action = CCE_DISABLE_BOOL;
   struct startTimerActionStruct startTimerSt = {timerID, CCE_DYNAMIC_MAP2D};
   const void *moveHorizontallyActionArgs[3] = {&moveSt, &setBoolSt, &startTimerSt};
   uint32_t moveHorizontallyActionArgSizes[3] = {sizeof(struct moveActionStruct), sizeof(struct setBoolActionStruct), sizeof(struct startTimerActionStruct)};

   uint32_t moveAndSetBoolAndTimerAction[3] = {0, 5, 7};
   cceUpdateLogicActionsDynamicMap2D(logicIDs[2], 3, moveAndSetBoolAndTimerAction, moveHorizontallyActionArgs, moveHorizontallyActionArgSizes);
   moveSt.x = -1;
   cceUpdateLogicActionsDynamicMap2D(logicIDs[3], 3, moveAndSetBoolAndTimerAction, moveHorizontallyActionArgs, moveHorizontallyActionArgSizes);
   uint32_t setBoolAndTimerAction[2] = {5, 7};
   const void *timerAndBool[2] = {&setBoolSt, &startTimerSt};
   uint32_t timerAndBoolSizes[2] = {sizeof(struct setBoolActionStruct), sizeof(struct startTimerActionStruct)};
   cceUpdateLogicActionsDynamicMap2D(logicIDs[4], 2, setBoolAndTimerAction, timerAndBool, timerAndBoolSizes);
   return;
}

void createAdditionalDynamicElement (struct cce_ivec2 coords, uint32_t globalBoolsQuantity)
{
   struct Map2DElementDev element = {coords.x, coords.y, 2, 2, {0.0f, 0.0f, 1.0f, 1.0f, 0}, 0, 1, 1, 0, 0, 0, 1};
   cceCreateMap2DElementDynamicMap2D(&element, 1);

   uint_fast16_t *aandb = cceParseStringToLogicOperations("a & b", NULL);
   uint16_t logicIDs[5];
   uint16_t timerID = cceCreateTimerDynamicMap2D(0.03f);
   uint16_t bools[5] = {globalBoolsQuantity - 5, globalBoolsQuantity - 7, globalBoolsQuantity - 8, globalBoolsQuantity - 6, globalBoolsQuantity + 1};
   uint16_t moveElements[2] = {0, timerID};
   cce_enum types[2] = {CCE_GLOBAL_BOOL_LOGIC_ELEMENT, CCE_TIMER_LOGIC_ELEMENT};
   for (uint16_t *biterator = bools, *iterator = logicIDs, *end = logicIDs + 5; iterator < end; ++iterator, ++biterator)
   {
      moveElements[0] = *biterator;
      *iterator = cceCreateLogicDynamicMap2D();
      cceUpdateLogicElementsByTruthTableDynamicMap2D(*iterator, 2, moveElements, types, aandb);
   }
   free(aandb);
   uint32_t moveAndSetBoolAction[2] = {0, 5};
   struct setBoolActionStruct setBoolSt = {globalBoolsQuantity + 1, CCE_ENABLE_BOOL};
   struct moveActionStruct moveSt = {0, 1, 1, CCE_SHIFT, CCE_DYNAMIC_MAP2D};
   const void *moveVerticallyActionArgs[2] = {&moveSt, &setBoolSt};
   uint32_t moveVerticallyActionArgSizes[2] = {sizeof(struct moveActionStruct), sizeof(struct setBoolActionStruct)};
   cceUpdateLogicActionsDynamicMap2D(logicIDs[0], 2, moveAndSetBoolAction, moveVerticallyActionArgs, moveVerticallyActionArgSizes);
   moveSt.y = -1;
   cceUpdateLogicActionsDynamicMap2D(logicIDs[1], 2, moveAndSetBoolAction, moveVerticallyActionArgs, moveVerticallyActionArgSizes);
   moveSt.x = -1;
   moveSt.y = 0;
   setBoolSt.action = CCE_DISABLE_BOOL;
   struct startTimerActionStruct startTimerSt = {timerID, CCE_DYNAMIC_MAP2D};
   const void *moveHorizontallyActionArgs[3] = {&moveSt, &setBoolSt, &startTimerSt};
   uint32_t moveHorizontallyActionArgSizes[3] = {sizeof(struct moveActionStruct), sizeof(struct setBoolActionStruct), sizeof(struct startTimerActionStruct)};

   uint32_t moveAndSetBoolAndTimerAction[3] = {0, 5, 7};
   cceUpdateLogicActionsDynamicMap2D(logicIDs[2], 3, moveAndSetBoolAndTimerAction, moveHorizontallyActionArgs, moveHorizontallyActionArgSizes);
   moveSt.x = 1;
   cceUpdateLogicActionsDynamicMap2D(logicIDs[3], 3, moveAndSetBoolAndTimerAction, moveHorizontallyActionArgs, moveHorizontallyActionArgSizes);
   uint32_t setBoolAndTimerAction[2] = {5, 7};
   const void *timerAndBool[2] = {&setBoolSt, &startTimerSt};
   uint32_t timerAndBoolSizes[2] = {sizeof(struct setBoolActionStruct), sizeof(struct startTimerActionStruct)};
   cceUpdateLogicActionsDynamicMap2D(logicIDs[4], 2, setBoolAndTimerAction, timerAndBool, timerAndBoolSizes);
   return;
}

int main (int argc, char **argv)
{
   char path[256];
   if (argc < 2)
   {
      cceGetCurrentPath(path, 256);
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
         strncpy(path, argv[1], 256);
      }
      else
      {
         char *appendString = argv[1];
         appendString += (appendString[0] == '.') * 2;
         cceGetCurrentPath(path, 256);
         strncat(path, appendString, 256 - strnlen(path, 256));
      }
   }
   const uint32_t globalBoolsQuantity = 32768;
   if (cceInitEngine2D(globalBoolsQuantity, 16, 16, "CoffeeChain TEST", path) != 0)
   {
      return -1;
   }
   cceAppendPath(path, 256, "test2/textures");
   cceSetTexturesPath(path);
   cceSetFlags2D(CCE_RENDER_ONLY_CURRENT_MAP | CCE_PROCESS_LOGIC_ONLY_FOR_CURRENT_MAP);
   {
      char *path = cceGetTemporaryDirectory(0);
      cceSetMap2Dpath(path);
      free(path);
      
      struct Map2DElement elements[] = {
         {  3,   3, 10, 10, {0.0f, 0.0f, 5.0f, 5.0f, 2}, 0, 0, 0, 0},
         {-11, -11, 10, 24, {0.0f, 0.0f, 1.0f, 1.0f, 0}, 0, 0, 0, 1},
         {  3, -11, 10, 10, {0.0f, 0.0f, 1.0f, 1.0f, 0}, 0, 0, 0, 2},
         {  0, -11,  2, 24, {0.0f, 0.0f, 1.0f, 1.0f, 0}, 0, 0, 0, 3},
         {  2,   0, 11,  2, {0.0f, 0.0f, 1.0f, 1.0f, 0}, 0, 0, 0, 3},
      };
      struct ElementGroup moveGroups[1] = {
         {NULL, 0},
      };
      struct changeColorActionStruct colors[4] = {{0.711f, 0.64f,  0.453f, 1.0f, 1, CCE_CURRENT_MAP2D},
                                                  {0.001f, 0.487f, 0.0f,   1.0f, 2, CCE_CURRENT_MAP2D},
                                                  {0.69f,  0.645f, 0.042f, 1.0f, 3, CCE_CURRENT_MAP2D},
                                                  {0.70f,  0.40f,  0.0f,   1.0f, 1, CCE_DYNAMIC_MAP2D}};
      uint32_t colorActions[4] = {4, 4, 4, 4};
      uint32_t colorsOffsets[5] = {0,  sizeof(struct changeColorActionStruct), 2 * sizeof(struct changeColorActionStruct),
                                   3 * sizeof(struct changeColorActionStruct), 4 * sizeof(struct changeColorActionStruct)};
      struct Map2Ddev map = {0, 5, 0, elements, 1, moveGroups, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0, NULL,
                             4, colorActions, colorsOffsets, (cce_void*) colors, 0, NULL};
      cceWriteMap2Ddev(&map, NULL);
   }
   initPlayer(globalBoolsQuantity);
   createAdditionalDynamicElement((struct cce_ivec2) {2, 2}, globalBoolsQuantity);
   printf("Initialization complete\n");
   return cceEngine2D();
}
