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
#define MOVE_DURATION 128

static uint8_t g_board[CCE_POW2(BOARD_SIZE)] = {0};
static struct cce_buffer *g_map;

uint8_t freeCells;
uint8_t flags;

struct movecell_t
{
   uint32_t UID;
   int16_t  xDiff;
   int16_t  yDiff;
   uint8_t  cell;
};

struct setcelltype_t
{
   uint32_t ID;
   uint8_t cell;
   uint8_t type;
};

static uint32_t moveCellUID, setCellTypeUID, createCellUID, unlockControlsUID;

struct cce_i8vec2 controlAxes;

static void moveCell (void *data, uint32_t count, struct cce_buffer *state);

static void controls (int8_t horizontal, int8_t vertical)
{
   flags |= ((horizontal || vertical) && ((controlAxes.x != 0) != (horizontal != 0) || (controlAxes.y != 0) != (vertical != 0))); // Control update
   controlAxes.x = horizontal;
   controlAxes.y = vertical;
}

static void calculateCellMove (void)
{
   flags |= LOCK_CONTROLS;
   int8_t verticalAxis = (controlAxes.x == 0) ? controlAxes.y : 0;
   int8_t diff = ((int8_t)(verticalAxis | controlAxes.x) < 0) * 2 - 1;
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
               CCEA_RUNACTIONS_CREATE_STATIC1(moveAction, struct cceaDelayActionsRepeated, ((struct cceaDelayActionsRepeated){cceaBasicActionUIDs[CCEA_DELAY_ACTIONS_REPEATED], 0, MOVE_DURATION/TEXSIDE, TEXSIDE}),
                                                          struct movecell_t,               ((struct movecell_t){moveCellUID, 0, -cells, i}));
               cceaRunAction((struct cceaAction*)moveAction, 1, g_map);
               CCEA_RUNACTIONS_CREATE_STATIC3(runActions, struct cceaDelayActions, ((struct cceaDelayActions){cceaBasicActionUIDs[CCEA_DELAY_ACTIONS], 0, MOVE_DURATION}),
                                                          struct movecell_t,     ((struct movecell_t){moveCellUID, 0, TEXSIDE * cells, i}),
                                                          struct setcelltype_t,  ((struct setcelltype_t){setCellTypeUID, i, 0}),
                                                          struct setcelltype_t,  ((struct setcelltype_t){setCellTypeUID, i - cells * BOARD_SIZE, g_board[i] + isNewCell}));
               cceaRunAction((struct cceaAction*)runActions, 1, g_map);
               g_board[i - cells * BOARD_SIZE] = (g_board[i] + isNewCell) | isNewCell << 7;
               g_board[i] = 0;
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
               CCEA_RUNACTIONS_CREATE_STATIC1(moveAction, struct cceaDelayActionsRepeated, ((struct cceaDelayActionsRepeated){cceaBasicActionUIDs[CCEA_DELAY_ACTIONS_REPEATED], 0, MOVE_DURATION/TEXSIDE, TEXSIDE}),
                                                          struct movecell_t,               ((struct movecell_t){moveCellUID, 0, cells, i}));
               cceaRunAction((struct cceaAction*)moveAction, 1, g_map);
               CCEA_RUNACTIONS_CREATE_STATIC3(runActions, struct cceaDelayActions, ((struct cceaDelayActions){cceaBasicActionUIDs[CCEA_DELAY_ACTIONS], 0, MOVE_DURATION}),
                                                          struct movecell_t,     ((struct movecell_t){moveCellUID, 0, -TEXSIDE * cells, i}),
                                                          struct setcelltype_t,  ((struct setcelltype_t){setCellTypeUID, i, 0}),
                                                          struct setcelltype_t,  ((struct setcelltype_t){setCellTypeUID, i + cells * BOARD_SIZE, g_board[i] + isNewCell}));
               cceaRunAction((struct cceaAction*)runActions, 1, g_map);
               g_board[i + cells * BOARD_SIZE] = (g_board[i] + isNewCell) | isNewCell << 7;
               g_board[i] = 0;
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
                  CCEA_RUNACTIONS_CREATE_STATIC1(moveAction, struct cceaDelayActionsRepeated, ((struct cceaDelayActionsRepeated){cceaBasicActionUIDs[CCEA_DELAY_ACTIONS_REPEATED], 0, MOVE_DURATION/TEXSIDE, TEXSIDE}),
                                                             struct movecell_t,               ((struct movecell_t){moveCellUID, -cells, 0, i}));
                  cceaRunAction((struct cceaAction*)moveAction, 1, g_map);
                  CCEA_RUNACTIONS_CREATE_STATIC3(runActions, struct cceaDelayActions, ((struct cceaDelayActions){cceaBasicActionUIDs[CCEA_DELAY_ACTIONS], 0, MOVE_DURATION}),
                                                             struct movecell_t,     ((struct movecell_t){moveCellUID, TEXSIDE * cells, 0, i}),
                                                             struct setcelltype_t,  ((struct setcelltype_t){setCellTypeUID, i, 0}),
                                                             struct setcelltype_t,  ((struct setcelltype_t){setCellTypeUID, i - cells, g_board[i] + isNewCell}));
                  cceaRunAction((struct cceaAction*)runActions, 1, g_map);
                  g_board[i - cells] = (g_board[i] + isNewCell) | isNewCell << 7;
                  g_board[i] = 0;
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
                  CCEA_RUNACTIONS_CREATE_STATIC1(moveAction, struct cceaDelayActionsRepeated, ((struct cceaDelayActionsRepeated){cceaBasicActionUIDs[CCEA_DELAY_ACTIONS_REPEATED], 0, MOVE_DURATION/TEXSIDE, TEXSIDE}),
                                                             struct movecell_t,               ((struct movecell_t){moveCellUID, cells, 0, i}));
                  cceaRunAction((struct cceaAction*)moveAction, 1, g_map);
                  CCEA_RUNACTIONS_CREATE_STATIC3(runActions, struct cceaDelayActions, ((struct cceaDelayActions){cceaBasicActionUIDs[CCEA_DELAY_ACTIONS], 0, MOVE_DURATION}),
                                                             struct movecell_t,     ((struct movecell_t){moveCellUID, -TEXSIDE * cells, 0, i}),
                                                             struct setcelltype_t,  ((struct setcelltype_t){setCellTypeUID, i, 0}),
                                                             struct setcelltype_t,  ((struct setcelltype_t){setCellTypeUID, i + cells, g_board[i] + isNewCell}));
                  cceaRunAction((struct cceaAction*)runActions, 1, g_map);
                  g_board[i + cells] = (g_board[i] + isNewCell) | isNewCell << 7;
                  g_board[i] = 0;
                  freeCells += isNewCell;
               }
            }
         }
         break;
   }
   if (isMoved)
   {
      CCEA_RUNACTIONS_CREATE_STATIC2(runActions, struct cceaDelayActions, ((struct cceaDelayActions){cceaBasicActionUIDs[CCEA_DELAY_ACTIONS], 0, MOVE_DURATION}),
                                                 uint32_t, createCellUID,
                                                 uint32_t, unlockControlsUID);
      cceaRunAction((struct cceaAction*)runActions, 1, g_map);
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

static void createCell (void *data, uint32_t count, struct cce_buffer *state)
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
      struct cce_element *element = cceGetElements(realPos, 1, state);
      element->data.texturePosition.x = TEXSIDE * (1 + type);
      --freeCells;
      --count;
   }
   while (count > 0);
   cceSetElementsUpdated(cceGetRenderingInfo(state));
}

static void moveCell (void *data, uint32_t count, struct cce_buffer *state)
{
   const struct movecell_t *vals = data;
   struct cce_elementposition *position = cceGetElementsPosition(0, vals->cell, 1, state);
   position->position.x += vals->xDiff * (int16_t)count;
   position->position.y += vals->yDiff * (int16_t)count;
   cceSetElementsPositionsUpdated(cceGetElementPositionArray(0, state));
}

static void setElementType (void *data, uint32_t count, struct cce_buffer *state)
{
   CCE_UNUSED(count);
   const struct setcelltype_t *vals = data;
   struct cce_element *element = cceGetElements(vals->cell, 1, state);
   element->data.texturePosition.x = (vals->type % BOARD_SIZE) * TEXSIDE;
   element->data.texturePosition.y = (vals->type / BOARD_SIZE) * TEXSIDE;
   cceSetElementsUpdated(cceGetRenderingInfo(state));
}

static void unlockControls (void *data, uint32_t count, struct cce_buffer *state)
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
   cceaLoadActionsPlugin();
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
   moveCellUID = cceNameToUID("mvcell");
   setCellTypeUID = cceNameToUID("stcell");
   createCellUID = cceNameToUID("mkcell");
   unlockControlsUID = cceNameToUID("ulctl");
   cceaRegisterAction(createCellUID, createCell, NULL, sizeof(uint32_t));
   cceaRegisterAction(moveCellUID, moveCell, NULL, sizeof(struct movecell_t));
   cceaRegisterAction(setCellTypeUID, setElementType, NULL, sizeof(struct setcelltype_t));
   cceaRegisterAction(unlockControlsUID, unlockControls, NULL, sizeof(uint32_t));
   {
      unsigned seed;
      cceGetRandomSeed(&seed, sizeof(unsigned));
      srand(seed);
      rand();
   }
   createCell(NULL, 1, g_map);
   while (cceEngineShouldTerminate() == 0)
   {
      cceRenderMap2D();
      if ((flags & (CONTROL_UPDATE | LOCK_CONTROLS)) == CONTROL_UPDATE)
      {
         calculateCellMove();
      }
      cceUpdate();
      cceaRunDelayedActions(g_map);
      cceScreenUpdate();
   }
   cceFreeMap2D(g_map);
   cceTerminate();
   return 0;
}
