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

#ifndef CCE_DUMMY_TEXTURE_PRIMARY_COLOR
#define CCE_DUMMY_TEXTURE_PRIMARY_COLOR 255,0,255
#endif // CCE_DUMMY_TEXTURE_PRIMARY_COLOR

#ifndef CCE_DUMMY_TEXTURE_SECONDARY_COLOR
#define CCE_DUMMY_TEXTURE_SECONDARY_COLOR 0,0,0
#endif // CCE_DUMMY_TEXTURE_SECONDARY_COLOR

#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../../../include/cce/engine_common.h"
#include "../../../include/cce/engine_common_IO.h"
#include "../../../include/cce/utils.h"
#include "../../../include/cce/os_interaction.h"

#include "../../external/stb_image.h"
#include "../../../include/cce/plugins/map2D/map2D.h"
#include "map2D_internal.h"

static struct cce_u16vec2               g_textureSize = {0, 0};
CCE_API const struct cce_u16vec2 *const cceTextureSize = &g_textureSize;
CCE_ARRAY(g_textures, static struct cce_loadedtextures, static uint16_t);
static uint16_t                         g_textureBufferSize;
CCE_ARRAY(g_texturesEmpty, static struct cce_loadedtextures*, static uint16_t);
static struct cce_layer                *g_renderingLayers;
static uint8_t                          g_renderingLayersQuantity;
static size_t                           g_renderingDataSize;
struct cce_rendereringfuns              cce__renderingFunctions;

static char  *texturesPath = NULL;
static size_t texturesPathLength = 0;

uint16_t cce__pixelsPerCoordinate;
uint8_t  cce__viewRotationAngle;
struct cce_i16vec2 cce__cameraPosition;

cce_flag cce__map2Dflags;
static void cce__updateTexturesArray (void);

CCE_API void cceSetPixelsPerCoordinate (uint16_t k)
{
   cce__pixelsPerCoordinate = k;
}

CCE_API void cceSetViewRotation (uint8_t normalizedAngle)
{
   cce__viewRotationAngle = normalizedAngle;
}

CCE_API void cceSetCameraPosition (struct cce_i16vec2 position)
{
   cce__cameraPosition = position;
}

static int loadCallback (void *data, const char *name, const char *value)
{
   CCE_UNUSED(data);
   char buf[24] = {0};
   strncpy(buf, name, 24);
   cceMemoryToLowercase(buf, 23);
   if (CCE_STREQ(buf, "renderinglayers") || CCE_STREQ(buf, "renderinglayersquantity"))
   {
      g_renderingLayersQuantity = atoi(value);
   }
   else if (CCE_STREQ(buf, "texsize") || CCE_STREQ(buf, "texturesize"))
   {
      g_textureSize = cceStringToU16Vec2(value);
   }
   else if (CCE_STREQ(buf, "texpath") || CCE_STREQ(buf, "texturepath"))
   {
      cceSetTexturesPath(value);
   }
   else if (CCE_STREQ(buf, "mapspath") || CCE_STREQ(buf, "mappath"))
   {
      cceSetMap2Dpath(value);
   }
   else if (CCE_STREQ(buf, "usefallbackmap") || CCE_STREQ(buf, "genfallbackmaponfailure") || CCE_STREQ(buf, "genmaponfailure"))
   {
      cce__map2Dflags &= ~CCE_RETURN_NULL_ON_MAP_LOADING_FAILURE;
      cce__map2Dflags |= ((cceStringToBool(value) - 1) & CCE_RETURN_NULL_ON_MAP_LOADING_FAILURE);
   }
   else if (CCE_STREQ(buf, "pxpercoord") || CCE_STREQ(buf, "pixelspercoordinate") || CCE_STREQ(buf, "pxpercell") || CCE_STREQ(buf, "pixelspercell"))
   {
      char *last;
      uint16_t px = strtoul(value, &last, 0);
      if (px == 0 && value == last)
      {
         fprintf(stderr, "%s is not a valid number of pixels per coordinate\n", value);
         return 0;
      }
      cce__pixelsPerCoordinate = px;
   }
   return 0;
}

CCE_API void cceSetRenderingLayerMap2D (uint8_t layer, uint8_t mapLayer, struct cce_buffer *map)
{
   assert(map != NULL);
   assert(map->loadingFunctionBlockID == cce__staticMapFunctionSet || map->loadingFunctionBlockID == cce__dynamicMapFunctionSet);
   if (layer >= g_renderingLayersQuantity)
      return;
   g_renderingLayers[layer].layersData = (struct cce_renderinginfo*)((cce_void*) map + cce__renderingInfoOffset);
   g_renderingLayers[layer].layer = mapLayer;
   g_renderingLayers[layer].flags = map->loadingFunctionBlockID == cce__dynamicMapFunctionSet;
}

CCE_API void cceRenderMap2D (void)
{
   if (cce__map2Dflags & CCE_LOADEDTEXTURES_TOBELOADED)
      cce__updateTexturesArray();
   cce__drawMap2D(g_renderingLayers, g_renderingLayersQuantity);
}

CCE_API void cceSetTexturesPath (const char *path)
{
   CCE_SET_PATH(texturesPath, texturesPathLength, path);
}

static int loadTexture (char *path, uint16_t position)
{
   unsigned int width, height;
   void *data;
   size_t length = 0;
   if (path[0] != '/'
   #ifdef WINDOWS_SYSTEM
   && path[0] != '\\' && path[1] != ':'
   #endif // WINDOWS_SYSTEM
   )
   {
      length = strlen(path);
      if (length <= 16)
      {
         memcpy(texturesPath + texturesPathLength, path, length + 1);
         path = texturesPath;
      }
      else
      {
         char *newPath = malloc(length + texturesPathLength + 1);
         memcpy(newPath, texturesPath, texturesPathLength);
         memcpy(newPath + texturesPathLength, path, length + 1);
         path = newPath;
      }
   }
   data = stbi_load(path, (int*) &width, (int*) &height, NULL, 4);
   if (!data)
   {
      fprintf(stderr, "ENGINE::TEXTURE::DECODING_ERROR:\n%s\nFile located at %s\n", stbi_failure_reason(), path);
      if (length >= 16)
         free(path);
      else if (length > 0)
         texturesPath[texturesPathLength] = '\0';
      return -1;
   }
   if (width > g_textureSize.x || height > g_textureSize.y)
   {
      fprintf(stderr, "ENGINE::TEXTURE::APPLYING_ERROR:\n%s is bigger then texture buffer allocated for it. The texture were truncated\n", path);
   }
   cce__loadTexture(data, width, height, position);
   stbi_image_free(data);
   (g_textures + position)->size.x = width;
   (g_textures + position)->size.y = height;
   if (length >= 16)
      free(path);
   else if (length > 0)
      texturesPath[texturesPathLength] = '\0';
   return 0;
}

static int setTextureAttributes (uint16_t ID)
{
   if (g_textures[ID].size.x != 0 && g_textures[ID].size.y != 0)
      return 0;
   int width = 0, height = 0, channels, result;
   char *path = g_textures[ID].path;
   size_t length = 0;
   if (*path != '/' 
   #ifdef WINDOWS_SYSTEM
       && *path != '\\'  && path[1] != ':'
   #endif // WINDOWS_SYSTEM
   )
   {
      length = strlen(path);
      if (length <= 16)
      {
         memcpy(texturesPath + texturesPathLength, path, length + 1);
         path = texturesPath;
      }
      else
      {
         char *newPath = malloc(length + texturesPathLength + 1);
         memcpy(newPath, texturesPath, texturesPathLength);
         memcpy(newPath + texturesPathLength, path, length + 1);
         path = newPath;
      }
   }
   result = stbi_info(g_textures[ID].path, &width, &height, &channels);
   *(texturesPath + texturesPathLength) = '\0';
   if (!width || !height)
   {
      width = g_textureSize.x;
      height = g_textureSize.y;
   }
   g_textures[ID].size = (struct cce_u16vec2){width, height};
   if (length >= 16)
      free(path);
   else if (length > 0)
      texturesPath[texturesPathLength] = '\0';
   return result;
}

CCE_API void* cceGenDummyTextureRGBA8 (uint16_t width, uint16_t height)
{
   struct cce_u8vec4 colors[2] = {{CCE_DUMMY_TEXTURE_SECONDARY_COLOR, 255}, {CCE_DUMMY_TEXTURE_PRIMARY_COLOR, 255}};
   struct cce_u8vec4 *image = malloc(width * height * sizeof(struct cce_u8vec4));
   uint8_t colorToUse = 0, flipEveryRow = !(width & 1);
   for (struct cce_u8vec4 *iterator = image, *rowEnd = image + width * height; iterator < rowEnd;)
   {
      for (struct cce_u8vec4 *columnEnd = iterator + width; iterator < columnEnd; ++iterator)
      {
         *iterator = colors[colorToUse ^= 1];
      }
      colorToUse ^= flipEveryRow;
   }
   return image;
}

static void cce__updateTexturesArray (void)
{
   {
      struct cce_loadedtextures *iterator = g_textures + g_texturesQuantity;
      do
      {
         --iterator;
      }
      while (iterator > g_textures && iterator->dependantMapsQuantity == 0u);
      size_t newTexturesQuantity = CCE_MAX(iterator - g_textures, 0);
      struct cce_loadedtextures **jiterator = g_texturesEmpty;
      for (struct cce_loadedtextures **end = g_texturesEmpty + g_texturesEmptyQuantity; jiterator < end && (size_t)(*jiterator - g_textures) >= (g_texturesQuantity - newTexturesQuantity); ++jiterator)
      {
         
      }
      size_t emptyTexturesIgnore = jiterator - g_texturesEmpty;
      g_texturesEmptyQuantity -= emptyTexturesIgnore;
      g_texturesQuantity -= newTexturesQuantity;
      if (emptyTexturesIgnore != 0)
      {
         memmove(g_texturesEmpty, g_texturesEmpty + emptyTexturesIgnore, g_texturesEmptyQuantity);
         CCE_REALLOC_ARRAY(g_texturesEmpty, g_texturesEmptyQuantity);
         CCE_REALLOC_ARRAY(g_textures, g_texturesQuantity);
      }
   }
   const uint8_t arrayResized = g_texturesAllocated >= g_textureBufferSize;
   if (arrayResized)
      cce__reallocateTextureArray(g_texturesAllocated);
   
   for (struct cce_loadedtextures *iterator = g_textures, *end = g_textures + g_texturesQuantity; iterator < end; ++iterator)
   {
      if (iterator->dependantMapsQuantity > 0u)
      {
         if ((iterator->flags & CCE_LOADEDTEXTURES_TOBELOADED))
         {
            if (loadTexture(iterator->path, iterator - g_textures) != 0)
            {
               void *data = cceGenDummyTextureRGBA8(g_textureSize.x, g_textureSize.y);
               cce__loadTexture(data, g_textureSize.x, g_textureSize.y, iterator - g_textures);
               free(data);
            }
            *(texturesPath + texturesPathLength) = '\0';
            iterator->flags &= ~CCE_LOADEDTEXTURES_TOBELOADED;
         }
         else if (arrayResized)
         {
            cce__moveTextureFromOldArray(iterator - g_textures);
         }
      }
      else if (arrayResized)
      {
         free(iterator->path);
         iterator->path = NULL;
      }
   }
   if (arrayResized)
   {
      cce__removeOldArray();
   }
   cce__map2Dflags &= ~CCE_LOADEDTEXTURES_TOBELOADED;
   return;
}

CCE_API uint16_t cceLoadTexture (char *path, uint8_t usersQuantity)
{
   assert(path != NULL);
   cce__map2Dflags |= CCE_LOADEDTEXTURES_TOBELOADED;
   for (struct cce_loadedtextures *iterator = g_textures, *end = g_textures + g_texturesQuantity; iterator < end; ++iterator)
   {
      if (iterator->dependantMapsQuantity == 0u)
      {
         continue;
      }
      if (strcmp(iterator->path, path) == 0)
      {
         iterator->dependantMapsQuantity += usersQuantity;
         return iterator - g_textures;
      }
   }
   struct cce_loadedtextures *current;
   if (g_texturesEmptyQuantity == 0)
   {
      CCE_REALLOC_ARRAY_ZEROED(g_textures, g_texturesQuantity + 1);
      current = g_textures + g_texturesQuantity++;
   }
   else
   {
      current = g_texturesEmpty[--g_texturesEmptyQuantity];
   }
   free(current->path);
   size_t pathLength = strlen(path);
   current->path = malloc((pathLength + 1) * sizeof(char));
   memcpy(current->path, path, pathLength + 1);
   current->dependantMapsQuantity = usersQuantity;
   current->flags = CCE_LOADEDTEXTURES_TOBELOADED;
   setTextureAttributes(current - g_textures);
   return current - g_textures + 1;
}

int stringCompare (const void *a, const void *b)
{
   return strcmp(*(char**)a, *(char**)b);
}

// Creates repeating paths when they are already loaded, then ignores them.
// Done this way because we need dependency order in texturesMapDependOn to exactly match the order of paths (used when elements load)
int cce__loadTextures (void *buffer, struct cce_buffer *info, char **paths)
{
   assert(buffer);
   CCE_UNUSED(info);
   cce__map2Dflags |= CCE_LOADEDTEXTURES_TOBELOADED;
   struct cce_usedtexinfo *data = buffer;
   size_t pathsLength;
   for (char **iterator = paths;; ++iterator)
   {
      if (*iterator != NULL)
         continue;
      
      pathsLength = iterator - paths;
      break;
   }
   char **path;
   data->texturesMapDependsOn = malloc(pathsLength * sizeof(uint16_t));
   data->texturesMapDependsOnQuantity = pathsLength;
   data->texturesMapDependsOnAllocated = pathsLength;
   uint16_t *depTextureIt = data->texturesMapDependsOn;
   for (struct cce_loadedtextures *iterator = g_textures, *end = g_textures + g_texturesQuantity; iterator < end; ++iterator)
   {
      if (iterator->dependantMapsQuantity == 0)
         continue;
      path = (char**) bsearch(&iterator->path, paths, pathsLength, sizeof(char*), stringCompare);
      if (strcmp(*path, iterator->path) == 0)
      {
         setTextureAttributes(iterator - g_textures);
         ++iterator->dependantMapsQuantity;
         depTextureIt[(path - paths)] = iterator - g_textures + 1;
         if (path > paths)
         {
            for (char **jiterator = path + 1, **jend = paths + pathsLength; jiterator < jend && *jiterator == *path; ++jiterator)
            {
               *jiterator = path[-1];
            }
            *path = path[-1];
         }
         else
         {
            uint16_t repeats = 1;
            for (char **jiterator = path + 1, **jend = paths + pathsLength; *jiterator == jiterator[-1]; ++jiterator, ++repeats)
            {
               if (jiterator >= jend)
               {
                  return 0;
               }
            }
            paths += repeats;
            depTextureIt += repeats;
            pathsLength -= repeats;
         }
      }
   }
   char **jiterator = paths, **jend = paths + pathsLength;
   size_t len;
   if (g_texturesEmptyQuantity > 0)
   {
      struct cce_loadedtextures **iterator = g_texturesEmpty + g_texturesEmptyQuantity, **end = g_texturesEmpty;
      do
      {
         --iterator;
         len = strlen(*jiterator);
         free((**iterator).path);
         (**iterator).path = malloc((len + 1) * sizeof(char));;
         memcpy((**iterator).path, *jiterator, len + 1);
         (**iterator).flags |= CCE_LOADEDTEXTURES_TOBELOADED;
         setTextureAttributes(*iterator - g_textures);
         (**iterator).dependantMapsQuantity = 1;
         *depTextureIt = *iterator - g_textures + 1;
         ++jiterator;
         ++depTextureIt;
         if (jiterator >= jend)
         {
            g_texturesEmptyQuantity = iterator - g_texturesEmpty;
            return 0;
         }
         if (*jiterator == jiterator[-1])
         {
            uint16_t repeats = 1;
            for (char **kiterator = jiterator + 1, **kend = paths + pathsLength; *kiterator == kiterator[-1]; ++kiterator, ++repeats)
            {
               if (kiterator >= kend)
               {
                  g_texturesEmptyQuantity = iterator - g_texturesEmpty;
                  return 0;
               }
            }
            jiterator += repeats;
            depTextureIt += repeats;
         }
      }
      while (iterator > end);
      g_texturesEmptyQuantity = 0;
   }
   for (uint16_t i = g_texturesQuantity; jiterator < jend; ++i, ++g_texturesQuantity)
   {
      if (g_texturesQuantity >= g_texturesAllocated)
         CCE_REALLOC_ARRAY_ZEROED(g_textures, g_texturesQuantity + 1);
         
      len = strlen(*jiterator);
      free(g_textures[i].path);
      g_textures[i].path = malloc((len + 1) * sizeof(char));
      memcpy(g_textures[i].path, *jiterator, len + 1);
      g_textures[i].flags |= CCE_LOADEDTEXTURES_TOBELOADED;
      g_textures[i].dependantMapsQuantity = 1;
      setTextureAttributes(i);
      *depTextureIt = i + 1;
      ++jiterator;
      ++depTextureIt;
      if (*jiterator == jiterator[-1])
      {
         uint16_t repeats = 1;
         for (char **kiterator = jiterator + 1, **end = paths + pathsLength; *kiterator == kiterator[-1]; ++kiterator, ++repeats)
         {
            if (kiterator >= end)
            {
               return 0;
            }
         }
         jiterator += repeats;
         depTextureIt += repeats;
      }
   }
   return 0;
}

void cce__createTextures (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   struct cce_usedtexinfo *data = buffer;
   data->texturesMapDependsOn          = NULL;
   data->texturesMapDependsOnQuantity  = 0;
   data->texturesMapDependsOnAllocated = 0;
}

int ptrcomp (const void *__a, const void *__b)
{
   const uint8_t *a = *(const uint8_t**)__a;
   const uint8_t *b = *(const uint8_t**)__b;
   return a - b;
}

void cce__releaseTextures (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   uint16_t textureID;
   struct cce_usedtexinfo *data = buffer;
   if (data->texturesMapDependsOnQuantity == 0)
      return;
   // Iteration from the end to increase likelyhood of freeing last textures first (potentially avoiding expensive move and decreasing it's cost)
   uint16_t *iterator = data->texturesMapDependsOn + data->texturesMapDependsOnQuantity, *end = data->texturesMapDependsOn;
   do
   {
      --iterator;
      textureID = *(iterator) - 1u;
      --(g_textures[textureID].dependantMapsQuantity);
      if (g_textures[textureID].dependantMapsQuantity == 0)
      {
         if (textureID == g_texturesQuantity - 1)
         {
            --g_texturesQuantity;
            continue;
         }
            
         if (g_texturesEmptyQuantity >= g_texturesEmptyAllocated)
            CCE_REALLOC_ARRAY(g_texturesEmpty, g_texturesEmptyQuantity + 1);
         void *tofind = g_textures + textureID - 1;
         struct cce_loadedtextures **pos = cceBinarySearchFirst(&tofind, g_texturesEmpty, g_texturesEmptyQuantity, sizeof(struct cce_loadedtextures*), ptrcomp);
         memmove(pos + 1, pos, g_texturesEmptyQuantity - (pos - g_texturesEmpty));
         *pos = g_textures + textureID;
      }
   }
   while (iterator > end);
   free(data->texturesMapDependsOn);
   return;
}

void cce__releaseTexture (uint16_t textureID)
{
   if (textureID == 0u)
      return;
   --textureID;
   if (--(g_textures[textureID].dependantMapsQuantity) == 0)
   {
      if (textureID == g_texturesQuantity - 1)
      {
         --g_texturesQuantity;
         return;
      }
      if (g_texturesEmptyQuantity >= g_texturesEmptyAllocated)
         CCE_REALLOC_ARRAY(g_texturesEmpty, g_texturesEmptyQuantity + 1);
      void *tofind = g_textures + textureID - 1;
      struct cce_loadedtextures **pos = cceBinarySearchFirst(&tofind, g_texturesEmpty, g_texturesEmptyQuantity, sizeof(struct cce_loadedtextures*), ptrcomp);
      memmove(pos + 1, pos, g_texturesEmptyQuantity - (pos - g_texturesEmpty));
      *pos = g_textures + textureID;
   }
   return;
}

int textureCompare (const void *_a, const void *_b)
{
   struct cce_loadedtextures *a = g_textures + *(uint16_t*)_a - 1;
   struct cce_loadedtextures *b = g_textures + *(uint16_t*)_b - 1;
   return strcmp(a->path, b->path);
}

char** cce__storeTextures (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   struct cce_usedtexinfo *data = buffer;
   qsort(data->texturesMapDependsOn, data->texturesMapDependsOnQuantity, sizeof(uint16_t), textureCompare);
   char **paths = malloc((data->texturesMapDependsOnQuantity + 1) * sizeof(char*));
   char **jiterator = paths;
   for (uint16_t *iterator = data->texturesMapDependsOn, *end = data->texturesMapDependsOn + data->texturesMapDependsOnQuantity; iterator < end; ++iterator, ++jiterator)
   {
      *jiterator = g_textures[*iterator - 1].path;
   }
   *jiterator = NULL;
   return paths;
}

int initMap2DRenderer__openGL (const struct cce_loadedtextures **textures);

static void terminateMap2D (void)
{
   cce__terminateMap2DRenderer();
   cce__terminateMap2DLoaders();
   for (struct cce_loadedtextures *it = g_textures, *end = g_textures + g_texturesAllocated; it < end; ++it)
   {
      free(it->path);
   }
   free(g_textures);
   free(texturesPath);
   free(g_renderingLayers);
   texturesPath = NULL;
   texturesPathLength = 0;
   g_textureSize = (struct cce_u16vec2){0, 0};
}

static int initMap2D (void *data)
{
   CCE_UNUSED(data);
   cce__map2Dflags = CCE_INIT;
   
   cce__initMap2DLoaders();
   if (initMap2DRenderer__openGL((const struct cce_loadedtextures**) &g_textures) != 0)
   {
      fputs("MAP2D::INIT::RENDERER_FAILURE:\nCan't initialize map2D without renderer\n", stderr);
      return -1;
   }
   cceRegisterMapCustomResourceCallback(cce__loadTextures, cce__releaseTextures, cce__createTextures, cce__storeTextures, sizeof(struct cce_usedtexinfo));
   g_renderingDataSize = cce__getRenderingDataSize();
   
   CCE_ALLOC_ARRAY_ZEROED(g_textures, 1);
   CCE_ALLOC_ARRAY(g_texturesEmpty, 1);
   g_textureBufferSize = 0;
   cce__map2Dflags &= ~CCE_INIT;
   g_renderingLayers = calloc(g_renderingLayersQuantity, sizeof(struct cce_layer));
   return 0;
}

CCE_API void cceLoadMap2Dplugin (void)
{
   cceRegisterPlugin("map2d", NULL, loadCallback, initMap2D, terminateMap2D, CCE_DEFAULT);
}
