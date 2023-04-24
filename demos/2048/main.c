/*
   Conservative Creator's Engine - open source engine for making games.
   Copyright © 2023 Andrey Gaivoronskiy

   This file is part of Conservative Creator's Engine.

   Conservative Creator's Engine is free software: you can redistribute it and/or modify it under 
   the terms of the GNU Lesser General Public License as published by the Free Software Foundation,
   either version 2 of the License, or (at your option) any later version.

   Conservative Creator's Engine is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
   PURPOSE. See the GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License along
   with Conservative Creator's Engine. If not, see <https://www.gnu.org/licenses/>.
*/

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <cce/os_interaction.h>

#include <cce/plugins/actions.h>
#include <cce/engine_common.h>
#include <cce/plugins/map2D/map2D.h>

#define BOARD_SIZE 4
#define CONTROL_UPDATE 0x1
#define LOCK_CONTROLS  0x2
#define TEXSIDE 64
#define MOVE_DURATION 96000

static uint8_t g_board[CCE_POW2(BOARD_SIZE)] = {0};
static struct cce_buffer *g_map;

uint8_t freeCells;
uint8_t flags;

struct movecell_t
{
   uint32_t ID;
   int16_t   xDiff;
   int16_t   yDiff;
   uint8_t  cell;
};

struct setcelltype_t
{
   uint32_t ID;
   uint8_t cell;
   uint8_t type;
};

struct cce_i8vec2 controlAxes;

static void moveCell (const void *data, uint16_t count);

static void controls (int8_t horizontal, int8_t vertical)
{
   flags |= ((horizontal || vertical) && ((controlAxes.x != 0) != (horizontal != 0) || (controlAxes.y != 0) != (vertical != 0))); // Control update
   controlAxes.x = horizontal;
   controlAxes.y = vertical;
}

static void calculateCellMove (void)
{
   struct movecell_t moveInfo;
   flags |= LOCK_CONTROLS;
   moveInfo.ID = 17;
   int8_t verticalAxis = (controlAxes.x == 0) ? controlAxes.y : 0;
   int8_t diff = ((int8_t)(verticalAxis | controlAxes.x) < 0) * 2 - 1;
   cce_void tmp[sizeof(struct cceRunActions) + 2 * sizeof(uint16_t) + sizeof(struct movecell_t) + 2 * sizeof(struct setcelltype_t)];
   struct cceRunActions *runActions = (struct cceRunActions*) &tmp;
   runActions->actionID = CCE_ACTION_RUNACTIONS;
   runActions->actionQuantity = 3;
   runActions->actionSizes[0] = sizeof(struct movecell_t);
   runActions->actionSizes[1] = sizeof(struct setcelltype_t);
   struct movecell_t *move = (struct movecell_t*) (tmp + sizeof(struct cceRunActions) + 2 * sizeof(uint16_t));
   move->ID = 17;
   struct setcelltype_t *setType1 = (struct setcelltype_t*)(move + 1);
   setType1->ID = 18;
   struct setcelltype_t *setType2 = (setType1 + 1);
   setType2->ID = 18;
   uint8_t isNewCell = 0, isMoved = 0;
   switch (((verticalAxis > 0) << 1) | ((controlAxes.x > 0) << 1) | (controlAxes.x != 0))
   {
      case 0:
         for (unsigned i = BOARD_SIZE; i < CCE_POW2(BOARD_SIZE); ++i)
         {
            if (g_board[i] != 0)
            {
               isNewCell = 0;
               unsigned cells = 0;
               for (int j = i - BOARD_SIZE; j >= 0; j -= BOARD_SIZE, ++cells)
               {
                  if (g_board[j] == g_board[i])
                  {
                     isNewCell = 1;
                     ++cells;
                     break;
                  }
                  if (g_board[j] != 0)
                     break;
               }
               if (cells == 0)
                  continue;
               isMoved = 1;
               moveInfo.cell = i;
               moveInfo.xDiff = 0;
               moveInfo.yDiff = -cells;
               cceDelayAction(TEXSIDE, MOVE_DURATION/TEXSIDE, sizeof(struct movecell_t), &moveInfo, CCE_DEFAULT);
               move->cell = i;
               move->xDiff = 0;
               move->yDiff = TEXSIDE * cells;
               setType1->cell = i;
               setType1->type = 0;
               setType2->cell = i - cells * BOARD_SIZE;
               setType2->type = g_board[i] + isNewCell;
               g_board[i - cells * BOARD_SIZE] = (g_board[i] + isNewCell) | isNewCell << 7;
               g_board[i] = 0;
               cceDelayAction(1, MOVE_DURATION, sizeof(tmp), runActions, CCE_DEFAULT);
               freeCells += isNewCell;
            }
         }
         break;
      case 2:
         for (unsigned i = BOARD_SIZE * (BOARD_SIZE - 1); i > 0;)
         {
            --i;
            if (g_board[i] != 0)
            {
               isNewCell = 0;
               unsigned cells = 0;
               for (unsigned j = i + BOARD_SIZE; j < CCE_POW2(BOARD_SIZE); j += BOARD_SIZE, ++cells)
               {
                  if (g_board[j] == g_board[i])
                  {
                     isNewCell = 1;
                     ++cells;
                     break;
                  }
                  if (g_board[j] != 0)
                     break;
               }
               if (cells == 0)
                  continue;
               isMoved = 1;
               moveInfo.cell = i;
               moveInfo.xDiff = 0;
               moveInfo.yDiff = cells;
               cceDelayAction(TEXSIDE, MOVE_DURATION/TEXSIDE, sizeof(struct movecell_t), &moveInfo, CCE_DEFAULT);
               move->cell = i;
               move->xDiff = 0;
               move->yDiff = -TEXSIDE * cells;
               setType1->cell = i;
               setType1->type = 0;
               setType2->cell = i + cells * BOARD_SIZE;
               setType2->type = g_board[i] + isNewCell;
               g_board[i + cells * BOARD_SIZE] = (g_board[i] + isNewCell) | isNewCell << 7;
               g_board[i] = 0;
               cceDelayAction(1, MOVE_DURATION, sizeof(tmp), runActions, CCE_DEFAULT);
               freeCells += isNewCell;
            }
         }
         break;
      case 1:
         for (unsigned i = 1; i < CCE_POW2(BOARD_SIZE); ++i)
         {
            for (unsigned end = i + BOARD_SIZE - 1; i < end; ++i)
            {
               if (g_board[i] != 0)
               {
                  isNewCell = 0;
                  unsigned cells = 0;
                  for (int j = i - 1; j >= (int)(i - (i % BOARD_SIZE)); --j, ++cells)
                  {
                     if (g_board[j] == g_board[i])
                     {
                        isNewCell = 1;
                        ++cells;
                        break;
                     }
                     if (g_board[j] != 0)
                        break;
                  }
                  if (cells == 0)
                     continue;
                  isMoved = 1;
                  moveInfo.cell = i;
                  moveInfo.xDiff = -cells;
                  moveInfo.yDiff = 0;
                  cceDelayAction(TEXSIDE, MOVE_DURATION/TEXSIDE, sizeof(struct movecell_t), &moveInfo, CCE_DEFAULT);
                  move->cell = i;
                  move->xDiff = TEXSIDE * cells;
                  move->yDiff = 0;
                  setType1->cell = i;
                  setType1->type = 0;
                  setType2->cell = i - cells;
                  setType2->type = g_board[i] + isNewCell;
                  g_board[i - cells] = (g_board[i] + isNewCell) | isNewCell << 7;
                  g_board[i] = 0;
                  cceDelayAction(1, MOVE_DURATION, sizeof(tmp), runActions, CCE_DEFAULT);
                  freeCells += isNewCell;
               }
            }
         }
         break;
      case 3:
         for (unsigned i = BOARD_SIZE * BOARD_SIZE; i > 0; )
         {
            --i;
            for (unsigned end = i - BOARD_SIZE + 1; i > end;)
            {
               --i;
               if (g_board[i] != 0)
               {
                  isNewCell = 0;
                  unsigned cells = 0;
                  for (unsigned j = i + 1; j < (i - (i % BOARD_SIZE) + BOARD_SIZE); ++j, ++cells)
                  {
                     if (g_board[j] == g_board[i])
                     {
                        isNewCell = 1;
                        ++cells;
                        break;
                     }
                     if (g_board[j] != 0)
                        break;
                  }
                  if (cells == 0)
                     continue;
                  isMoved = 1;
                  moveInfo.cell = i;
                  moveInfo.xDiff = cells;
                  moveInfo.yDiff = 0;
                  cceDelayAction(TEXSIDE, MOVE_DURATION/TEXSIDE, sizeof(struct movecell_t), &moveInfo, CCE_DEFAULT);
                  move->cell = i;
                  move->xDiff = -TEXSIDE * cells;
                  move->yDiff = 0;
                  setType1->cell = i;
                  setType1->type = 0;
                  setType2->cell = i + cells;
                  setType2->type = g_board[i] + isNewCell;
                  g_board[i + cells] = (g_board[i] + isNewCell) | isNewCell << 7;
                  g_board[i] = 0;
                  cceDelayAction(1, MOVE_DURATION, sizeof(tmp), runActions, CCE_DEFAULT);
                  freeCells += isNewCell;
               }
            }
         }
         break;
   }
   if (isMoved)
   {
      runActions->actionQuantity = 2;
      runActions->actionSizes[0] = sizeof(uint32_t);
      *(uint32_t*)(tmp + sizeof(struct cceRunActions)) = 16;
      *(uint32_t*)(tmp + sizeof(struct cceRunActions) + sizeof(uint32_t)) = 19;
      cceDelayAction(1, 96000, sizeof(struct cceRunActions) + 2 * sizeof(uint32_t), runActions, CCE_DEFAULT);
   }
   else
   {
      flags &= ~LOCK_CONTROLS;
   }
   for (unsigned i = 0; i < CCE_POW2(BOARD_SIZE); ++i)
      g_board[i] &= ~0x80; // Avoiding recursive merge (ununtended mechanic for this clone)
   flags &= ~CONTROL_UPDATE;
}

static struct cce_buffer* genBoard (void)
{
   struct cce_buffer *map = cceCreateMap2Ddynamic();
   struct cce_usedtexinfo *textures = cceGetResource(CCE_RESOURCE_TEXTURE, map);
   CCE_ALLOC_ARRAY(textures->texturesMapDependsOn, 1);
   textures->texturesMapDependsOnQuantity = 1;
   uint16_t texture = cceLoadTexture("tiles.png", 1);
   textures->texturesMapDependsOn[0] = texture;
   struct cce_elementposition positions[] =
   {
      {{0,  0},  1, 0, 0},
      {{0,  0},  2, 0, 0},
      {{0,  0},  3, 0, 0},
      {{0,  0},  4, 0, 0},
      {{0,  0},  5, 0, 0},
      {{0,  0},  6, 0, 0},
      {{0,  0},  7, 0, 0},
      {{0,  0},  8, 0, 0},
      {{0,  0},  9, 0, 0},
      {{0,  0}, 10, 0, 0},
      {{0,  0}, 11, 0, 0},
      {{0,  0}, 12, 0, 0},
      {{0,  0}, 13, 0, 0},
      {{0,  0}, 14, 0, 0},
      {{0,  0}, 15, 0, 0},
      {{0,  0}, 16, 0, 0},
   };
   struct cce_element elements[] = 
   {
      {{-2*TEXSIDE, -2*TEXSIDE}, {0, 0}, {TEXSIDE, TEXSIDE}, texture, 0, 0},
      {{-1*TEXSIDE, -2*TEXSIDE}, {0, 0}, {TEXSIDE, TEXSIDE}, texture, 0, 0},
      {{ 0,         -2*TEXSIDE}, {0, 0}, {TEXSIDE, TEXSIDE}, texture, 0, 0},
      {{ 1*TEXSIDE, -2*TEXSIDE}, {0, 0}, {TEXSIDE, TEXSIDE}, texture, 0, 0},
      {{-2*TEXSIDE, -1*TEXSIDE}, {0, 0}, {TEXSIDE, TEXSIDE}, texture, 0, 0},
      {{-1*TEXSIDE, -1*TEXSIDE}, {0, 0}, {TEXSIDE, TEXSIDE}, texture, 0, 0},
      {{ 0        , -1*TEXSIDE}, {0, 0}, {TEXSIDE, TEXSIDE}, texture, 0, 0},
      {{ 1*TEXSIDE, -1*TEXSIDE}, {0, 0}, {TEXSIDE, TEXSIDE}, texture, 0, 0},
      {{-2*TEXSIDE,  0        }, {0, 0}, {TEXSIDE, TEXSIDE}, texture, 0, 0},
      {{-1*TEXSIDE,  0        }, {0, 0}, {TEXSIDE, TEXSIDE}, texture, 0, 0},
      {{ 0        ,  0        }, {0, 0}, {TEXSIDE, TEXSIDE}, texture, 0, 0},
      {{ 1*TEXSIDE,  0        }, {0, 0}, {TEXSIDE, TEXSIDE}, texture, 0, 0},
      {{-2*TEXSIDE,  1*TEXSIDE}, {0, 0}, {TEXSIDE, TEXSIDE}, texture, 0, 0},
      {{-1*TEXSIDE,  1*TEXSIDE}, {0, 0}, {TEXSIDE, TEXSIDE}, texture, 0, 0},
      {{ 0,          1*TEXSIDE}, {0, 0}, {TEXSIDE, TEXSIDE}, texture, 0, 0},
      {{ 1*TEXSIDE,  1*TEXSIDE}, {0, 0}, {TEXSIDE, TEXSIDE}, texture, 0, 0},
   };
   memcpy(cceGetElementsPosition(0, 0, BOARD_SIZE*BOARD_SIZE, map), positions, BOARD_SIZE*BOARD_SIZE * sizeof(struct cce_elementposition));
   memcpy(cceGetElements(0,            BOARD_SIZE*BOARD_SIZE, map), elements,  BOARD_SIZE*BOARD_SIZE * sizeof(struct cce_element));
   struct cce_renderinginfo *info = cceGetRenderingInfo(map);
   return map;
}

static void createCell (const void *data, uint16_t count)
{
   CCE_UNUSED(data);
   do
   {
      // Extracting randomness from rand (lower-order bits are less random then higher-order bits, so prefer the latter)
      unsigned random = rand();
      div_t position = div(random, ((RAND_MAX + 1u)/freeCells));
      unsigned type = (position.rem / ((RAND_MAX + 1u)/freeCells/10)) == 0; // 0 is 4, other is 2 (10%)
      unsigned i = 0, realPos = 0;
      while (i <= position.quot)
      {
         i += g_board[realPos] == 0;
         ++realPos;
      }
      --realPos;
      g_board[realPos] = 1 + type;
      struct cce_element *element = cceGetElements(realPos, 1, g_map);
      element->data.texturePosition.x = TEXSIDE * (1 + type);
      --freeCells;
      --count;
   }
   while (count > 0);
   cceSetElementsUpdated(cceGetRenderingInfo(g_map));
}

static void moveCell (const void *data, uint16_t count)
{
   const struct movecell_t *vals = data;
   struct cce_elementposition *position = cceGetElementsPosition(0, vals->cell, 1, g_map);
   position->position.x += vals->xDiff * (int16_t)count;
   position->position.y += vals->yDiff * (int16_t)count;
   cceSetElementsPositionsUpdated(cceGetElementPositionArray(0, g_map));
}

static void setElementType (const void *data, uint16_t count)
{
   CCE_UNUSED(count);
   const struct setcelltype_t *vals = data;
   struct cce_element *element = cceGetElements(vals->cell, 1, g_map);
   element->data.texturePosition.x = (vals->type % BOARD_SIZE) * TEXSIDE;
   element->data.texturePosition.y = (vals->type / BOARD_SIZE) * TEXSIDE;
   cceSetElementsUpdated(cceGetRenderingInfo(g_map));
}

static void unlockControls (const void *data, uint16_t count)
{
   CCE_UNUSED(data);
   CCE_UNUSED(count);
   flags &= ~LOCK_CONTROLS;
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
   if (cceInit("demos/2048/game.ini"))
   {
      fputs("Initialization error", stderr);
      return -1;
   }
   g_map = genBoard();
   freeCells = CCE_POW2(BOARD_SIZE);
   controlAxes = (struct cce_i8vec2) {0, 0};
   cceSetRenderingLayerMap2D(0, 0, g_map);
   cceSetAxisChangeCallback(controls, CCE_AXISPAIR_LSTICK);
   cceRegisterAction(16, createCell, NULL);
   cceRegisterAction(17, moveCell, NULL);
   cceRegisterAction(18, setElementType, NULL);
   cceRegisterAction(19, unlockControls, NULL);
   {
      unsigned seed;
      cceGetRandom(&seed, sizeof(unsigned));
      srand(seed);
      rand();
   }
   createCell(NULL, 1);
   while (cceEngineShouldTerminate() == 0)
   {
      cceRenderMap2D();
      if ((flags & (CONTROL_UPDATE | LOCK_CONTROLS)) == CONTROL_UPDATE)
      {
         calculateCellMove();
      }
      cceUpdate();
      cceScreenUpdate();
   }
   cceFreeMap2D(g_map);
   cceTerminate();
   return 0;
}
