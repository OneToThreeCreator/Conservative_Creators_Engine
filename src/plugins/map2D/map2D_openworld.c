/*
    Conservative Creator's Engine - open source engine for making games.
    Copyright (C) 2020-2022 Andrey Gaivoronskiy

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
#include <string.h>
#include <stdlib.h>

#include <cce/engine_common_IO.h>
#include <cce/endianess.h>
#include <cce/utils.h>

#include <cce/map2D/actions.h>

#include "../../include/cce/plugins/openworld.h"

int loadOpenWorldData (void *buffer, uint8_t sectionSize, struct cce_buffer *info, FILE *file)
{
   CCE_UNUSED(info);
   struct OpenWorldInfo *map = buffer;
   uint32_t exitMapNamesSize = 0;
   fread(&map->exitMapsQuantity, sizeof(uint8_t),  sectionSize > 3, file);
   fread(&exitMapNamesSize,      sizeof(uint32_t), sectionSize > 4, file);
   exitMapNamesSize    = cceLittleEndianToHostEndianInt32(exitMapNamesSize);
   if (map->exitMapsQuantity == 0)
      return 0;
   uint32_t onEnterActionsSize = 0, onLeaveActionsSize = 0;
   if (sectionSize > 1)
      CCE__LOAD_ACTION_METADATA(map, file, map->onEnterActions, map->onLeaveActions, onEnterActionsSize, onLeaveActionsSize);
   map->exitMaps = malloc(map->exitMapsQuantity * sizeof(struct ExitMap2D) + exitMapNamesSize + onEnterActionsSize + onLeaveActionsSize);
   for (struct ExitMap2D *iterator = map->exitMaps, *end = map->exitMaps + map->exitMapsQuantity; iterator < end; ++iterator)
   {
      fread(&iterator->xOffset, sizeof(uint32_t), 5, file);
      fread(&iterator->flags, sizeof(uint8_t), 1, file);
      cceLittleEndianToHostEndianArrayInt32(&iterator->xOffset, 5);
   }
   char *exitMapNames = (char*)(map->exitMaps + map->exitMapsQuantity);
   fread(exitMapNames, sizeof(char), exitMapNamesSize, file);
   size_t size = 0;
   struct ExitMap2D *exitMaps = map->exitMaps;
   for (char *iterator = exitMapNames, *end = exitMapNames + exitMapNamesSize; iterator < end; iterator += size + 1, ++exitMaps)
   {
      exitMaps->mapName = iterator;
   }
   if (sectionSize > 1)
      CCE__LOAD_ACTIONS(map, file, map->onEnterActions, map->onLeaveActions, onEnterActionsSize, onLeaveActionsSize,
                        (struct Action*)(exitMapNames + exitMapNamesSize), (struct Action*)((cce_void*)map->onEnterActions + onEnterActionsSize),
                        (uint16_t*)((cce_void*)map->onLeaveActions + onLeaveActionsSize), (map->onEnterActionsSizes + map->onEnterActionsQuantity));
   return 0;
}

int loadOpenWorldDataDynamic (void *buffer, uint8_t sectionSize, struct cce_buffer *info, FILE *file)
{
   CCE_UNUSED(info);
   struct OpenWorldInfoDynamic *map = buffer;
   uint32_t exitMapNamesSize = 0;
   fread(&map->exitMapsQuantity, sizeof(uint8_t),  sectionSize > 3, file);
   fread(&exitMapNamesSize,      sizeof(uint32_t), sectionSize > 4, file);
   exitMapNamesSize    = cceLittleEndianToHostEndianInt32(exitMapNamesSize);
   if (map->exitMapsQuantity == 0)
      return 0;
   map->exitMaps = malloc(map->exitMapsQuantity * sizeof(struct ExitMap2D));
   for (struct ExitMap2D *iterator = map->exitMaps, *end = map->exitMaps + map->exitMapsQuantity; iterator < end; ++iterator)
   {
      fread(&iterator->xOffset, sizeof(uint32_t), 5, file);
      fread(&iterator->flags, sizeof(uint8_t), 1, file);
      cceLittleEndianToHostEndianArrayInt32(&iterator->xOffset, 5);
   }
   char *exitMapNames = malloc(exitMapNamesSize * sizeof(char));
   fread(exitMapNames, sizeof(char), exitMapNamesSize, file);
   size_t size = 0;
   struct ExitMap2D *exitMaps = map->exitMaps;
   for (char *iterator = exitMapNames, *end = exitMapNames + exitMapNamesSize; iterator < end; iterator += size + 1, ++exitMaps)
   {
      size = strlen(iterator);
      exitMaps->mapName = malloc((size + 1) * sizeof(char));
      memcpy(exitMaps->mapName, iterator, size + 1);
   }
   free(exitMapNames);
   map->exitMapsAllocated = map->exitMapsQuantity;
   return 0;
}

void freeOpenWorldData (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   struct OpenWorldInfo *map = buffer;
   free(map->exitMaps);
}

void freeOpenWorldDataDynamic (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   struct OpenWorldInfoDynamic *map = buffer;
   for (struct ExitMap2D *iterator = map->exitMaps, *end = map->exitMaps + map->exitMapsQuantity; iterator < end; ++iterator)
   {
      free(iterator->mapName);
   }
   free(map->exitMaps);
}

uint8_t storeOpenWorldData (void *buffer, struct cce_buffer *info, FILE *file)
{
   struct OpenWorldInfo *map = buffer;
   fwrite(&map->exitMapsQuantity, sizeof(uint8_t), 1, file);
   size_t section4 = ftell(file), fend;
   fseek(file, sizeof(uint32_t), SEEK_CUR);
   uint32_t temporary[5];
   for (struct ExitMap2D *iterator = map->exitMaps, *end = map->exitMaps + map->exitMapsQuantity; iterator < end; ++iterator)
   {
      cceLittleEndianToHostEndianNewArrayInt32(temporary, &iterator->xOffset, 5);
      fwrite(temporary, sizeof(uint32_t), 5, file);
      fwrite(&iterator->flags, sizeof(uint8_t), 1, file);
   }
   uint32_t bytesWritten = 0, size;
   for (struct ExitMap2D *iterator = map->exitMaps, *end = map->exitMaps + map->exitMapsQuantity; iterator < end; ++iterator)
   {
      size = strlen(iterator->mapName) + 1;
      fwrite(iterator->mapName, sizeof(char), size, file);
      bytesWritten += size;
   }
   fend = ftell(file);
   fseek(file, section4, SEEK_SET);
   fwrite(&bytesWritten, sizeof(uint32_t), 1, file);
   fseek(file, fend, SEEK_SET);
   return 1;
}
