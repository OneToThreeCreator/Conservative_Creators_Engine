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

void printMap2Ddev (struct Map2Ddev *map)
{
   printf("2-Dimentional Map with ID %u consists:\n", map->ID);
   printf(" %u elements, with %u ones without collider:\n", map->elementsQuantity, map->elementsWithoutColliderQuantity);
   for (struct Map2DElement *iterator = map->elements, *end = map->elements + map->elementsQuantity; iterator < end; ++iterator)
   {
      printf("  %lu: x: %d, y: %d, width: %u, height: %u, textureCoords: {sx: %f, sy: %f, ex: %f, ey: %f, ID: %u}, rotateGroup: %u, textureOffsetGroup: %u, colorGroup: %u\n",
      iterator - map->elements, iterator->x, iterator->y, iterator->width, iterator->height,
      iterator->textureInfo.startX, iterator->textureInfo.startY, iterator->textureInfo.endX, iterator->textureInfo.endY, iterator->textureInfo.ID,
      iterator->rotateGroup, iterator->textureOffsetGroup, iterator->colorGroup);
   }
   printf(" %u colliders:\n", map->elementsQuantity - map->elementsWithoutColliderQuantity + map->collidersQuantity);
   for (struct Map2DElement *iterator = map->elements + map->elementsWithoutColliderQuantity, *end = map->elements + map->elementsQuantity; iterator < end; ++iterator)
   {
      printf("  %lu: x: %d, y: %d, width: %u, height: %u\n",
      iterator - map->elements, iterator->x, iterator->y, iterator->width, iterator->height);
   }
   for (struct Map2DCollider *iterator = map->colliders, *end = map->colliders + map->collidersQuantity; iterator < end; ++iterator)
   {
      printf("  %lu: x: %d, y: %d, width: %u, height: %u\n",
      iterator - map->colliders + map->elementsQuantity - map->elementsWithoutColliderQuantity,
      iterator->x, iterator->y, iterator->width, iterator->height);
   }
   printf(" %u moveGroups:\n", map->moveGroupsQuantity);
   for (struct ElementGroup *group = map->moveGroups, *gend = map->moveGroups + map->moveGroupsQuantity; group < gend; ++group)
   {
      printf("  %lu: [", group - map->moveGroups);
      for (uint32_t *iterator = group->elementIDs, *end = group->elementIDs + group->elementsQuantity; iterator < end; ++iterator)
      {
         printf("%u, ", *iterator);
      }
      printf("\b\b] \n");
   }
   printf(" %u extensionGroups:\n", map->extensionGroupsQuantity);
   for (struct ElementGroup *group = map->extensionGroups, *gend = map->extensionGroups + map->extensionGroupsQuantity; group < gend; ++group)
   {
      printf("  %lu: [", group - map->extensionGroups);
      for (uint32_t *iterator = group->elementIDs, *end = group->elementIDs + group->elementsQuantity; iterator < end; ++iterator)
      {
         printf("%u, ", *iterator);
      }
      printf("\b\b] \n");
   }
   printf(" %u collisionGroups:\n", map->collisionGroupsQuantity);
   for (struct ElementGroup *group = map->collisionGroups, *gend = map->collisionGroups + map->collisionGroupsQuantity; group < gend; ++group)
   {
      printf("  %lu: [", group - map->collisionGroups);
      for (uint32_t *iterator = group->elementIDs, *end = group->elementIDs + group->elementsQuantity; iterator < end; ++iterator)
      {
         printf("%u, ", *iterator);
      }
      printf("\b\b] \n");
   }
   printf(" %u collisions:\n", map->collisionQuantity);
   for (struct CollisionGroup *iterator = map->collision, *end = map->collision + map->collisionQuantity; iterator < end; ++iterator)
   {
      printf("  %lu: has collision groups %u and %u colliding\n", iterator - map->collision, iterator->group1, iterator->group2);
   }
   printf(" %u timers", map->timersQuantity);
   for (float *iterator = map->delaysOfTimers, *end = map->delaysOfTimers + map->timersQuantity; iterator < end; ++iterator)
   {
      printf("  %lu: has delay %f\n", iterator - map->delaysOfTimers, *iterator);
   }
   printf(" %u logic\n", map->logicQuantity);
   for (struct ElementLogic *iterator = map->logic, *end = map->logic + map->logicQuantity; iterator < end; ++iterator)
   {
      printf("  %u logic has:\n   logic elements: [", iterator->logicElementsQuantity);
      for (uint16_t i = 0u, *element = iterator->logicElements, *eend = iterator->logicElements + iterator->logicElementsQuantity;
           element < eend; ++element, i += 2u)
      {
         char *type;
         switch ((iterator->elementType >> i) & 3u)
         {
            case 0u:
            {
               type = "bool";
               break;
            }
            case 1u:
            {
               type = "plotNumber >=";
               break;
            }
            case 2u:
            {
               type = "timer";
               break;
            }
            case 3u:
            {
               type = "collision";
               break;
            }
         }
         printf("%s %u, ", type, *element);
      }
      printf("\b\b] \n   operations table: {");
      uint_fast16_t *table = iterator->operations, *tend = iterator->operations + ((1 << iterator->logicElementsQuantity) / sizeof(uint_fast16_t));
      tend += (table == tend);
      while (table < tend)
      {
         printf("%lx", *table);
         ++table;
      }
      printf("}\n");
   }
   printf(" %u exitMaps:\n", map->exitMapsQuantity);
   for (struct ExitMap2D *iterator = map->exitMaps, *end = map->exitMaps + map->exitMapsQuantity; iterator < end; ++iterator)
   {
      printf("  %lu: is a line ", iterator - map->exitMaps);
      if (iterator->flags & 1u)
      {
         printf("with begin {x: %d, y: %d} and end {x: %d, y: %d}, ",
         iterator->aBorder, iterator->b1Border, iterator->aBorder, iterator->b2Border);
      }
      else
      {
         printf("with begin {x: %d, y: %d} and end {x: %d, y: %d}, ",
         iterator->b1Border, iterator->aBorder, iterator->b2Border, iterator->aBorder);
      }
      char arrow;
      switch (iterator->flags)
      {
         case 0u:
         {
            arrow = '>';
            break;
         }
         case 1u:
         {
            arrow = 'v';
            break;
         }
         case 2u:
         {
            arrow = '<';
            break;
         }
         case 3u:
         {
            arrow = '^';
            break;
         }
         default:
         {
            arrow = ' ';
            break;
         }
      }
      printf("that loads map %u at point {x: %d y: %d}, and has %c pass direction\n",
      iterator->ID, iterator->xOffset, iterator->yOffset, arrow);
   }
}

void printMap2D (struct Map2D *map)
{
   printf("2-Dimentional Map with ID %u consists:\n", map->ID);
   printf(" %u colliders:\n", map->collidersQuantity);
   for (struct Map2DCollider *iterator = map->colliders, *end = map->colliders + map->collidersQuantity; iterator < end; ++iterator)
   {
      printf("  %lu: x: %d, y: %d, width: %u, height: %u\n",
      iterator - map->colliders, iterator->x, iterator->y, iterator->width, iterator->height);
   }
   printf(" %u moveGroups:\n", map->moveGroupsQuantity);
   for (struct ElementGroup *group = map->moveGroups, *gend = map->moveGroups + map->moveGroupsQuantity; group < gend; ++group)
   {
      printf("  %lu: [", group - map->moveGroups);
      for (uint32_t *iterator = group->elementIDs, *end = group->elementIDs + group->elementsQuantity; iterator < end; ++iterator)
      {
         printf("%u, ", *iterator);
      }
      printf("\b\b] \n");
   }
   printf(" %u extensionGroups:\n", map->extensionGroupsQuantity);
   for (struct ElementGroup *group = map->extensionGroups, *gend = map->extensionGroups + map->extensionGroupsQuantity; group < gend; ++group)
   {
      printf("  %lu: [", group - map->extensionGroups);
      for (uint32_t *iterator = group->elementIDs, *end = group->elementIDs + group->elementsQuantity; iterator < end; ++iterator)
      {
         printf("%u, ", *iterator);
      }
      printf("\b\b] \n");
   }
   printf(" %u collisionGroups:\n", map->collisionGroupsQuantity);
   for (struct ElementGroup *group = map->collisionGroups, *gend = map->collisionGroups + map->collisionGroupsQuantity; group < gend; ++group)
   {
      printf("  %lu: [", group - map->collisionGroups);
      for (uint32_t *iterator = group->elementIDs, *end = group->elementIDs + group->elementsQuantity; iterator < end; ++iterator)
      {
         printf("%u, ", *iterator);
      }
      printf("\b\b] \n");
   }
   printf(" %u collisions:\n", map->collisionQuantity);
   for (struct CollisionGroup *iterator = map->collision, *end = map->collision + map->collisionQuantity; iterator < end; ++iterator)
   {
      printf("  %lu: has collision groups %u and %u colliding\n", iterator - map->collision, iterator->group1, iterator->group2);
   }
   printf(" %u timers", map->timersQuantity);
   for (struct Timer *iterator = map->timers, *end = map->timers + map->timersQuantity; iterator < end; ++iterator)
   {
      printf("  %lu: has delay %f\n", iterator - map->timers, iterator->delay);
   }
   printf(" %u logic\n", map->logicQuantity);
   for (struct ElementLogic *iterator = map->logic, *end = map->logic + map->logicQuantity; iterator < end; ++iterator)
   {
      printf("  %u logic has:\n   logic elements: [", iterator->logicElementsQuantity);
      for (uint16_t i = 0u, *element = iterator->logicElements, *eend = iterator->logicElements + iterator->logicElementsQuantity;
           element < eend; ++element, i += 2u)
      {
         char *type;
         switch ((iterator->elementType >> i) & 3u)
         {
            case 0u:
            {
               type = "bool";
               break;
            }
            case 1u:
            {
               type = "plotNumber >=";
               break;
            }
            case 2u:
            {
               type = "timer";
               break;
            }
            case 3u:
            {
               type = "collision";
               break;
            }
         }
         printf("%s %u, ", type, *element);
      }
      printf("\b\b] \n   operations table: {");
      uint_fast16_t *table = iterator->operations, *tend = iterator->operations + ((1 << iterator->logicElementsQuantity) / sizeof(uint_fast16_t));
      tend += (table == tend);
      while (table < tend)
      {
         printf("%lx", *table);
         ++table;
      }
      printf("}\n");
   }
   printf(" %u exitMaps:\n", map->exitMapsQuantity);
   for (struct ExitMap2D *iterator = map->exitMaps, *end = map->exitMaps + map->exitMapsQuantity; iterator < end; ++iterator)
   {
      printf("  %lu: is a line ", iterator - map->exitMaps);
      if (iterator->flags & 1u)
      {
         printf("with begin {x: %d, y: %d} and end {x: %d, y: %d}, ",
         iterator->aBorder, iterator->b1Border, iterator->aBorder, iterator->b2Border);
      }
      else
      {
         printf("with begin {x: %d, y: %d} and end {x: %d, y: %d}, ",
         iterator->b1Border, iterator->aBorder, iterator->b2Border, iterator->aBorder);
      }
      char arrow;
      switch (iterator->flags)
      {
         case 0u:
         {
            arrow = '>';
            break;
         }
         case 1u:
         {
            arrow = 'v';
            break;
         }
         case 2u:
         {
            arrow = '<';
            break;
         }
         case 3u:
         {
            arrow = '^';
            break;
         }
      }
      printf("that loads map %u at point {x: %d y: %d}, and has %c pass direction\n",
      iterator->ID, iterator->xOffset, iterator->yOffset, arrow);
   }
}
