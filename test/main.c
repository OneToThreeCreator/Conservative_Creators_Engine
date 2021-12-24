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

int main (int argc, char **argv)
{
   char path[256];
   if (argc < 2)
   {
      getCurrentPath(path, 256);
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
         getCurrentPath(path, 256);
         strncat(path, appendString, 256 - strnlen(path, 256));
      }
   }
   const uint32_t globalBoolsQuantity = 32768u;
   if (initEngine2D(globalBoolsQuantity, 48u, 48u, "CoffeeChain TEST", path) != 0)
   {
      return -1;
   }
   setFlags2D(CCE_RENDER_ONLY_CURRENT_MAP | CCE_PROCESS_LOGIC_ONLY_FOR_CURRENT_MAP);
   {
      struct Map2DElement elements[] = {
         {3, 3, 10u, 10u, {0.0f, 0.0f, 0.0f, 0.0f, 0u}, 0, 0u, 0u, 2u},
         {-11, -11, 10u, 24u, {0.0f, 0.0f, 0.0f, 0.0f, 0u}, 0, 0u, 0u, 1u},
         {3, -11, 10u, 10u, {0.0f, 0.0f, 0.0f, 0.0f, 0u}, 0, 0u, 0u, 2u},
         {0, -11, 2u, 24u,  {0.0f, 0.0f, 0.0f, 0.0f, 0u}, 0, 0u, 0u, 3u},
         {2, 0, 11u, 2u,  {0.0f, 0.0f, 0.0f, 0.0f, 0u}, 0, 0u, 0u, 3u},
      };
      struct ElementGroup moveGroups[1u] = {
         {NULL, 0u},
      };
      uint16_t timerID = 0u;
      uint32_t timerAndColorsActions[4] = {7u, 4u, 4u, 4u};
      uint32_t timerAndColorsOffsets[5] = {0u, sizeof(uint16_t), 4 * sizeof(float) + 2 * sizeof(uint16_t),
                                           8 * sizeof(float) + 3 * sizeof(uint16_t), 12 * sizeof(float) + 4 * sizeof(uint16_t)};
      cce_void timerAndColors[timerAndColorsOffsets[4]];
      *((uint16_t*) (timerAndColors + timerAndColorsOffsets[0]))      = timerID;
      
      *((float*)    (timerAndColors + timerAndColorsOffsets[1]) + 0u) = 0.711f;
      *((float*)    (timerAndColors + timerAndColorsOffsets[1]) + 1u) = 0.64f;
      *((float*)    (timerAndColors + timerAndColorsOffsets[1]) + 2u) = 0.453f;
      *((float*)    (timerAndColors + timerAndColorsOffsets[1]) + 3u) = 1.0f;
      *((uint16_t*) (timerAndColors + timerAndColorsOffsets[1]) + 8u) = 1u;
      
      *((float*)    (timerAndColors + timerAndColorsOffsets[2]) + 0u) = 0.001f;
      *((float*)    (timerAndColors + timerAndColorsOffsets[2]) + 1u) = 0.487f;
      *((float*)    (timerAndColors + timerAndColorsOffsets[2]) + 2u) = 0.0f;
      *((float*)    (timerAndColors + timerAndColorsOffsets[2]) + 3u) = 1.0f;
      *((uint16_t*) (timerAndColors + timerAndColorsOffsets[2]) + 8u) = 2u;
      
      *((float*)    (timerAndColors + timerAndColorsOffsets[3]) + 0u) = 0.69f;
      *((float*)    (timerAndColors + timerAndColorsOffsets[3]) + 1u) = 0.645f;
      *((float*)    (timerAndColors + timerAndColorsOffsets[3]) + 2u) = 0.042f;
      *((float*)    (timerAndColors + timerAndColorsOffsets[3]) + 3u) = 1.0f;
      *((uint16_t*) (timerAndColors + timerAndColorsOffsets[3]) + 8u) = 3u;
      
      uint16_t moveUpElements[2]   = {globalBoolsQuantity - 12u, timerID};
      uint16_t moveDownElements[2] = {globalBoolsQuantity - 11u, timerID};
      uint16_t moveLeftElements[2] = {globalBoolsQuantity - 10u, timerID};
      uint16_t moveRightElements[2] = {globalBoolsQuantity - 9u, timerID};
      uint32_t moveAndTimerActions[2] = {0u, 7u};
      uint32_t startTimerAction = 7u;
      uint32_t moveAndTimerOffsets[3] = {0u, 2 * sizeof(uint32_t) + sizeof(uint16_t), 2 * sizeof(uint32_t) + 2 * sizeof(uint16_t)};
      uint32_t timerOffsets[2] = {0u, sizeof(uint16_t)};
      
      int32_t moveUp[3] = {0, -1};
      *(uint16_t*) (moveUp + 2u) = 0u;
      *((uint16_t*) (moveUp + 2u) + 1u) = timerID;
      int32_t moveDown[3] = {0, 1};
      *(uint16_t*) (moveDown + 2u) = 0u;
      *((uint16_t*) (moveDown + 2u) + 1u) = timerID;
      int32_t moveLeft[3] = {1, 0};
      *(uint16_t*) (moveLeft + 2u) = 0u;
      *((uint16_t*) (moveLeft + 2u) + 1u) = timerID;
      int32_t moveRight[3] = {-1, 0};
      *(uint16_t*) (moveRight + 2u) = 0u;
      *((uint16_t*) (moveRight + 2u) + 1u) = timerID;
      
      uint_fast16_t *aandb = parseStringToLogicOperations("a & b", NULL);
      
      struct ElementLogic logic[] = {
         {2u, 2u, moveUpElements,    aandb, 0x8 /*00001000 in hex*/, moveAndTimerActions, moveAndTimerOffsets, (void*) moveUp},
         {2u, 2u, moveDownElements,  aandb, 0x8 /*00001000 in hex*/, moveAndTimerActions, moveAndTimerOffsets, (void*) moveDown},
         {2u, 2u, moveLeftElements,  aandb, 0x8 /*00001000 in hex*/, moveAndTimerActions, moveAndTimerOffsets, (void*) moveLeft},
         {2u, 2u, moveRightElements, aandb, 0x8 /*00001000 in hex*/, moveAndTimerActions, moveAndTimerOffsets, (void*) moveRight},
         
      };
      double globalTimer = 0.01;
      struct Map2Ddev map = {0u, 5u, 0u, elements, 1u, moveGroups, 0u, NULL, 0u, NULL, 0u, NULL, 0u, NULL, 1u, &globalTimer, 4u, logic,
                             4u, timerAndColorsActions, timerAndColorsOffsets, (void*) timerAndColors, 0u, NULL};
      char *path = getTemporaryDirectory(0u);
      setMap2Dpath(path);
      free(path);
      writeMap2Ddev(&map, NULL);
      free(aandb);
   }
   {
      struct Map2DElementDev player = {-1, -1, 2u, 2u, {0.0f, 0.0f, 0.0f, 0.0f, 0u}, 0u, 0u, 0u, 0u, 0u, 0u, 1u};
      createMap2DElementDynamicMap2D(&player, 1u);
   }
   printf("Initialization complete\n");
   return engine2D();
}
