#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <cce/os_interaction.h>

#ifndef CCE_API
#define CCE_API
#endif

#include <cce/plugins/actions.h>
#include <cce/engine_common.h>
#include <cce/plugins/map2D/map2D.h>

#define BOARD_SIZE 4
#define CONTROL_UPDATE 1

static uint8_t g_board[CCE_POW2(BOARD_SIZE)] = {0};
static struct cce_buffer *g_map;

uint8_t freeCells;
uint8_t flags;

struct movecell_t
{
   uint32_t ID;
   int8_t   xDiff;
   int8_t   yDiff;
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
   moveInfo.ID = 17;
   int8_t verticalAxis = (controlAxes.x == 0) ? controlAxes.y : 0;
   int8_t diff = ((int8_t)(verticalAxis | controlAxes.x) < 0) * 2 - 1;
   cce_void tmp[sizeof(struct cceRunActions) + 2 * sizeof(uint16_t) + sizeof(struct movecell_t) + 2 * sizeof(struct setcelltype_t)];
   struct cceRunActions *runActions = (struct cceRunActions*) &tmp;
   runActions->actionID = CCE_ACTION_RUNACTIONS;
   runActions->actionQuantity = 3;
   runActions->actionSizes[0] = sizeof(struct movecell_t);
   runActions->actionSizes[1] = sizeof(struct setcelltype_t);
   runActions->actionSizes[2] = sizeof(struct setcelltype_t);
   struct movecell_t *move = (struct movecell_t*) (tmp + sizeof(struct cceRunActions) + 2 * sizeof(uint16_t));
   move->ID = 17;
   struct setcelltype_t *setType1 = (struct setcelltype_t*)(move + 1);
   setType1->ID = 18;
   struct setcelltype_t *setType2 = (setType1 + 1);
   setType2->ID = 18;
   uint8_t maxMoveDuration = 0, isNewCell = 0;
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
               maxMoveDuration = CCE_MAX(maxMoveDuration, cells);
               moveInfo.cell = i;
               moveInfo.xDiff = 0;
               moveInfo.yDiff = -1;
               cceDelayAction(cells * 16, 6000, sizeof(struct movecell_t), &moveInfo, CCE_DEFAULT);
               move->cell = i;
               move->xDiff = 0;
               move->yDiff = 16 * cells;
               setType1->cell = i;
               setType1->type = 0;
               setType2->cell = i - cells * BOARD_SIZE;
               setType2->type = g_board[i] + isNewCell;
               g_board[i - cells * BOARD_SIZE] = (g_board[i] + isNewCell) | isNewCell << 7;
               g_board[i] = 0;
               cceDelayAction(1, 96000 * cells, sizeof(tmp), runActions, CCE_DEFAULT);
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
               maxMoveDuration = CCE_MAX(maxMoveDuration, cells);
               moveInfo.cell = i;
               moveInfo.xDiff = 0;
               moveInfo.yDiff = 1;
               cceDelayAction(cells * 16, 6000, sizeof(struct movecell_t), &moveInfo, CCE_DEFAULT);
               move->xDiff = 0;
               move->yDiff = -16 * cells;
               move->cell = i;
               setType1->cell = i;
               setType1->type = 0;
               setType2->cell = i + cells * BOARD_SIZE;
               setType2->type = g_board[i] + isNewCell;
               g_board[i + cells * BOARD_SIZE] = (g_board[i] + isNewCell) | isNewCell << 7;
               g_board[i] = 0;
               cceDelayAction(1, 96000 * cells, sizeof(tmp), runActions, CCE_DEFAULT);
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
                  maxMoveDuration = CCE_MAX(maxMoveDuration, cells);
                  moveInfo.cell = i;
                  moveInfo.xDiff = -1;
                  moveInfo.yDiff = 0;
                  cceDelayAction(cells * 16, 6000, sizeof(struct movecell_t), &moveInfo, CCE_DEFAULT);
                  move->cell = i;
                  move->xDiff = 16 * cells;
                  move->yDiff = 0;
                  setType1->cell = i;
                  setType1->type = 0;
                  setType2->cell = i - cells;
                  setType2->type = g_board[i] + isNewCell;
                  g_board[i - cells] = (g_board[i] + isNewCell) | isNewCell << 7;
                  g_board[i] = 0;
                  cceDelayAction(1, 96000 * cells, sizeof(tmp), runActions, CCE_DEFAULT);
                  freeCells += isNewCell;
               }
            }
         }
         break;
      case 3:
         for (unsigned i = CCE_POW2(BOARD_SIZE); i > 0; )
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
                  maxMoveDuration = CCE_MAX(maxMoveDuration, cells);
                  moveInfo.cell = i;
                  moveInfo.xDiff = 1;
                  moveInfo.yDiff = 0;
                  cceDelayAction(cells * 16, 6000, sizeof(struct movecell_t), &moveInfo, CCE_DEFAULT);
                  move->cell = i;
                  move->xDiff = -16 * cells;
                  move->yDiff = 0;
                  setType1->cell = i;
                  setType1->type = 0;
                  setType2->cell = i + cells;
                  setType2->type = g_board[i] + isNewCell;
                  g_board[i + cells] = (g_board[i] + isNewCell) | isNewCell << 7;
                  g_board[i] = 0;
                  cceDelayAction(1, 96000 * cells, sizeof(tmp), runActions, CCE_DEFAULT);
                  freeCells += isNewCell;
               }
            }
         }
         break;
   }
   if (maxMoveDuration > 0)
   {
      uint32_t action = 16;
      cceDelayAction(1, 96000 * maxMoveDuration, sizeof(uint32_t), &action, CCE_DEFAULT);
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
      {{-32, -32}, {0, 0}, {16, 16}, texture, 0, 0},
      {{-16, -32}, {0, 0}, {16, 16}, texture, 0, 0},
      {{  0, -32}, {0, 0}, {16, 16}, texture, 0, 0},
      {{ 16, -32}, {0, 0}, {16, 16}, texture, 0, 0},
      {{-32, -16}, {0, 0}, {16, 16}, texture, 0, 0},
      {{-16, -16}, {0, 0}, {16, 16}, texture, 0, 0},
      {{  0, -16}, {0, 0}, {16, 16}, texture, 0, 0},
      {{ 16, -16}, {0, 0}, {16, 16}, texture, 0, 0},
      {{-32,   0}, {0, 0}, {16, 16}, texture, 0, 0},
      {{-16,   0}, {0, 0}, {16, 16}, texture, 0, 0},
      {{  0,   0}, {0, 0}, {16, 16}, texture, 0, 0},
      {{ 16,   0}, {0, 0}, {16, 16}, texture, 0, 0},
      {{-32,  16}, {0, 0}, {16, 16}, texture, 0, 0},
      {{-16,  16}, {0, 0}, {16, 16}, texture, 0, 0},
      {{  0,  16}, {0, 0}, {16, 16}, texture, 0, 0},
      {{ 16,  16}, {0, 0}, {16, 16}, texture, 0, 0},
   };
   memcpy(cceGetElementsPosition(0, 0, 16, map), positions, 16 * sizeof(struct cce_elementposition));
   memcpy(cceGetElements(0, 16, map),            elements,  16 * sizeof(struct cce_element));
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
      element->data.texturePosition.x = 16 * (1 + type);
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
   position->position.x += vals->xDiff * count;
   position->position.y += vals->yDiff * count;
   cceSetElementsPositionsUpdated(cceGetElementPositionArray(0, g_map));
}

static void setElementType (const void *data, uint16_t count)
{
   CCE_UNUSED(count);
   const struct setcelltype_t *vals = data;
   struct cce_element *element = cceGetElements(vals->cell, 1, g_map);
   element->data.texturePosition.x = (vals->type % 4) * 16;
   element->data.texturePosition.y = (vals->type / 4) * 16;
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
      if (flags & CONTROL_UPDATE)
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
