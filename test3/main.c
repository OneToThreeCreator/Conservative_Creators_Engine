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

#include <coffeechain/map2D/base_actions.h>
#include <coffeechain/engine_common.h>
#include <coffeechain/map2D/map2D.h>
#include <coffeechain/path_getters.h>
#include <coffeechain/plugins/text_rendering.h>

static void initPlayer (uint32_t globalBoolsQuantity)
{
   struct Map2DElementDev player = {0, -1, 1, 2, {0, 0, 7, 16, 1}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0};
   cceCreateMap2DElementDynamicMap2D(&player, CCE_ELEMENT_WITHOUT_COLLIDER, CCE_DEFAULT);
   struct Map2DElementDev playerTrigger = {0, 0, 1, 1, {0, 0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 0};
   uint32_t colliders[4];
   colliders[0] = cceCreateMap2DElementDynamicMap2D(&playerTrigger, CCE_COLLIDER, CCE_DEFAULT);
   playerTrigger.y = -2;
   colliders[1] = cceCreateMap2DElementDynamicMap2D(&playerTrigger, CCE_COLLIDER, CCE_DEFAULT);
   playerTrigger.x = -1;
   playerTrigger.y = -1;
   colliders[2] = cceCreateMap2DElementDynamicMap2D(&playerTrigger, CCE_COLLIDER, CCE_DEFAULT);
   playerTrigger.x = 1;
   colliders[3] = cceCreateMap2DElementDynamicMap2D(&playerTrigger, CCE_COLLIDER, CCE_DEFAULT);
   uint16_t collisions[4];
   for (uint8_t i = 0; i < 4; ++i)
   {
      if (cceAddElementInGroupDynamicMap2D(CCE_COLLISION_GROUP, i + 1, colliders[i]) != 0)
         printf("cceAddElementInGroupDynamicMap2D indicates an error!\n");
      collisions[i] = cceCreateCollisionDynamicMap2D(i + 1, 0, 0, 1);
   }
   uint_fast16_t *abNc = cceParseStringToLogicOperations("a & b & !c", NULL);
   uint16_t logicIDs[5];
   uint16_t timerID = cceCreateTimerDynamicMap2D(0.03f);
   uint16_t moveElements[3] = {globalBoolsQuantity - 12, timerID, collisions[0]};
   cce_enum types[3] = {CCE_GLOBAL_BOOL_LOGIC_ELEMENT, CCE_TIMER_LOGIC_ELEMENT, CCE_COLLISION_LOGIC_ELEMENT};
   for (uint16_t *iterator = logicIDs, *end = logicIDs + 4; iterator < end; ++iterator)
   {
      *iterator = cceCreateLogicDynamicMap2D();
      cceUpdateLogicElementsByTruthTableDynamicMap2D(*iterator, 3, moveElements, types, abNc);
      ++(moveElements[0]);
      ++(moveElements[2]);
   }
   {
      uint16_t updateTimerElements[2] = {globalBoolsQuantity, timerID};
      logicIDs[4] = cceCreateLogicDynamicMap2D();
      cceUpdateLogicElementsByBooleanExpressionDynamicMap2D(logicIDs[4], updateTimerElements, types, "a & b");
   }
   free(abNc);
   uint32_t moveAndSetBoolAction[2] = {0, 5};
   struct setBoolActionStruct setBoolSt = {globalBoolsQuantity, CCE_ENABLE_BOOL};
   struct moveActionStruct moveSt = {{0, -1}, 0, CCE_SHIFT, CCE_DYNAMIC_MAP2D};
   const void *moveVerticallyActionArgs[2] = {&moveSt, &setBoolSt};
   uint32_t moveVerticallyActionArgSizes[2] = {sizeof(struct moveActionStruct), sizeof(struct setBoolActionStruct)};
   cceUpdateLogicActionsDynamicMap2D(logicIDs[0], 2, moveAndSetBoolAction, moveVerticallyActionArgs, moveVerticallyActionArgSizes);
   moveSt.coords.y = 1;
   cceUpdateLogicActionsDynamicMap2D(logicIDs[1], 2, moveAndSetBoolAction, moveVerticallyActionArgs, moveVerticallyActionArgSizes);
   moveSt.coords.x = 1;
   moveSt.coords.y = 0;
   setBoolSt.action = CCE_DISABLE_BOOL;
   struct startTimerActionStruct startTimerSt = {timerID, CCE_DYNAMIC_MAP2D};
   const void *moveHorizontallyActionArgs[3] = {&moveSt, &setBoolSt, &startTimerSt};
   uint32_t moveHorizontallyActionArgSizes[3] = {sizeof(struct moveActionStruct), sizeof(struct setBoolActionStruct), sizeof(struct startTimerActionStruct)};

   uint32_t moveAndSetBoolAndTimerAction[3] = {0, 5, 7};
   cceUpdateLogicActionsDynamicMap2D(logicIDs[2], 3, moveAndSetBoolAndTimerAction, moveHorizontallyActionArgs, moveHorizontallyActionArgSizes);
   moveSt.coords.x = -1;
   cceUpdateLogicActionsDynamicMap2D(logicIDs[3], 3, moveAndSetBoolAndTimerAction, moveHorizontallyActionArgs, moveHorizontallyActionArgSizes);
   uint32_t setBoolAndTimerAction[2] = {5, 7};
   const void *timerAndBool[2] = {&setBoolSt, &startTimerSt};
   uint32_t timerAndBoolSizes[2] = {sizeof(struct setBoolActionStruct), sizeof(struct startTimerActionStruct)};
   cceUpdateLogicActionsDynamicMap2D(logicIDs[4], 2, setBoolAndTimerAction, timerAndBool, timerAndBoolSizes);
   return;
}

static void createAdditionalDynamicElement (struct cce_i32vec2 coords, uint32_t globalBoolsQuantity)
{
   struct Map2DElementDev element = {coords.x, coords.y, 2, 2, {0, 0, 16, 16, 0}, {1, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, {0, 0, 0, 0}, 0, 1};
   cceCreateMap2DElementDynamicMap2D(&element, CCE_ELEMENT_WITHOUT_COLLIDER, CCE_DEFAULT);

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
   struct moveActionStruct moveSt = {{0, 1}, 1, CCE_SHIFT, CCE_DYNAMIC_MAP2D};
   const void *moveVerticallyActionArgs[2] = {&moveSt, &setBoolSt};
   uint32_t moveVerticallyActionArgSizes[2] = {sizeof(struct moveActionStruct), sizeof(struct setBoolActionStruct)};
   cceUpdateLogicActionsDynamicMap2D(logicIDs[0], 2, moveAndSetBoolAction, moveVerticallyActionArgs, moveVerticallyActionArgSizes);
   moveSt.coords.y = -1;
   cceUpdateLogicActionsDynamicMap2D(logicIDs[1], 2, moveAndSetBoolAction, moveVerticallyActionArgs, moveVerticallyActionArgSizes);
   moveSt.coords.x = -1;
   moveSt.coords.y = 0;
   setBoolSt.action = CCE_DISABLE_BOOL;
   struct startTimerActionStruct startTimerSt = {timerID, CCE_DYNAMIC_MAP2D};
   const void *moveHorizontallyActionArgs[3] = {&moveSt, &setBoolSt, &startTimerSt};
   uint32_t moveHorizontallyActionArgSizes[3] = {sizeof(struct moveActionStruct), sizeof(struct setBoolActionStruct), sizeof(struct startTimerActionStruct)};

   uint32_t moveAndSetBoolAndTimerAction[3] = {0, 5, 7};
   cceUpdateLogicActionsDynamicMap2D(logicIDs[2], 3, moveAndSetBoolAndTimerAction, moveHorizontallyActionArgs, moveHorizontallyActionArgSizes);
   moveSt.coords.x = 1;
   cceUpdateLogicActionsDynamicMap2D(logicIDs[3], 3, moveAndSetBoolAndTimerAction, moveHorizontallyActionArgs, moveHorizontallyActionArgSizes);
   uint32_t setBoolAndTimerAction[2] = {5, 7};
   const void *timerAndBool[2] = {&setBoolSt, &startTimerSt};
   uint32_t timerAndBoolSizes[2] = {sizeof(struct setBoolActionStruct), sizeof(struct startTimerActionStruct)};
   cceUpdateLogicActionsDynamicMap2D(logicIDs[4], 2, setBoolAndTimerAction, timerAndBool, timerAndBoolSizes);
   return;
}

static void createMap2D (uint16_t ID, uint16_t exitMapsQuantity, struct ExitMap2D *exitMaps)
{
   struct Map2DElement elements[29] = {
      {  0, -11,  2, 24, {0, 0, 16, 16, 0}, {0, 0, 0, 0}, {3, 0, 0, 0}, 0},
      {  2,   0, 11,  2, {0, 0, 16, 16, 0}, {0, 0, 0, 0}, {3, 0, 0, 0}, 0},
      {-11, -11, 10, 24, {0, 0, 16, 16, 0}, {0, 0, 0, 0}, {1, 0, 0, 0}, 0},
      {  3, -11, 10, 10, {0, 0, 16, 16, 0}, {0, 0, 0, 0}, {2, 0, 0, 0}, 0},
   };
   uint32_t walls[27] = {2, 3};
   uint32_t *witerator = walls + 2;
   for (uint32_t j = 0; j < 5; ++j)
   {
      for (uint32_t i = 0; i < 5; ++i)
      {
         elements[j*5 + i + 4] = (struct Map2DElement) {3 + i*2, 3 + j*2, 2, 2, {0, 0, 16, 16, 2}, {0, 0, 0, 0}, {0, 0, 0, 0}, 0};
         *witerator = j*5 + i + 4;
         ++witerator;
      }
   }
   struct changeColorActionStruct colors[4] = {{0.711f, 0.64f,  0.453f, 1.0f, 1, CCE_CURRENT_MAP2D},
                                               {0.001f, 0.487f, 0.0f,   1.0f, 2, CCE_CURRENT_MAP2D},
                                               {0.69f,  0.645f, 0.042f, 1.0f, 3, CCE_CURRENT_MAP2D},
                                               {0.70f,  0.40f,  0.0f,   1.0f, 1, CCE_DYNAMIC_MAP2D}};
   uint32_t colorActions[4] = {4, 4, 4, 4};
   uint32_t colorsOffsets[5] = {0,  sizeof(struct changeColorActionStruct), 2 * sizeof(struct changeColorActionStruct),
                                3 * sizeof(struct changeColorActionStruct), 4 * sizeof(struct changeColorActionStruct)};
   
   struct ElementGroup collisionGroup = {walls, 27};
   struct Map2Ddev map = {ID, 29, 0, elements, 0, NULL, 0, NULL, 0, NULL, 1, &collisionGroup, 0, NULL, 0, NULL, 0, NULL,
                          4, colorActions, colorsOffsets, (cce_void*) colors, exitMapsQuantity, exitMaps};
   cceWriteMap2Ddev(&map, NULL);
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
      if (argv[1][0] == '/')
      {
         pathLength = strlen(argv[1]);
         path = malloc(pathLength + 16);
         memcpy(path, argv[1], pathLength + 1);
      }
      else
      {
         char *appendString = argv[1];
         appendString += (appendString[0] == '.') * 2;
         size_t appendLength = strlen(appendString);
         path = cceGetCurrentPath(appendLength + 15);
         pathLength = strlen(path);
         memcpy(path + pathLength, appendString, appendLength + 1);
         pathLength += appendLength;
      }
   }
   const uint32_t globalBoolsQuantity = 32768;
   if (cceInitEngine2D(globalBoolsQuantity, 48, 48, "CoffeeChain TEST", path, CCE_RENDER_VISIBLE_MAPS | CCE_PROCESS_LOGIC_FOR_VISIBLE_MAPS) != 0)
   {
      free(path);
      printf("Initialization failure\n");
      return -1;
   }
   cceAppendPath(path, pathLength + 16, "test3/textures");
   cceSetTexturesPath(path);
   free(path);
   {
      char *path = cceGetTemporaryDirectory(0);
      cceSetMap2Dpath(path);
      free(path);
      struct ExitMap2D exitMaps[2] = {
         {0, 0,  24,  13, -11, 13, 0x0},
         {0, 0, -24, -11, -11, 13, 0x2},
      };
      createMap2D(0, 2, exitMaps);
   }
   initPlayer(globalBoolsQuantity);
   createAdditionalDynamicElement((struct cce_i32vec2) {2, 2}, globalBoolsQuantity);
   cceSetGridMultiplierMap2D(1.0f);
   // Add delayed actions
   {
      struct moveActionStruct actionSt = {-1, 0, 1, CCE_SHIFT, CCE_DYNAMIC_MAP2D};
      cceDelayActionMap2D(CCE_MOVE_ACTION, sizeof(struct moveActionStruct), &actionSt, 10, 0.2, CCE_DYNAMIC_MAP2D);
      actionSt = (struct moveActionStruct) {0, 1, 0, CCE_SHIFT, CCE_DYNAMIC_MAP2D};
      cceDelayActionMap2D(CCE_MOVE_ACTION, sizeof(struct moveActionStruct), &actionSt, 5, 0.4, CCE_DYNAMIC_MAP2D);
   }
   // Render some text
   {
      if (cceInitTextRendering(CCE_UTF8_ENCODING) != 0)
      {
         printf("Initialization failure\n");
         return -1;
      }
      if (cceLoadBitmapFont("ascii_48x48_l5x8") != 0)
      {
         printf("Font loading failure\n");
         return -1;
      }
      struct Map2DElementDev textTemplate = {-16, 9, 0, 0, {0}, {0}, {0}, {0}, {0}, 0, 0};
      char string[] = "axj%";
      ccePrintString(string, &textTemplate, CCE_ELEMENT_WITHOUT_COLLIDER, CCE_DEFAULT);
   }
   printf("Initialization complete\n");
   return cceEngine2D();
}
