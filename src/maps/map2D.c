#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <stdarg.h>

#include "../engine_common.h"
#include "../engine_common_internal.h"
#include "../shader.h"
#include "../log.h"
#include "../path_getters.h"
#include "../external/stb_image.h"
#include "map2D.h"
#include "map2D_internal.h"

#define BASIC_ACTIONS_QUANTITY 16u

static void (**cce_actions)(void*);

uint32_t                          g_texturesWidth;
uint32_t                          g_texturesHeight;
static struct LoadedTextures     *g_textures;
static uint16_t                   g_texturesQuantity;
static uint16_t                   g_texturesQuantityAllocated;
static GLuint                     glTexturesArray = 0u;
static uint16_t                   glTexturesArraySize = 0u;
static struct UsedUBO            *g_UBOs;
static uint16_t                   g_UBOsQuantity;
static uint16_t                   g_UBOsQuantityAllocated;
static GLint                      g_uniformBufferSize;
static const struct DynamicMap2D *g_dynamicMap;
static struct cce_ivec2           globalOffsetCoords = {0, 0};

static void (*cce_setUniformBufferToDefault)(GLuint, GLint);
static GLuint shaderProgram;
static GLint *bufferUniformsOffsets;
static GLint *uniformLocations;

static uint32_t actionsQuantity;

static char *texturesPath;

static cce_flag map2Dflags;

/* CBO - clear buffer object. Requires function glClearBufferSubData to present in openGL */
void setUniformBufferToDefault_withCBOext (GLuint UBO, GLint RotateAngleCosOffset)
{
   glBindBuffer(GL_UNIFORM_BUFFER, UBO);
   GL_CHECK_ERRORS;
   glClearBufferSubData(GL_UNIFORM_BUFFER, GL_R32F, 0u, g_uniformBufferSize, GL_RED, GL_FLOAT, NULL);
   GL_CHECK_ERRORS;
   float one = 1;
   glClearBufferSubData(GL_UNIFORM_BUFFER, GL_R32F, RotateAngleCosOffset, 255u * sizeof(float) , GL_RED, GL_FLOAT, &one);
   GL_CHECK_ERRORS;
}

static const float *g_ones;

static void setUniformBufferToDefault_withoutCBOext (GLuint UBO, GLint RotateAngleCosOffset)
{
   glBindBuffer(GL_UNIFORM_BUFFER, UBO);
   GL_CHECK_ERRORS;
   void *uboData = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
   GL_CHECK_ERRORS;
   memset(uboData, 0, g_uniformBufferSize);
   memcpy(uboData + RotateAngleCosOffset, g_ones, 255 * sizeof(float));
   glUnmapBuffer(GL_UNIFORM_BUFFER);
   GL_CHECK_ERRORS;
}

static inline void drawMap2D (struct Map2D *map)
{
   glBindVertexArray(map->VAO);
   GL_CHECK_ERRORS;
   glBindBufferRange(GL_UNIFORM_BUFFER, 1u, (g_UBOs + map->UBO_ID)->UBO, 0u, g_uniformBufferSize);
   GL_CHECK_ERRORS;
   glDrawArrays(GL_POINTS, 0u, map->elementsQuantity);
   GL_CHECK_ERRORS;
}

static void drawMap2Dmain (struct Map2Darray *maps)
{
   drawMap2D(maps->main);
}

static struct Map2D *lastNearestMap2D = NULL;

static void drawMap2Dnearest (struct Map2Darray *maps)
{
   drawMap2D(maps->main);
   drawMap2D(lastNearestMap2D);
}

static void drawMap2Dall (struct Map2Darray *maps)
{
   drawMap2D(maps->main);
   for (struct Map2D **iterator = maps->dependies, **end = maps->dependies + maps->main->exitMapsQuantity; iterator < end; ++iterator)
   {
      drawMap2D(*iterator);
   }
}

static void processLogicMap2Dmain (struct Map2Darray *maps)
{
   processLogicMap2D(maps->main);
}

static void processLogicMap2Dnearest (struct Map2Darray *maps)
{
   processLogicMap2D(maps->main);
   processLogicMap2D(lastNearestMap2D);
}

static void processLogicMap2Dall (struct Map2Darray *maps)
{
   processLogicMap2D(maps->main);
   for (struct Map2D **iterator = maps->dependies, **end = maps->dependies + maps->main->exitMapsQuantity; iterator < end; ++iterator)
   {
      processLogicMap2D((*iterator));
   }
}

static void (*drawMap2Dcommon) (struct Map2Darray*);
static void (*processLogicMap2Dcommon) (struct Map2Darray*);

void setFlags2D (cce_flag flags)
{
   switch (flags & (CCE_RENDER_ONLY_CURRENT_MAP | CCE_RENDER_CLOSEST_MAP | CCE_RENDER_ALL_LOADED_MAPS))
   {
      case CCE_RENDER_ONLY_CURRENT_MAP:
      {
         drawMap2Dcommon = drawMap2Dmain;
         break;
      }
      case CCE_RENDER_CLOSEST_MAP:
      {
         drawMap2Dcommon = drawMap2Dnearest;
         break;
      }
      case CCE_RENDER_ALL_LOADED_MAPS:
      {
         drawMap2Dcommon = drawMap2Dall;
         break;
      }
   }
   
   switch (flags & (CCE_PROCESS_LOGIC_ONLY_FOR_CURRENT_MAP | CCE_PROCESS_LOGIC_FOR_CLOSEST_MAP | CCE_PROCESS_LOGIC_FOR_ALL_MAPS))
   {
      case CCE_PROCESS_LOGIC_ONLY_FOR_CURRENT_MAP:
      {
         processLogicMap2Dcommon = processLogicMap2Dmain;
         break;
      }
      case CCE_PROCESS_LOGIC_FOR_CLOSEST_MAP:
      {
         processLogicMap2Dcommon = processLogicMap2Dnearest;
         break;
      }
      case CCE_PROCESS_LOGIC_FOR_ALL_MAPS:
      {
         processLogicMap2Dcommon = processLogicMap2Dall;
         break;
      }
   }
}

static GLuint createTextureArray (uint16_t newSize)
{
   GLuint texture;
   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
   glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, g_texturesWidth, g_texturesHeight, newSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   GL_CHECK_ERRORS;   
   
   glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   
   return texture;
}

uint8_t initEngine2D (uint16_t globalBoolsQuantity, uint32_t textureMaxWidth, uint32_t textureMaxHeight,
                      const char *windowLabel, const char *resourcePath)
{
   if (!resourcePath)
   {
      resourcePath = getenv("CCE_RESOURCE_PATH");
      if (!resourcePath)
      {
         perror("ENGINE::INIT::NO_RESOURCE_PATH:\nEngine could not load the game without knowing where it is");
         return 1;
      }
   }
   
   map2Dflags = CCE_INIT;
   if (initEngine(windowLabel, globalBoolsQuantity) != 0)
   {
      return 1;
   }
      
   {
      /*strlen("#define GLOBAL_OFFSET_CONTROL_MASK xxxxx\n") == 41 */
      char string[42] = "#define GLOBAL_OFFSET_CONTROL_MASK ";
      shortToString(string, CCE_GLOBAL_OFFSET_MASK, "\n");
      
      #ifdef SYSTEM_SHADER_PATH
      if (!(checkDirectoryExistance(SYSTEM_SHADER_PATH)))
      {
         shaderProgram = makeVGFshaderProgram(SYSTEM_SHADER_PATH "vertex_shader.glsl", SYSTEM_SHADER_PATH "geometry_shader.glsl", SYSTEM_SHADER_PATH "fragment_shader.glsl", 330u, string, "", "");
      }
      else
      #endif // SYSTEM_SHADER_PATH
      {
         char *path = createNewPathFromOldPath(resourcePath, "shaders", 0u);
         if (!checkDirectoryExistance(path))
         {
            char *vertexPath = createNewPathFromOldPath(path, "vertex_shader.glsl", 0u);
            char *geometryPath = createNewPathFromOldPath(path, "geometry_shader.glsl", 0u);
            char *fragmentPath = createNewPathFromOldPath(path, "fragment_shader.glsl", 0u);
            free(path);
            shaderProgram = makeVGFshaderProgram(vertexPath, geometryPath, fragmentPath, 330u, string, "", "");
            free(vertexPath);
            free(geometryPath);
            free(fragmentPath);
            
         }
         else
         {
            fprintf(stderr, "ENGINE::INIT::CANNOT_FIND_SHADERS:\nOpenGL shaders cannot be found at:\n%s", path);
            return 1;
         }
      }
   }
   if (!shaderProgram)
   {
      perror("ENGINE::INIT::SHADERS_CANNOT_BE_LOADED");
      return 1;
   }
   
   uniformLocations = malloc(2u * sizeof(GLint));
   *uniformLocations = glGetUniformLocation(shaderProgram, "Step");
   GL_CHECK_ERRORS;
   *(uniformLocations + 1u) = glGetUniformLocation(shaderProgram, "GlobalMoveCoords");
   GL_CHECK_ERRORS;
   {
      const GLchar *uniformNames[] = {"Colors", "MoveCoords", "Extention", "TextureOffset", "RotationOffset", "RotateAngleSin", "RotateAngleCos"};
      GLuint indices[7];
      glGetUniformIndices(shaderProgram, 7u, uniformNames, indices);
      GL_CHECK_ERRORS;
      bufferUniformsOffsets = (GLint*) malloc(7u * sizeof(GLint));
      glGetActiveUniformsiv(shaderProgram, 7, indices, GL_UNIFORM_OFFSET, bufferUniformsOffsets);
      GL_CHECK_ERRORS;
      glUniformBlockBinding(shaderProgram, glGetUniformBlockIndex(shaderProgram, "Variables"), 1u);
      GL_CHECK_ERRORS;
   }
   g_UBOsQuantityAllocated = CCE_ALLOCATION_STEP;
   g_UBOs = (struct UsedUBO*) malloc(g_UBOsQuantityAllocated * sizeof(struct UsedUBO));
   {
      GLint maxUniformOffset = 0;
      uint8_t i = 0, maxI;
      for (GLint *iterator = bufferUniformsOffsets, *end = bufferUniformsOffsets + 7u; iterator < end; ++iterator, ++i)
      {
         if (maxUniformOffset < (*iterator))
         {
            maxUniformOffset = (*iterator);
            maxI = i;
         }
      }
      switch (maxI)
      {
         case 0:
         {
            g_uniformBufferSize = maxUniformOffset + (4/*GLint and GLfloat*/ * 4/*vec4*/ * 255/*array*/);
            break;
         }
         case 1:
         case 2:
         case 3:
         case 4:
         {
            g_uniformBufferSize = maxUniformOffset + (4/*GLint and GLfloat*/ * 2/*vec2*/ * 255/*array*/);
            break;
         }
         case 5:
         case 6:
         {
            g_uniformBufferSize = maxUniformOffset + (4/*GLint and GLfloat*/ * 255/*array*/);
            break;
         }
      }
   }
   if (GLAD_GL_ARB_clear_buffer_object)
   {
      cce_setUniformBufferToDefault = setUniformBufferToDefault_withCBOext;
   }
   else
   {
      cce_setUniformBufferToDefault = setUniformBufferToDefault_withoutCBOext;
      float *ones = malloc(255u * sizeof(float));
      for (float *iterator = ones, *end = ones + 255u; iterator < end; ++iterator)
      {
         *iterator = 1.0f;
      }
      g_ones = ones;
   }
   
   for (struct UsedUBO *iterator = g_UBOs, *end = g_UBOs + g_UBOsQuantityAllocated; iterator < end; ++iterator)
   {
      glGenBuffers(1u, &(iterator->UBO));
      GL_CHECK_ERRORS;
      glBindBuffer(GL_UNIFORM_BUFFER, iterator->UBO);
      GL_CHECK_ERRORS;
      glBufferData(GL_UNIFORM_BUFFER, (g_uniformBufferSize), NULL, GL_DYNAMIC_DRAW);
      GL_CHECK_ERRORS;
      iterator->flags = 0u;
      cce_setUniformBufferToDefault(iterator->UBO, *(bufferUniformsOffsets + 6));
   }
   initMap2DLoaders(&cce_actions);
   {
      char *path = createNewPathFromOldPath(resourcePath, "maps", 0u);
      setMap2Dpath(path);
      free(path);
   }
   
   actionsQuantity = CCE_BASIC_ACTIONS_QUANTITY + CCE_ALLOCATION_STEP;
   cce_actions = (void (**)(void*)) calloc((actionsQuantity), sizeof(void (*)(void*)));
   g_texturesWidth = textureMaxWidth;
   g_texturesHeight = textureMaxHeight;
   g_textures = (struct LoadedTextures*) calloc(CCE_ALLOCATION_STEP, sizeof(struct LoadedTextures));
   g_textures->flags = 0x40u;
   g_texturesQuantity = 0u;
   g_texturesQuantityAllocated = CCE_ALLOCATION_STEP;
   glTexturesArray = createTextureArray(CCE_ALLOCATION_STEP);
   glTexturesArraySize = CCE_ALLOCATION_STEP;
   stbi_set_flip_vertically_on_load(1);
   texturesPath = createNewPathFromOldPath(resourcePath, "textures/img_", 9u);
   baseActionsInit(g_dynamicMap, g_UBOs, bufferUniformsOffsets, uniformLocations, shaderProgram, cce_setUniformBufferToDefault);
   g_dynamicMap = initDynamicMap2D();
   setFlags2D(CCE_DEFAULT);
   map2Dflags &= ~CCE_INIT;
   return 0u;
}

uint8_t registerAction (uint32_t ID, void (*action)(void*))
{
   if (ID >= actionsQuantity)
   {
      uint32_t lastActionsQuantity = actionsQuantity;
      actionsQuantity = (ID & (CCE_ALLOCATION_STEP - 1u)) + CCE_ALLOCATION_STEP;
      cce_actions = (void (**)(void*)) realloc(cce_actions, actionsQuantity * sizeof(void (*)(void*)));
      memset(cce_actions + lastActionsQuantity, 0u, actionsQuantity - lastActionsQuantity);
   }
   if ((ID < BASIC_ACTIONS_QUANTITY) != ((map2Dflags & CCE_BASIC_ACTIONS_NOT_SET) > 0u))
      return CCE_ATTEMPT_TO_OVERRIDE_DEFAULT_ELEMENT;
   *(cce_actions + ID) = action;
   return 0u;
}

void updateTexturesArray (void)
{
   int width, height, nrChannels;
   uint8_t arrayResized = 0u;
   cce_ubyte *data;
   uint16_t emptyLoadedTexturesInRow = 0u;
   GLuint textureArray;
   if (glTexturesArraySize < g_texturesQuantityAllocated)
   {
      textureArray = createTextureArray(g_texturesQuantityAllocated);
      glBindBuffer(GL_READ_BUFFER, glTexturesArray);
      GL_CHECK_ERRORS;
      arrayResized = 1u;
   }
   else
   {
      textureArray = glTexturesArray;
   }
   
   for (struct LoadedTextures *iterator = g_textures, *end = g_textures + g_texturesQuantity;; ++iterator)
   {
      if ((iterator >= end))
      {
         g_texturesQuantity -= emptyLoadedTexturesInRow;
         break;
      }
      if (iterator->dependantMapsQuantity)
      {
         emptyLoadedTexturesInRow = 0u;
         if ((iterator->flags & 0x80u))
         {
            shortToString(texturesPath, iterator->ID, ".png");
            data = stbi_load(texturesPath, &width, &height, &nrChannels, 4);
            if (!data)
            {
               data = stbi_load("./textures/img_dummy.png", &width, &height, &nrChannels, 4);
               if (!data)
               {
                  criticalErrorPrint("ENGINE::TEXTURE::DUMMY::FAILED_TO_LOAD:\nFailed to load dummy texture requested because %s was not found.", texturesPath);
               }
            }
            
            glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, g_texturesWidth - width, g_texturesHeight - height, (iterator - g_textures), width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
            GL_CHECK_ERRORS;
            stbi_image_free(data);
         }
         else if (arrayResized)
         {
            glCopyTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, 0, 0, 0, g_texturesWidth, g_texturesHeight);
            GL_CHECK_ERRORS;
         }
      }
      else 
      {
         iterator->ID = 0u;
         ++emptyLoadedTexturesInRow;
      }
   }
   if (arrayResized)
   {
      glDeleteTextures(1, &glTexturesArray);
      GL_CHECK_ERRORS;
      glTexturesArray = textureArray;
   }
   return;
}

void updateUBOarray ()
{
   for (struct UsedUBO *iterator = g_UBOs, *end = g_UBOs + g_UBOsQuantity; iterator < end; ++iterator)
   {
      if (iterator->flags & 0x2)
      {
         iterator->flags &= 0x1;
         cce_setUniformBufferToDefault(iterator->UBO, *(bufferUniformsOffsets + 6));
      }
   }
}

uint16_t getFreeUBO (void)
{
   map2Dflags |= CCE_PROCESS_UBO_ARRAY;
   for (struct UsedUBO *iterator = g_UBOs, *end = g_UBOs + g_UBOsQuantity; iterator < end; ++iterator)
   {
      if (iterator->flags & 0x1)
         continue;
      iterator->flags |= 0x1;
      return (uint16_t) (iterator - g_UBOs);
   }
   if (g_UBOsQuantity >= g_UBOsQuantityAllocated)
   {
      g_UBOsQuantityAllocated += CCE_ALLOCATION_STEP;
      g_UBOs = realloc(g_UBOs, g_UBOsQuantityAllocated * sizeof(struct UsedUBO));
      for (struct UsedUBO *iterator = g_UBOs + g_UBOsQuantityAllocated - CCE_ALLOCATION_STEP, *end = g_UBOs + g_UBOsQuantityAllocated; iterator < end; ++iterator)
      {
         glGenBuffers(1u, &(iterator->UBO));
         GL_CHECK_ERRORS;
         glBindBuffer(GL_UNIFORM_BUFFER, iterator->UBO);
         GL_CHECK_ERRORS;
         glBufferData(GL_UNIFORM_BUFFER, (g_uniformBufferSize), NULL, GL_DYNAMIC_DRAW);
         GL_CHECK_ERRORS;
         iterator->flags = 0u;
         cce_setUniformBufferToDefault(iterator->UBO, *(bufferUniformsOffsets + 6));
      }
   }
   struct UsedUBO *ubo = g_UBOs + g_UBOsQuantity;
   ubo->flags |= 0x1;
   return g_UBOsQuantity++;
}

void releaseUBO (uint16_t ID)
{
   (g_UBOs + ID)->flags = 0x2;
   return;
}

void releaseUnusedUBO (uint16_t ID)
{
   (g_UBOs + ID)->flags = 0x0;
   return;
}

static void extendLoadedTextures (uint16_t amount)
{
   g_texturesQuantityAllocated = (amount & ~(CCE_ALLOCATION_STEP - 1u)) + CCE_ALLOCATION_STEP;
   g_textures = realloc(g_textures, g_texturesQuantityAllocated * sizeof(struct LoadedTextures));
}

uint16_t loadTextureDynamicMap2D (struct DynamicMap2DElement *element)
{
   map2Dflags |= CCE_PROCESS_TEXTURES;
   uint16_t current_g_texture = 0u;
   for (;;)
   {
      if (current_g_texture < g_texturesQuantity)
      {
         extendLoadedTextures(CCE_ALLOCATION_STEP);
         ++g_texturesQuantity;
         break;
      }
      if ((g_textures + current_g_texture)->ID == element->textureInfo.ID)
      {
         ++((g_textures + current_g_texture)->dependantMapsQuantity);
         element->textureInfo.ID = current_g_texture;
         return current_g_texture;;
      }
      if ((g_textures + current_g_texture)->dependantMapsQuantity == 0u)
      {
         uint16_t i = current_g_texture;
         while (i < g_texturesQuantity)
         {
            if ((g_textures + i)->ID == element->textureInfo.ID)
            {
               ++((g_textures + i)->dependantMapsQuantity);
               element->textureInfo.ID = i;
               return i;
            }
            ++i;
         }
         break;
      }
      ++current_g_texture;
   }
   (g_textures + current_g_texture)->ID = element->textureInfo.ID;
   element->textureInfo.ID = current_g_texture;
   (g_textures + current_g_texture)->flags = 0x80u;
   (g_textures + current_g_texture)->dependantMapsQuantity = 1u;
   return current_g_texture;
}

uint16_t* loadTexturesMap2D (struct Map2DElement *elements, uint32_t elementsQuantity, uint16_t *texturesLoadedMapReliesOnQuantity)
{
   map2Dflags |= CCE_PROCESS_TEXTURES;
   uint32_t *texturesMapReliesOn = NULL;
   uint16_t  texturesMapReliesOnQuantity = 0u, texturesMapReliesOnAllocated = 0u;
   cce_ubyte isLoaded = 0u;
   for (struct Map2DElement *iterator = elements, *end = elements + elementsQuantity - 1u; iterator < end; ++iterator)
   {
      uint32_t ID = iterator->textureInfo.ID;
      if (ID == 0u) continue;
      for (uint32_t *jiterator = texturesMapReliesOn, *jend = texturesMapReliesOn + texturesMapReliesOnQuantity; jiterator < jend; ++jiterator)
      {
         if ((*jiterator) == ID)
         {
            isLoaded = 1u;
            break;
         }
      }
      if (!isLoaded)
      {
         if (texturesMapReliesOnQuantity == texturesMapReliesOnAllocated)
         {
            texturesMapReliesOnAllocated += CCE_ALLOCATION_STEP;
            texturesMapReliesOn = (uint32_t*) realloc(texturesMapReliesOn, texturesMapReliesOnAllocated * sizeof(uint32_t));
         }
         (*(texturesMapReliesOn + texturesMapReliesOnQuantity)) = ID;
         ++texturesMapReliesOnQuantity;
      }
   }
   uint16_t *texturesLoadedMapReliesOn = (uint16_t*) texturesMapReliesOn;
   uint16_t *literator = texturesLoadedMapReliesOn;
   uint16_t *end = texturesLoadedMapReliesOn + texturesMapReliesOnQuantity;
   uint32_t *jiterator, *kiterator = texturesMapReliesOn;
   uint32_t tmp;
   uint16_t *freeLoadedTextures = NULL;
   uint16_t freeLoadedTexturesQuantity = 0u, freeLoadedTexturesAllocated = 0u;
   isLoaded = 0u;
   for (uint16_t current_g_texture = 0u; current_g_texture < g_texturesQuantity; ++current_g_texture)
   {
      jiterator = kiterator;
      if ((g_textures + current_g_texture)->ID)
      {
         for (uint16_t *iterator = literator; iterator < end; ++iterator, ++jiterator)
         {
            if ((*jiterator) == ((g_textures + current_g_texture)->ID))
            {
               tmp = (*jiterator);
               (*jiterator) = (*kiterator);
               (*kiterator) = tmp;
               (*literator) = current_g_texture;
               ++((g_textures + current_g_texture)->dependantMapsQuantity);
               ++literator;
               ++kiterator;
               isLoaded = 1u;
               break;
            }
         }
         if ((!isLoaded) && (!(g_textures + current_g_texture)->dependantMapsQuantity) && ((end - literator) > freeLoadedTexturesQuantity))
         {
            if (freeLoadedTexturesQuantity == freeLoadedTexturesAllocated)
            {
               freeLoadedTexturesAllocated += CCE_ALLOCATION_STEP;
               freeLoadedTextures = (uint16_t *) realloc(freeLoadedTextures, freeLoadedTexturesAllocated * sizeof(uint16_t));
            }
            (*(freeLoadedTextures + freeLoadedTexturesQuantity)) = current_g_texture;
            ++freeLoadedTexturesQuantity;
         }
      }
      else if ((end - literator) > freeLoadedTexturesQuantity)
      {
         if (freeLoadedTexturesQuantity == freeLoadedTexturesAllocated)
         {
            freeLoadedTexturesAllocated += CCE_ALLOCATION_STEP;
            freeLoadedTextures = (uint16_t *) realloc(freeLoadedTextures, freeLoadedTexturesAllocated * sizeof(uint16_t));
         }
         (*(freeLoadedTextures + freeLoadedTexturesQuantity)) = current_g_texture;
         ++freeLoadedTexturesQuantity;
      }
   }
   uint16_t *iterator = freeLoadedTextures, *iend = freeLoadedTextures + freeLoadedTexturesQuantity;
   while ((literator < end) && (iterator < iend))
   {
      ((g_textures + (*iterator))->ID) = (*kiterator) + 1u; // 0u is invalid for openGL shaders, but perfectly fine here
      (*literator) = (*iterator);
      ((g_textures + (*iterator))->dependantMapsQuantity) = 1;
      ((g_textures + (*iterator))->flags) = 0x80;
      ++literator, ++kiterator, ++iterator;
   }
   free(freeLoadedTextures);
   uint16_t current_g_texture = g_texturesQuantity;
   while (literator < end)
   {
      if (current_g_texture >= g_texturesQuantity)
      {
         errorPrint("ENGINE::LOAD_TEXTURES_MAP_2D::NO_FREE_TEXTURE_SPACE_LEFT:\nCannot load texture because no free space left. \
                             Make bigger texture atlases or increase maxSimultaneouslyLoadedTexturesQuantity to avoid this.", NULL);
         while (literator < end)
         {
            (*literator) = 0u;
            ++literator;
         }
         break;
      }
      ((g_textures + current_g_texture)->ID) = (*kiterator);
      (*literator) = current_g_texture + 1u; // 0u is invalid for openGL shaders (it's the way to say "We don't need texture here"), but perfectly fine here
      ((g_textures + current_g_texture)->dependantMapsQuantity) = 1;
      ((g_textures + current_g_texture)->flags) = 0x80;
      ++literator, ++kiterator, ++current_g_texture;
   }
   *texturesLoadedMapReliesOnQuantity = texturesMapReliesOnQuantity;
   return (uint16_t*) realloc(texturesMapReliesOn, texturesMapReliesOnQuantity * sizeof(uint16_t));
}

void releaseTextures (uint16_t *texturesMapReliesOn, uint16_t texturesMapReliesOnQuantity)
{
   for (uint16_t *iterator = texturesMapReliesOn, *end = texturesMapReliesOn + texturesMapReliesOnQuantity; iterator < end; ++iterator)
   {
      --((g_textures + (*iterator))->dependantMapsQuantity);
   }
   free(texturesMapReliesOn);
   return;
}

cce_ubyte fourthLogicTypeFuncMap2D(uint16_t ID, va_list argp)
{
   struct Map2D *map = (struct Map2D*) va_arg(argp, struct Map2D*);
   uint32_t *group1IDs = ((map->collisionGroups + (map->collision + ID)->group1)->elementIDs);
   uint32_t *group1lastID = (group1IDs + (map->collisionGroups + (map->collision + ID)->group1)->elementsQuantity - 1u);
   uint32_t *group2firstID = ((map->collisionGroups + (map->collision + ID)->group2)->elementIDs);
   uint32_t *group2IDs;
   uint32_t *group2lastID = (group2firstID + (map->collisionGroups + (map->collision + ID)->group2)->elementsQuantity - 1u);
   struct Map2DCollider *element1, *element2;
   while (group1IDs <= group1lastID)
   {
      group2IDs = group2firstID;
      while (group2IDs <= group2lastID)
      {
         element1 = (map->colliders + *group1IDs);
         element2 = (map->colliders + *group2IDs);
         // ignore comparing with itself
         if ((group1IDs != group2IDs) && checkCollisionMap2D(element1, element2))
         {
            return 1u;
         }
         ++group2IDs;
      }
      ++group1IDs;
   }
   return 0u;
}

void swapMap2D (struct Map2D **a, struct Map2D **b)
{
   struct Map2D *tmp = (*a);
   (*a) = (*b);
   (*b) = tmp;
}

/* Manages dynamic memory! */
static struct Map2Darray* loadMap2DwithDependies (struct Map2Darray *maps, uint16_t number)
{
   if (!maps)
   {
      maps = (struct Map2Darray*) calloc(1u, sizeof(struct Map2Darray));
   }
   if (maps->main)
   {
      uint8_t oldExitMapsQuantity = maps->main->exitMapsQuantity;
      if (maps->main->ID != number)
      {
         if (maps->dependies)
         {
            for (struct Map2D **iterator = maps->dependies, **end = (maps->dependies + oldExitMapsQuantity - 1u);; ++iterator)
            {
               if (((*iterator)->ID) == number)
               {
                  swapMap2D(&(maps->main), iterator);
                  break;
               }
               if (iterator >= end)
               {
                  freeMap2D(maps->main);
                  maps->main = loadMap2D(number);
                  break;
               }
            }
         }
         else
         {
            freeMap2D(maps->main);
            maps->main = loadMap2D(number);
         }
      }
      if (!(maps->main->exitMapsQuantity))
      {
         if (maps->dependies)
         {
            for (struct Map2D **iterator = maps->dependies, **end = (maps->dependies + oldExitMapsQuantity - 1u); iterator <= end; ++iterator)
            {
               freeMap2D((*iterator));
            }
            free(maps->dependies);
            maps->dependies = NULL;
         }
         return maps;
      }
      struct Map2D **dependies = (struct Map2D**) malloc(maps->main->exitMapsQuantity * sizeof(struct Map2D*));
      struct Map2D **j = dependies;
      for (struct ExitMap2D *i = maps->main->exitMaps, *iend = (maps->main->exitMaps + maps->main->exitMapsQuantity - 1u); i <= iend; ++i, ++j)
      {
         for (struct Map2D **k = maps->dependies, **kend = (maps->dependies + oldExitMapsQuantity);; ++k)
         {
            if (!(*k)) continue;
            if ((*k)->ID == i->ID)
            {
               (*j) = (*k);
               (*k) = NULL;
               break;
            }
            if (k >= kend)
            {
               (*j) = loadMap2D(i->ID);
               break;
            }
         }
      }
      for (struct Map2D **iterator = maps->dependies, **end = (maps->dependies + oldExitMapsQuantity); iterator < end; ++iterator)
      {
         freeMap2D((*iterator));
      }
      free(maps->dependies);
      maps->dependies = dependies;
   }
   else
   {
      maps->main = loadMap2D(number);
      if (maps->dependies)
      {
         errorPrint("ENGINE::MAP2DARRAY_LOADER::DEPENDENCY_OF_NOTHING:\nMaps->dependies initialized without maps->main. Impossible to free maps->dependies. Possible memory leak", NULL);
      }
      maps->dependies = (struct Map2D**) malloc(maps->main->exitMapsQuantity * sizeof(struct Map2D*));
      struct ExitMap2D *exitmap = maps->main->exitMaps;
      for (struct Map2D **iterator = maps->dependies, **end = (maps->dependies + maps->main->exitMapsQuantity - 1u); iterator <= end; ++iterator, ++exitmap)
      {
         (*iterator) = loadMap2D(exitmap->ID);
      }
   }
   return maps;
}

static inline int getMapBorderDistance (struct ExitMap2D *borderInfo)
{
   int32_t globalOffsetA, globalOffsetB;
   if (borderInfo->flags & 1u)
   {
      globalOffsetA = globalOffsetCoords.x;
      globalOffsetB = globalOffsetCoords.y;
   }
   else
   {
      globalOffsetA = globalOffsetCoords.y;
      globalOffsetB = globalOffsetCoords.x;
   }
   if (borderInfo->b1Border < globalOffsetB && borderInfo->b2Border < globalOffsetB)
   {
      if (borderInfo->flags & 0x2)
      {
         return -(borderInfo->aBorder - globalOffsetA);
      }
      else
      {
         return borderInfo->aBorder - globalOffsetA;
      }
   }
   else
   {
      return INT32_MAX;
   }
}

static void terminateEngine2D (void)
{
   terminateDynamicMap2D();
   free(g_textures);
   glDeleteTextures(1, &glTexturesArray);
   for (struct UsedUBO *iterator = g_UBOs, *end = g_UBOs + g_UBOsQuantity; iterator < end; ++iterator)
   {
      glDeleteBuffers(1, &(iterator->UBO));
   }
   free(g_UBOs);
   free(bufferUniformsOffsets);
   free(uniformLocations);
   free(texturesPath);
   glDeleteProgram(shaderProgram);
   terminateEngine();
}

double timeFromLastFPSCheck = 0.0;
uint32_t frames = 0u;

int engine2D (void)
{
   if (map2Dflags & CCE_INIT)
      return -1;
   glUseProgram(shaderProgram);
   showWindow();
   engineUpdate();
   GL_CHECK_ERRORS;
   {
      struct cce_uvec2 aspectRatio = getCurrentStep();
      glUniform2f(*uniformLocations, aspectRatio.x, aspectRatio.y);
   }
   struct Map2Darray *maps = loadMap2DwithDependies(NULL, 0u);
   setCurrentArrayOfMaps(maps);
   uint32_t closestMapPosition;
   int32_t closestMapDistance = 0u, currentDistance;
   while (!(*cce__flags & CCE_ENGINE_STOP))
   {
      glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);
      GL_CHECK_ERRORS;
      if (maps->main->exitMapsQuantity)
      {
         closestMapDistance = INT32_MAX;
         for (struct ExitMap2D *iterator = maps->main->exitMaps, *end = maps->main->exitMaps + maps->main->exitMapsQuantity; iterator < end; ++iterator)
         {
            currentDistance = getMapBorderDistance(iterator);
            if (currentDistance < closestMapDistance)
            {
               closestMapDistance = currentDistance;
               closestMapPosition = iterator - maps->main->exitMaps;
            }
         }
         lastNearestMap2D = *(maps->dependies + closestMapPosition);
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
      glDrawArrays(GL_POINTS, 0u, g_dynamicMap->elementsQuantity);
      GL_CHECK_ERRORS;
      swapBuffers();
      engineUpdate();
      processLogicMap2Dcommon(maps);
      processLogicDynamicMap2D(g_dynamicMap, maps->main);
      
      if (closestMapDistance < 0)
      {
         maps = loadMap2DwithDependies(maps, (maps->main->exitMaps + closestMapPosition)->ID);
         setCurrentArrayOfMaps(maps);
      }
      ++frames;
      timeFromLastFPSCheck += *cce_deltaTime;
      if (timeFromLastFPSCheck >= 2.0)
      {
         printf("%lf FPS\n", frames / timeFromLastFPSCheck);
         frames = 0u;
         timeFromLastFPSCheck = 0.0;
      }
      processDynamicMap2DElements();
   }
   for (struct Map2D **iterator = maps->dependies, **end = maps->dependies + maps->main->exitMapsQuantity; iterator < end; ++iterator)
   {
      freeMap2D(*iterator);
   }
   freeMap2D(maps->main);
   free(maps->dependies);
   free(maps);
   terminateEngine2D();
   return 0;
}