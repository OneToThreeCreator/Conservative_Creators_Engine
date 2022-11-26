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

#include "../platform/platforms.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#ifdef POSIX_SYSTEM
#include <strings.h>
#else
#include <ctype.h>
#endif
#include <stdarg.h>


#include "../../include/cce/engine_common.h"
#include "../../include/cce/engine_common_IO.h"
#include "../../include/cce/utils.h"
#include "../../include/cce/os_interaction.h"

#include "../engine_common_internal.h"
#include "../external/stb_image.h"
#include "../../include/cce/map2D/map2D.h"
#include "../platform/engine_common_keyboard.h"
#include "map2D_internal.h"

static char                                 *cce__resourcePath;
static struct cce_u32vec2                    g_textureSize;
CCE_PUBLIC_OPTIONS const struct cce_u32vec2 *cceTextureSize = &g_textureSize;
CCE_ARRAY(g_textures, static struct LoadedTextures, static uint16_t);
static uint16_t g_textureBufferSize;
CCE_ARRAY(g_texturesEmpty, static struct LoadedTextures*, static uint16_t);
static struct RenderingData **g_layers;
static uint8_t                g_layersQuantity;
static ptrdiff_t g_renderingBufferOffset;
static size_t g_renderingDataSize;
struct TransformationValues cce__groupsCache;
struct TransformationValues cce__transformations;
struct RendereringFunctions cce__renderingFunctions;

static char *texturesPath = NULL;
static size_t texturesPathLength;

cce_flag cce__map2Dflags;

CCE_PUBLIC_OPTIONS void cceRenderingLayerSetMap2D (uint8_t layer, uint8_t mapLayer, struct cce_buffer *map)
{
   if (layer >= g_layersQuantity)
      return;
   g_layers[layer] = (struct RenderingData*)((cce_void*)((struct RenderingInfo*)((cce_void*) map + g_renderingBufferOffset))->data + mapLayer * g_renderingDataSize);
}

CCE_PUBLIC_OPTIONS void cceRenderMap2D (void)
{
   cce__drawMap2D(g_layers, g_layersQuantity);
}

CCE_PUBLIC_OPTIONS void cceUpdateEngineMap2D (void)
{
   cce__screenUpdate();
   cce__engineUpdate();
}

CCE_PUBLIC_OPTIONS const char* cceGetResourcePath (void)
{
   return cce__resourcePath;
}

CCE_PUBLIC_OPTIONS void cceSetTexturesPath (const char *path)
{
   free(texturesPath);
   texturesPath = cceCreateNewPathFromOldPath(path, "", CCE_PATH_RESERVED);
   texturesPathLength = strlen(texturesPath);
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
      fprintf(stderr, "ENGINE::TEXTURE::DECODING_ERROR:\n%s\n", stbi_failure_reason());
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

CCE_PUBLIC_OPTIONS void* cceGenDummyTextureRGBA8 (uint16_t width, uint16_t height)
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

void cce__updateTexturesArray (void)
{
   {
      struct LoadedTextures *iterator = g_textures + g_texturesQuantity;
      do
      {
         --iterator;
      }
      while (iterator > g_textures && iterator->dependantMapsQuantity == 0u);
      size_t newTexturesQuantity = iterator - g_textures;
      g_texturesEmptyQuantity -= g_texturesQuantity - newTexturesQuantity;
      memmove(g_texturesEmpty, g_texturesEmpty + (g_texturesQuantity - newTexturesQuantity), g_texturesEmptyQuantity);
      g_texturesQuantity = newTexturesQuantity;
      CCE_REALLOC_ARRAY(g_texturesEmpty, g_texturesEmptyQuantity);
      CCE_REALLOC_ARRAY(g_textures, g_texturesQuantity);
   }
   const uint8_t arrayResized = g_texturesAllocated >= g_textureBufferSize;
   if (arrayResized)
      cce__reallocateTextureArray(g_texturesAllocated);
   
   for (struct LoadedTextures *iterator = g_textures, *end = g_textures + g_texturesQuantity; iterator < end; ++iterator)
   {
      if (iterator->dependantMapsQuantity > 0u)
      {
         if ((iterator->flags & CCE_LOADEDTEXTURES_TOBELOADED))
         {
            if (loadTexture(iterator->path, iterator - g_textures) != 0)
            {
               cce__loadTexture(cceGenDummyTextureRGBA8(g_textureSize.x, g_textureSize.y), g_textureSize.x, g_textureSize.y, iterator - g_textures);
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
   return;
}

CCE_PUBLIC_OPTIONS uint16_t cceLoadTexture (char *path)
{
   for (struct LoadedTextures *iterator = g_textures, *end = g_textures + g_texturesQuantity; iterator < end; ++iterator)
   {
      if (iterator->dependantMapsQuantity == 0u)
      {
         continue;
      }
      if (strcmp(iterator->path, path) == 0)
      {
         ++iterator->dependantMapsQuantity;
         return iterator - g_textures;
      }
   }
   struct LoadedTextures *current;
   if (g_texturesEmpty == 0)
   {
      CCE_REALLOC_ARRAY_ZEROED(g_textures, g_texturesQuantity + 1);
      current = g_textures + g_texturesQuantity++;
   }
   else
   {
      current = g_texturesEmpty[--g_texturesEmptyQuantity];
      if (current->path != NULL)
         free(current->path);
   }
   size_t pathLength = strlen(path);
   current->path = malloc((pathLength + 1) * sizeof(char));
   memcpy(current->path, path, pathLength + 1);
   current->dependantMapsQuantity = 1u;
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
   CCE_UNUSED(info);
   struct UsedTexturesInfo *data = buffer;
   size_t pathsLength;
   for (char **iterator = paths;; ++iterator)
   {
      if (*iterator != NULL)
         continue;
      
      pathsLength = iterator - paths;
      break;
   }
   char **path;
   data->texturesMapDependsOn = malloc(pathsLength);
   data->texturesMapDependsOnQuantity = pathsLength;
   data->texturesMapDependsOnAllocated = pathsLength;
   uint16_t *depTextureIt = data->texturesMapDependsOn;
   for (struct LoadedTextures *iterator = g_textures, *end = g_textures + g_texturesQuantity; iterator < end; ++iterator)
   {
      if (iterator->dependantMapsQuantity == 0)
         continue;
      path = (char**) cceBinarySearchCMP(paths, pathsLength, sizeof(char*), stringCompare, iterator->path);
      if (strcmp(*path, iterator->path) == 0)
      {
         setTextureAttributes(iterator - g_textures);
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
   {
      struct LoadedTextures **iterator = g_texturesEmpty + g_texturesEmptyQuantity, **end = g_texturesEmpty;
      do
      {
         --iterator;
         len = strlen(*jiterator);
         (**iterator).path = malloc((len + 1) * sizeof(char));;
         memcpy((**iterator).path, *jiterator, len + 1);
         (**iterator).flags |= CCE_LOADEDTEXTURES_TOBELOADED;
         setTextureAttributes(*iterator - g_textures);
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
      g_textures[i].path = malloc((len + 1) * sizeof(char));
      memcpy(g_textures[i].path, *jiterator, len + 1);
      g_textures[i].flags |= CCE_LOADEDTEXTURES_TOBELOADED;
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
   struct UsedTexturesInfo *data = buffer;
   data->texturesMapDependsOn          = NULL;
   data->texturesMapDependsOnQuantity  = 0;
   data->texturesMapDependsOnAllocated = 0;
}

void cce__releaseTextures (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   uint16_t textureID;
   struct UsedTexturesInfo *data = buffer;
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
         struct LoadedTextures **pos = (struct LoadedTextures**) cceBinarySearchFirstDescending(g_texturesEmpty, g_texturesEmptyQuantity, sizeof(struct LoadedTextures*), sizeof(struct LoadedTextures*), (size_t)(g_textures + textureID - 1));
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
      struct LoadedTextures **pos = (struct LoadedTextures**) cceBinarySearchFirstDescending(g_texturesEmpty, g_texturesEmptyQuantity, sizeof(struct LoadedTextures*), sizeof(struct LoadedTextures*), (size_t)(g_textures + textureID - 1));
      memmove(pos + 1, pos, g_texturesEmptyQuantity - (pos - g_texturesEmpty));
      *pos = g_textures + textureID;
   }
   return;
}

int textureCompare (const void *_a, const void *_b)
{
   struct LoadedTextures *a = g_textures + *(uint16_t*)_a - 1;
   struct LoadedTextures *b = g_textures + *(uint16_t*)_b - 1;
   return strcmp(a->path, b->path);
}

char** cce__storeTextures (void *buffer, struct cce_buffer *info)
{
   CCE_UNUSED(info);
   struct UsedTexturesInfo *data = buffer;
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



/*
cce_ubyte cce__checkCollision (const uint32_t *group1firstID, uint16_t groups1quantity, const uint32_t *group2firstID, uint16_t groups2quantity,
                               const cce_void *elements1, size_t element1size, const cce_void *elements2, size_t element2size)
{
   cce_ubyte isDifferent = (elements1 != elements2);
   const uint32_t *group1IDs = group1firstID, *group2IDs, *groups1end = group1firstID + groups1quantity, *groups2end = group2firstID + groups2quantity;
   while (group1IDs < groups1end)
   {
      group2IDs = group2firstID;
      while (group2IDs < groups2end)
      {
         if (((*group1IDs != *group2IDs) || isDifferent) &&
         cceCheckCollisionMap2D((struct Map2DCollider*) (elements1 + (*group1IDs * element1size)), (struct Map2DCollider*) (elements2 + (*group2IDs * element2size))))
         {
            return 1u;
         }
         ++group2IDs;
      }
      ++group1IDs;
   }
   return 0u;
}

cce_ubyte cce__checkCollisionWithOffset (const uint32_t *group1firstID, uint16_t groups1quantity, const uint32_t *group2firstID, uint16_t groups2quantity,
                                         const cce_void *elements1, size_t element1size, const struct cce_i32vec2 *elements1offset,
                                         const cce_void *elements2, size_t element2size, const struct cce_i32vec2 *elements2offset)
{
   cce_ubyte isDifferent = (elements1 != elements2);
   const uint32_t *group1IDs = group1firstID, *group2IDs, *groups1end = group1firstID + groups1quantity, *groups2end = group2firstID + groups2quantity;
   while (group1IDs < groups1end)
   {
      group2IDs = group2firstID;
      while (group2IDs < groups2end)
      {
         struct cce_i32vec2 offset = {elements2offset->x - elements1offset->x, elements2offset->y - elements1offset->y};
         if (((*group1IDs != *group2IDs) || isDifferent) &&
         cceCheckCollisionMap2DWithOffset((struct Map2DCollider*) (elements1 + (*group1IDs * element1size)), (struct Map2DCollider*) (elements2 + (*group2IDs * element2size)), &offset))
         {
            return 1u;
         }
         ++group2IDs;
      }
      ++group1IDs;
   }
   return 0u;
}

cce_ubyte cce__fourthLogicTypeFuncMap2D(uint16_t ID, va_list argp)
{
   struct Map2D *map = (struct Map2D*) va_arg(argp, struct Map2D*);
   return cce__checkCollision((map->collisionGroups + (map->collision + ID)->group1)->elements, (map->collisionGroups + (map->collision + ID)->group1)->elementsQuantity,
                              (map->collisionGroups + (map->collision + ID)->group2)->elements, (map->collisionGroups + (map->collision + ID)->group2)->elementsQuantity,
                              (cce_void*) map->colliders, sizeof(struct Map2DCollider), (cce_void*) map->colliders, sizeof(struct Map2DCollider));
}
*/


struct iniValues
{
   struct cce_u16vec2 gameResolution;
   uint8_t layersQuantity;
   struct cce_u16vec2 textureSize;
   char *windowName;
   char *texturePath;
   char *mapPath;
   cce_enum colorFormat;
   cce_enum colliderType;
   uint8_t useFallbackMap;
   struct cce_u8vec2 horizontalAxis;
   struct cce_u8vec2 verticalAxis;
   uint8_t buttons[8]; // A, B, X, Y, LB, RB, LT, RT
};

#ifndef POSIX_SYSTEM
// s2 must be lowercase (easy to provide most of the time)
static int strcasecmp(const char *s1, const char *s2)
{
   int diff;
   for (; *s1 != '\0' && *s2 != '\0', ++s1, ++s2)
   {
      if ((diff = tolower(*s1) - *s2) != 0)
         return diff;
   }
   return 0;
}
#endif

#define CCE_STREQ(x,y) (memcmp(x, y, strlen(y) + 1) == 0)

static int iniHandler (void *data, const char *section, const char *name, const char *value)
{
   struct iniValues *vals = data;
   char buf[24] = {0};
   size_t len = CCE_MAX(strlen(name), 23);
   memcpy(buf, name, len + 1);
   cceMemoryToLowercase(buf, len);
   if (strcasecmp(section, "properties") == 0)
   {
      if (CCE_STREQ(buf, "gameres") || CCE_STREQ(buf, "res") || CCE_STREQ(buf, "gameresolution") || CCE_STREQ(buf, "resolution"))
      {
         vals->gameResolution = cceStringToU16Vec2(value);
      }
      else if (CCE_STREQ(buf, "layers") || CCE_STREQ(buf, "layersquantity"))
      {
         vals->layersQuantity = atoi(value);
      }
      else if (CCE_STREQ(buf, "texsize") || CCE_STREQ(buf, "texturesize"))
      {
         vals->textureSize = cceStringToU16Vec2(value);
      }
      else if (CCE_STREQ(buf, "windowname") || CCE_STREQ(buf, "name"))
      {
         size_t len = strlen(value);
         vals->windowName = malloc((len + 1) * sizeof(char));
         memcpy(vals->windowName, value, len + 1);
      }
      else if (CCE_STREQ(buf, "texpath") || CCE_STREQ(buf, "texturepath"))
      {
         size_t len = strlen(value);
         vals->texturePath = malloc((len + 1) * sizeof(char));
         memcpy(vals->texturePath, value, len + 1);
      }
      else if (CCE_STREQ(buf, "mapspath") || CCE_STREQ(buf, "mappath"))
      {
         size_t len = strlen(value);
         vals->mapPath = malloc((len + 1) * sizeof(char));
         memcpy(vals->mapPath, value, len + 1);
      }
      else if (CCE_STREQ(buf, "colorformat"))
      {
         strncpy(buf, value, 4);
         cceMemoryToLowercase(buf, 3);
         if (CCE_STREQ(buf, "rgb"))
         {
            vals->colorFormat = CCE_COLOR_RGB;
         }
         else if (CCE_STREQ(buf, "hsv"))
         {
            vals->colorFormat = CCE_COLOR_HSV;
         }
         else if (CCE_STREQ(buf, "hsl"))
         {
            vals->colorFormat = CCE_COLOR_HSL;
         }
         else if (CCE_STREQ(buf, "hcl") || CCE_STREQ(buf, "lch"))
         {
            vals->colorFormat = CCE_COLOR_HCL;
         }
      }
      else if (CCE_STREQ(buf, "collidertype") || CCE_STREQ(buf, "collider"))
      {
         strncpy(buf, value, 10);
         cceMemoryToLowercase(buf, 9);
         if (CCE_STREQ(buf, "rect") || CCE_STREQ(buf, "rectangle"))
         {
            vals->colliderType = CCE_RECTANGLE_COLLIDER;
         }
         else if (CCE_STREQ(buf, "cir") || CCE_STREQ(buf, "circle"))
         {
            vals->colliderType = CCE_CIRCLE_COLLIDER;
         }
      }
      else if (CCE_STREQ(buf, "usefallbackmap") || CCE_STREQ(buf, "genfallbackmaponfailure") || CCE_STREQ(buf, "genmaponfailure"))
      {
         vals->useFallbackMap = cceStringToBool(value);
      }
      return 0;
   }
   if (strcasecmp(section, "keybinds") == 0)
   {
      if (CCE_STREQ(buf, "horizontalmove") || CCE_STREQ(buf, "horizontalaxis") || CCE_STREQ(buf, "horizontal"))
      {
         vals->horizontalAxis = cceKeysFromString2(value);
      }
      else if (CCE_STREQ(buf, "verticalmove") || CCE_STREQ(buf, "verticalaxis") || CCE_STREQ(buf, "vertical"))
      {
         vals->verticalAxis = cceKeysFromString2(value);
      }
      else if (CCE_STREQ(buf, "buttona"))
   }
   return 0;
}

static void parseGameINI (const char *path)
{
   
}

int initMap2DRenderer__openGL (char *cce__resourcePath, const struct LoadedTextures **textures, struct RendereringFunctions *funcStruct);

CCE_PUBLIC_OPTIONS int cceInitEngine2D (const char *gameINIpath)
{
   {
      char *path = getenv("CCE_RESOURCE_PATH");
      if (path != NULL && *path != '\0')
         resourcePath = path;
   }
   if (resourcePath == NULL || *resourcePath == '\0')
   {
      fputs("ENGINE::INIT::NO_RESOURCE_PATH:\nEngine could not load the game without knowing where it is", stderr);
      return -1;
   }
   size_t pathLength = strlen(resourcePath) + 1u;
   cce__resourcePath = malloc((pathLength + 11u) * sizeof(char));
   memcpy(cce__resourcePath, resourcePath, pathLength);
   
   cce__map2Dflags = CCE_INIT;
   if (cce__initEngine(windowLabel) != 0)
   {
      free(cce__resourcePath);
      return -1;
   }
   cce__initMap2DLoaders();
   g_textureSize.x = textureMaxWidth;
   g_textureSize.y = textureMaxHeight;
   g_renderingBufferOffset = cceGetFunctionBufferOffset(1, cce__staticMapFunctionSet);
   if (initMap2DRenderer__openGL(cce__resourcePath, (const struct LoadedTextures**) &g_textures, &cce__renderingFunctions) != 0)
   {
      fputs("ENGINE::INIT::BACKEND_FAILURE:\nCan't initialize engine without backend\n", stderr);
      return -1;
   }
   g_renderingDataSize = cce__getRenderingDataSize();
   cceAppendPath(cce__resourcePath, pathLength + 11, "maps");
   cceSetMap2Dpath(cce__resourcePath);
   *(cce__resourcePath + pathLength) = '\0';
   
   CCE_ALLOC_ARRAY_ZEROED(g_textures);
   CCE_ALLOC_ARRAY(g_texturesEmpty);
   g_textureBufferSize = 0;
   cceAppendPath(cce__resourcePath, pathLength + 11, "textures");
   cceSetTexturesPath(resourcePath);
   *(cce__resourcePath + pathLength) = '\0';
   cce__actionsInit();
   cce__map2Dflags &= ~CCE_INIT;
   g_layers = calloc(layersQuantity, sizeof(struct cce_buffer*));
   g_layerZero = g_layers + layerZeroOffset;
   g_layersQuantity = layersQuantity;
   return 0;
}

void cce__terminateEngine2D (void)
{
   cce__terminateMap2DRenderer();
   free(g_textures);
   free(texturesPath);
   cce__actionsTerminate();
   cce__terminateEngine();
}

/*

CCE_PUBLIC_OPTIONS int cceEngine2D (void)
{
   if (cce__map2Dflags & CCE_INIT)
      return -1;
      
   cce__showWindow();
   cce__engineUpdate();
   while (!(*cce__flags & CCE_ENGINE_STOP))
   {
      cce__processDynamicMap2DElements();
      
      if (cce__map2Dflags & CCE_PROCESS_TEXTURES)
      {
         cce__updateTexturesArray();
         cce__map2Dflags &= ~CCE_PROCESS_TEXTURES;
      }

      if (cce__map2Dflags & CCE_PROCESS_UBO_ARRAY)
      {
         updateUBOarray();
         cce__map2Dflags &= ~CCE_PROCESS_UBO_ARRAY;
      }
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
      GL_CHECK_ERRORS;
      glBindTexture(GL_TEXTURE_2D_ARRAY, glTexturesArray);
      GL_CHECK_ERRORS;
      if (maps->main->exitMapsQuantity > 0)
      {
         if (cce__map2Dflags & CCE_PROCESS_NEAREST_MAPS)
         {
            cce__processNearestMap2D(maps->main->exitMaps, maps->main->exitMapsQuantity);
            cce__map2Dflags &= ~CCE_PROCESS_NEAREST_MAPS;
         }
         drawMap2Dcommon(maps);
      }
      else
      {
         drawMap2D(maps->main);
      }
      
      glBindVertexArray(g_dynamicMap->VAO);
      GL_CHECK_ERRORS;
      glBindBufferRange(GL_UNIFORM_BUFFER, 1u, (g_UBOs + g_dynamicMap->UBO_ID)->UBO, 0u, g_uniformBufferSize);
      GL_CHECK_ERRORS;
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_EBO);
      GL_CHECK_ERRORS;
      glDrawElements(GL_TRIANGLES, g_dynamicMap->elementsQuantity * 6, GL_UNSIGNED_INT, (void*) 0);
      GL_CHECK_ERRORS;
      cce__swapBuffers();
      cce__engineUpdate();
      processLogicMap2Dcommon(maps);
#ifndef NDEBUG
      {
         static uint16_t frames = 0;
         static float timePassed = 0.0f;
         timePassed += *cceDeltaTime;
         ++frames;
         if (timePassed > 2.0f)
         {
            printf("%f FPS\n", frames / timePassed);
            frames = 0;
            timePassed = 0.0f;
         }
      }
#endif
      if (cce__map2Dflags & CCE_PROCESS_LOADEDMAP2D)
      {
         cce__loadMapsAndSetState(maps);
         cce__map2Dflags &= ~CCE_PROCESS_LOADEDMAP2D;
         cce__map2Dflags |= CCE_PROCESS_NEAREST_MAPS;
      }
   }
   for (struct Map2D **iterator = maps->dependies, **end = maps->dependies + maps->main->exitMapsQuantity; iterator < end; ++iterator)
   {
      cceFreeMap2D(*iterator);
   }
   cceFreeMap2D(maps->main);
   free(maps->dependies);
   free(maps);
   cce__terminateEngine2D();
   return 0;
}
*/
