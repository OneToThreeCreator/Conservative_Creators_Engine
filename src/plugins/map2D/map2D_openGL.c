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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include <glad/gl.h>
#include "../../shader.h"

#include "../../../include/cce/engine_common_IO.h"
#include "../../../include/cce/utils.h"
#include "../../../include/cce/os_interaction.h"

#include "../../engine_common_internal.h"
#include "map2D_internal.h"

#define CCE_CAMERATRANSFORM_OFFSET 0u
#define CCE_VIEWTRANSFORM_OFFSET 1u
#define CCE_TEXTUREOFFSET_OFFSET 2u

#define CCE_UPDATE_VIEW 0x1
#define CCE_UPDATE_CAMERA 0x2

struct cce_renderingdata
{
   GLuint   elementBuffer;
   GLuint   elementTexture;
   uint32_t elementsQuantity;
};

static const struct cce_loadedtextures **g_textures;
static GLuint                            glTexturesArray;
static GLuint                            glOldTexturesArray;
static GLuint                            glTemporaryFBO;
static GLuint                            shaderProgram;
static GLuint                            g_VAO, g_VBO;
static GLint                             g_uniformLocations[3];
static uint8_t                           g_rotationAngle;
static uint16_t                          g_pixelsPerCoordinate;
struct cce_i16vec2                       g_cameraPosition;

static void openGLErrorPrint (GLenum error, size_t line, const char *file)
{
   switch (error)
   {
      case GL_NO_ERROR: break;
      case GL_INVALID_ENUM:
      {
         fprintf(stderr, "%s: %zu: OPENGL::INVALID_ENUM:\nan unacceptable value is specified for an enumerated argument\n", file, line);
         break;
      }
      case GL_INVALID_VALUE:
      {
         fprintf(stderr, "%s: %zu: OPENGL::INVALID_VALUE:\na numeric argument is out of range\n", file, line);
         break;
      }
      case GL_INVALID_OPERATION:
      {
         fprintf(stderr, "%s: %zu: OPENGL::INVALID_OPERATION:\nthe specified operation is not allowed in the current state\n", file, line);
         break;
      }
      case GL_INVALID_FRAMEBUFFER_OPERATION:
      {
         fprintf(stderr, "%s: %zu: OPENGL::INVALID_OPERATION::FRAMEBUFFER:\nthe framebuffer object is not complete\n", file, line);
         break;
      }
      case GL_OUT_OF_MEMORY:
      {
         fprintf(stderr, "%s: %zu: OPENGL::OUT_OF_MEMORY:\nthere is not enough memory left to execute the command\n", file, line);
         break;
      }
      default:
      {
         fprintf(stderr, "%s: %zu: OPENGL::UNKNOWN:\n%d\n", file, line, error);
      }
   }
}

#ifdef NDEBUG
#define GL_CHECK_ERRORS
#else
#define GL_CHECK_ERRORS openGLErrorPrint(glGetError(), __LINE__, __FILE__)
#endif

static inline void updateView (void)
{
   g_pixelsPerCoordinate = cce__pixelsPerCoordinate;
   g_rotationAngle       = cce__viewRotationAngle;
   float matrix[3 * 3];
   matrix[0] = (g_pixelsPerCoordinate * 2.0f) / cce__gameResolution.x;
   matrix[4] = (g_pixelsPerCoordinate * 2.0f) / cce__gameResolution.y;
   matrix[8] = 1;
   float trigonometry = cceFastSinInt8(g_rotationAngle);
   matrix[1] =  trigonometry * matrix[0];
   matrix[3] = -trigonometry * matrix[4];
   trigonometry = cceFastCosInt8(g_rotationAngle);
   matrix[0] *= trigonometry;
   matrix[4] *= trigonometry;
   matrix[2] = 0;
   matrix[5] = 0;
   matrix[6] = 0;
   matrix[7] = 0;
   glUniformMatrix3fv(g_uniformLocations[CCE_VIEWTRANSFORM_OFFSET], 1, GL_FALSE, matrix);
}

static inline void updateCamera (void)
{
   g_cameraPosition = cce__cameraPosition;
   float matrix[3 * 3] = {0};
   matrix[0] = 1;
   matrix[2] = g_cameraPosition.x;
   matrix[4] = 1;
   matrix[5] = g_cameraPosition.y;
   matrix[8] = 1;
   glUniformMatrix3fv(g_uniformLocations[CCE_CAMERATRANSFORM_OFFSET], 1, GL_FALSE, matrix);
}

static size_t getRenderingDataSize__openGL (void)
{
   return sizeof(struct cce_renderingdata);
}

static GLuint createTextureArray (uint16_t newSize)
{
   GLuint texture;
   glGenTextures(1, &texture);
   GL_CHECK_ERRORS;
   glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
   GL_CHECK_ERRORS;
   glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, cceTextureSize->x, cceTextureSize->y, newSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   GL_CHECK_ERRORS;   
   
   glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
   GL_CHECK_ERRORS;
   glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
   GL_CHECK_ERRORS;
   glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   GL_CHECK_ERRORS;
   glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   GL_CHECK_ERRORS;
   
   return texture;
}

static void resizeTextureArrayBegin__openGL_copy_image_ext (uint16_t newSize)
{
   glOldTexturesArray = glTexturesArray;
   glTexturesArray = createTextureArray(newSize);
}

static void copyTextureToResizedArray__openGL_copy_image_ext (uint16_t texture)
{
   glCopyImageSubDataNV(glOldTexturesArray, GL_TEXTURE_2D_ARRAY, 0, 0, 0, texture,
                        glTexturesArray,    GL_TEXTURE_2D_ARRAY, 0, 0, 0, texture,
                        cceTextureSize->x,  cceTextureSize->y, 1);
   GL_CHECK_ERRORS;
}

static void resizeTextureArrayEnd__openGL_copy_image_ext (void)
{
   glDeleteTextures(1, &glOldTexturesArray);
   GL_CHECK_ERRORS;
}

static void resizeTextureArrayBegin__openGL_no_ext (uint16_t newSize)
{
   glTexturesArray = createTextureArray(newSize);
   glBindFramebuffer(GL_READ_FRAMEBUFFER, glTemporaryFBO);
   GL_CHECK_ERRORS;
   glBindTexture(GL_TEXTURE_2D_ARRAY, glTexturesArray);
   GL_CHECK_ERRORS;
}

static void copyTextureToResizedArray__openGL_no_ext (uint16_t texture)
{
   glFramebufferTextureLayer(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, glOldTexturesArray, 0, texture);
   GL_CHECK_ERRORS;
   glCopyTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, texture, 0, 0, cceTextureSize->x, cceTextureSize->y);
   GL_CHECK_ERRORS;
}

static void resizeTextureArrayEnd__openGL_no_ext (void)
{
   glDeleteTextures(1, &glOldTexturesArray);
   GL_CHECK_ERRORS;
   glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
   GL_CHECK_ERRORS;
}

/* Buffer MUST be bound to GL_TEXTURE_BUFFER!*/
#define UPDATE_LAYER(layer, mapFN) \
do \
{ \
   struct cce_i16vec4 *ITERATOR = mapFN; \
   GL_CHECK_ERRORS; \
   assert(ITERATOR != NULL); \
   struct cce_elementposition *position = layer->data; \
   for (struct cce_i16vec4 *END = ITERATOR + layer->dataQuantity; ITERATOR < END; ++ITERATOR, ++position) \
   { \
      ITERATOR->x = position->position.x; \
      ITERATOR->y = -position->position.y; \
      ITERATOR->z = (position->cce__reserved | (position->textureDataOffsetGroup << 8)) - (1 << (sizeof(uint16_t) * 8 - 1)); \
      ITERATOR->w = position->textureDataID - (1 << (sizeof(uint16_t) * 8 - 1)); /* We need to get it back as unsigned in glsl (simple reinterpret cast won't work - glsl doesn't have 16-bit types) */ \
   } \
} \
while (glUnmapBuffer(GL_TEXTURE_BUFFER) == GL_FALSE)

#define UPDATE_ELEMENTS(elements, elementsQuantity, mapFN) \
struct cce_u32vec4 *ITERATOR; \
do \
{ \
   ITERATOR = mapFN; \
   GL_CHECK_ERRORS; \
   memset(ITERATOR++, 0, sizeof(struct cce_u32vec4)); /* Zeroth element is always empty */ \
   uint16_t TEXTURE_POS_Y; \
   for (struct cce_u32vec4 *END = ITERATOR + elementsQuantity; ITERATOR < END; ++ITERATOR, ++elements) \
   { \
      if (elements->textureID == 0) /* Fragment has fixed color if no texture is applied */ \
      { \
         ITERATOR->x = (elements->data.rgba.x << 8) | elements->data.rgba.y | ((uint32_t)elements->position.x << 16); \
         ITERATOR->y = (elements->data.rgba.z << 8) | (elements->size.x & 0xFF) | ((uint32_t)(-elements->position.y - elements->size.y) << 16); \
         ITERATOR->w = elements->data.rgba.w | ((uint16_t)((int16_t)(cceFastCosInt8(elements->rotation + (-((elements->flags & CCE_ELEMENT_FLIP_VERTICALLY) > 0) & 128)) * INT16_MAX)) << 16) | \
                       (-(!(elements->flags & CCE_ELEMENT_IGNORE_CAMERA)) & 0x8000) | (-(((elements->flags & CCE_ELEMENT_FLIP_HORIZONTALLY) > 0) != ((elements->flags & CCE_ELEMENT_FLIP_VERTICALLY) > 0)) & 0x4000); \
      } \
      else \
      { \
         TEXTURE_POS_Y = cceTextureSize->y - elements->data.texturePosition.y - elements->size.y; /* Normally textures go from top to bottom. It is reversed by openGL. */ \
         ITERATOR->x = (elements->data.texturePosition.x & 0xFFF) | ((TEXTURE_POS_Y << 4) & 0xF000) | ((uint32_t)elements->position.x << 16); \
         ITERATOR->y = ((TEXTURE_POS_Y & 0xFF) << 8) | (elements->size.x & 0xFF) | ((uint32_t)(-elements->position.y - elements->size.y) << 16); \
         ITERATOR->w = ((elements->textureID + 255) & 0x3FFF) | ((uint16_t)((int16_t)(cceFastCosInt8(elements->rotation + (-((elements->flags & CCE_ELEMENT_FLIP_VERTICALLY) > 0) & 128)) * INT16_MAX)) << 16) | \
                       (-(!(elements->flags & CCE_ELEMENT_IGNORE_CAMERA)) & 0x8000) | (-(((elements->flags & CCE_ELEMENT_FLIP_HORIZONTALLY) > 0) != ((elements->flags & CCE_ELEMENT_FLIP_VERTICALLY) > 0)) & 0x4000); \
      } \
      ITERATOR->z = ((elements->size.x << 4) & 0xF000) | (elements->size.y & 0xFFF) | ((uint16_t)((int16_t)(cceFastSinInt8(elements->rotation + (-((elements->flags & CCE_ELEMENT_FLIP_VERTICALLY) > 0) & 128)) * INT16_MAX)) << 16); \
   } \
   elements -= elementsQuantity; \
} \
while (glUnmapBuffer(GL_TEXTURE_BUFFER) != GL_TRUE)

static struct cce_renderingdata* map2DElementsToRenderingBuffer__openGL (const struct cce_elementpositionarray *layers, uint8_t layersQuantity,
                                                                         const struct cce_element *elements, uint16_t elementsQuantity, uint16_t elementsAllocated)
{
   if (layersQuantity == 0 || elementsQuantity == 0)
      return NULL;
   GLuint elementsBuffers[256];
   GLuint textures[256];
   glGenBuffers(1 + layersQuantity, elementsBuffers);
   GL_CHECK_ERRORS;
   glGenTextures(1 + layersQuantity, textures);
   GL_CHECK_ERRORS;
   struct cce_renderingdata *data = malloc((layersQuantity + 1) * sizeof(struct cce_renderingdata)), *diterator = data + 1;
   GLuint *ebiterator = elementsBuffers + 1, *titerator = textures + 1;
   for (const struct cce_elementpositionarray *elementsEnd = layers + layersQuantity; layers < elementsEnd; ++layers, ++ebiterator, ++titerator, ++diterator)
   {
      diterator->elementsQuantity = CCE_MAX(layers->dataQuantity, layers->dataAllocated);
      glBindBuffer(GL_TEXTURE_BUFFER, *ebiterator);
      GL_CHECK_ERRORS;
      glBufferData(GL_TEXTURE_BUFFER, diterator->elementsQuantity * sizeof(struct cce_elementposition), NULL, GL_STATIC_DRAW);
      GL_CHECK_ERRORS;
      
      UPDATE_LAYER(layers, glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY));
      GL_CHECK_ERRORS;
      glBindTexture(GL_TEXTURE_BUFFER, *titerator);
      GL_CHECK_ERRORS;
      glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA16I, *ebiterator);
      GL_CHECK_ERRORS;
      diterator->elementBuffer = *ebiterator;
      diterator->elementTexture = *titerator;
      
   }
   data->elementBuffer = elementsBuffers[0];
   data->elementTexture = textures[0];
   data->elementsQuantity = elementsAllocated;
   glBindBuffer(GL_TEXTURE_BUFFER, elementsBuffers[0]);
   GL_CHECK_ERRORS;
   glBufferData(GL_TEXTURE_BUFFER, (elementsAllocated + 1) * 16, NULL, GL_DYNAMIC_DRAW);
   GL_CHECK_ERRORS;
   UPDATE_ELEMENTS(elements, elementsQuantity, glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY));
   GL_CHECK_ERRORS;
   glBindTexture(GL_TEXTURE_BUFFER, textures[0]);
   GL_CHECK_ERRORS;
   glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA32UI, elementsBuffers[0]);
   GL_CHECK_ERRORS;
   return data;
}

static void deleteMap2DRenderingBuffer__openGL (struct cce_renderingdata *data, uint8_t layersQuantity)
{
   if (data == NULL)
      return;
   GLuint buffers[256];
   GLuint textures[256];
   GLuint *jiterator = buffers;
   GLuint *kiterator = textures;
   for (struct cce_renderingdata *iterator = data, *end = data + layersQuantity + 1; iterator < end; ++iterator, ++jiterator, ++kiterator)
   {
      *jiterator = iterator->elementBuffer;
      *kiterator = iterator->elementTexture;
   }
   glDeleteBuffers(1 + layersQuantity, buffers);
   GL_CHECK_ERRORS;
   glDeleteTextures(1 + layersQuantity, buffers);
   GL_CHECK_ERRORS;
   free(data);
}

static void loadTexture__openGL (void *data, uint16_t width, uint16_t height, uint16_t textureID)
{
   struct cce_u8vec4 tmp[512];
   glBindTexture(GL_TEXTURE_2D_ARRAY, glTexturesArray);
   GL_CHECK_ERRORS;
   width = CCE_MIN(width, cceTextureSize->x);
   height = CCE_MIN(height, cceTextureSize->y);
   uint16_t widthRemainer = width & 511;
   for (struct cce_u8vec4 *iterator = data, *jiterator = iterator + (height - 1) * width; iterator <= jiterator; iterator += widthRemainer, jiterator -= width * 2 - widthRemainer)
   {
      for (struct cce_u8vec4 *end = iterator + width - widthRemainer; iterator < end; iterator += 512, jiterator -= 512)
      {
         memcpy(tmp,       iterator,  512 * sizeof(struct cce_u8vec4));
         memcpy(iterator,  jiterator, 512 * sizeof(struct cce_u8vec4));
         memcpy(jiterator, tmp,       512 * sizeof(struct cce_u8vec4));
      }
      memcpy(tmp,       iterator,  widthRemainer * sizeof(struct cce_u8vec4));
      memcpy(iterator,  jiterator, widthRemainer * sizeof(struct cce_u8vec4));
      memcpy(jiterator, tmp,       widthRemainer * sizeof(struct cce_u8vec4));
   }
   glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, cceTextureSize->y - height, textureID, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
   GL_CHECK_ERRORS;
}

static void drawMap2D__openGL (struct cce_layer *layers, uint32_t layersQuantity)
{
   if (g_pixelsPerCoordinate != cce__pixelsPerCoordinate || g_rotationAngle != cce__viewRotationAngle)
      updateView();
   if (g_cameraPosition.x != cce__cameraPosition.x || g_cameraPosition.y != cce__cameraPosition.y)
      updateCamera();
   glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
   GL_CHECK_ERRORS;
   glClear(GL_COLOR_BUFFER_BIT);
   GL_CHECK_ERRORS;
   glBindVertexArray(g_VAO);
   GL_CHECK_ERRORS;
   glActiveTexture(GL_TEXTURE0);
   GL_CHECK_ERRORS;
   glBindTexture(GL_TEXTURE_2D_ARRAY, glTexturesArray);
   GL_CHECK_ERRORS;
   for (struct cce_layer *iterator = layers, *end = layers + layersQuantity; iterator < end; ++iterator)
   {
      if (iterator->layersData == NULL)
         continue;
      struct cce_dynamicrenderinginfo *info = iterator->layersData;
      if (info->elementsQuantity == 0)
         continue;
      if (info->data == NULL)
      {
         info->data = map2DElementsToRenderingBuffer__openGL(info->positions, info->layersQuantity, info->elements, info->elementsQuantity, (iterator->flags & CCE_LAYER_DYNAMIC) ? info->elementsAllocated : info->elementsQuantity);
         info->flags &= ~CCE_ELEMENT_UPDATED;
         info->positions[iterator->layer].dataAllocated &= ~1;
         info->positions[iterator->layer].dataAllocated |= (info->positions->dataQuantity == 1 || info->positions->dataAllocated > 0x80000000);
      }
      else
      {
         if (info->flags & CCE_ELEMENT_UPDATED)
         {
            glBindBuffer(GL_TEXTURE_BUFFER, info->data[0].elementBuffer);
            GL_CHECK_ERRORS;
            if ((iterator->flags & CCE_LAYER_DYNAMIC) && info->elementsAllocated > info->data[0].elementsQuantity)
            {
               glBufferData(GL_TEXTURE_BUFFER, (info->elementsAllocated + 1) * sizeof(struct cce_element), NULL, GL_DYNAMIC_DRAW);
               UPDATE_ELEMENTS(info->elements, info->elementsQuantity, glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY));
               info->data[0].elementsQuantity = info->elementsAllocated;
            }
            else
            {
               // Invalidate buffer
               UPDATE_ELEMENTS(info->elements, info->elementsQuantity, glMapBufferRange(GL_TEXTURE_BUFFER, 0, (info->elementsQuantity + 1) * sizeof(struct cce_element), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
            }
            info->flags &= ~CCE_ELEMENT_UPDATED;
         }
         // Workaround!
         if ((info->positions[iterator->layer].dataAllocated & 1) == !(info->positions->dataQuantity == 1 || info->positions->dataAllocated > 0x80000000))
         {
            info->positions[iterator->layer].dataAllocated ^= 1;
            struct cce_elementpositionarray *layer = info->positions + iterator->layer;
            glBindBuffer(GL_TEXTURE_BUFFER, info->data[1 + iterator->layer].elementBuffer);
            GL_CHECK_ERRORS;
            if ((iterator->flags & CCE_LAYER_DYNAMIC) && layer->dataAllocated > info->data[1 + iterator->layer].elementsQuantity)
            {
               glBufferData(GL_TEXTURE_BUFFER, layer->dataAllocated * sizeof(struct cce_elementposition), NULL, GL_STATIC_DRAW);
               UPDATE_LAYER(layer, glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY));
               GL_CHECK_ERRORS;
               info->data[1 + iterator->layer].elementsQuantity = info->positions[iterator->layer].dataAllocated;
            }
            else
            {
               // Invalidate buffer
               UPDATE_LAYER(layer, glMapBufferRange(GL_TEXTURE_BUFFER, 0, layer->dataQuantity * sizeof(struct cce_elementposition), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT));
            }
         }
      }
      glActiveTexture(GL_TEXTURE1);
      GL_CHECK_ERRORS;
      glBindTexture(GL_TEXTURE_BUFFER, info->data[iterator->layer + 1].elementTexture);
      GL_CHECK_ERRORS;
      glActiveTexture(GL_TEXTURE2);
      GL_CHECK_ERRORS;
      glBindTexture(GL_TEXTURE_BUFFER, info->data->elementTexture);
      GL_CHECK_ERRORS;
      
      glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, info->positions[iterator->layer].dataQuantity);
      GL_CHECK_ERRORS;
   }
   glFlush();
   GL_CHECK_ERRORS;
}

void terminateMap2DRenderer__openGL (void)
{
   if (GLAD_GL_NV_copy_image == 0)
   {
      glDeleteFramebuffers(1, &glTemporaryFBO);
      GL_CHECK_ERRORS;
   }
   glDeleteBuffers(1, &g_VBO);
   GL_CHECK_ERRORS;
   glDeleteVertexArrays(1, &g_VAO);
   GL_CHECK_ERRORS;
   glDeleteTextures(1, &glTexturesArray);
   GL_CHECK_ERRORS;
   glDeleteProgram(shaderProgram);
   GL_CHECK_ERRORS;
}

int initMap2DRenderer__openGL (const struct cce_loadedtextures **textures)
{
   {
      /*strlen("const vec2 inverseTextureSize = vec2(0.XXXXXXXX, 0.XXXXXXXX);") == 61*/
      char vertexShaderAdditionalString[61 + 1] = "const vec2 inverseTextureSize = vec2(";
      sprintf(vertexShaderAdditionalString + 37, "%.8f, %.8f);", 1.0f / cceTextureSize->x, 1.0f / cceTextureSize->y);
      #ifdef SYSTEM_RESOURCE_PATH
      shaderProgram = cce__makeVFshaderProgram(SYSTEM_RESOURCE_PATH "shaders/map2D.vert", SYSTEM_RESOURCE_PATH "shaders/map2D.frag", vertexShaderAdditionalString, NULL);
      if (shaderProgram == 0u)
      #endif // SYSTEM_RESOURCE_PATH
      {
         shaderProgram = cce__makeVFshaderProgram("shaders/map2D.vert", "shaders/map2D.frag", vertexShaderAdditionalString, NULL);
      }
   }
   if (!shaderProgram)
   {
      fputs("MAP2D::RENDERER::SHADERS_CANNOT_BE_LOADED\n", stderr);
      return -1;
   }
   g_uniformLocations[0] = glGetUniformLocation(shaderProgram, "CameraTransform");
   GL_CHECK_ERRORS;
   g_uniformLocations[1] = glGetUniformLocation(shaderProgram, "ViewTransform");
   GL_CHECK_ERRORS;
   g_uniformLocations[2] = glGetUniformLocation(shaderProgram, "TextureOffset");
   GL_CHECK_ERRORS;
   glUseProgram(shaderProgram);
   GL_CHECK_ERRORS;
   glGenBuffers(1, &g_VBO);
   GL_CHECK_ERRORS;
   glEnable(GL_BLEND);
   GL_CHECK_ERRORS;
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   GL_CHECK_ERRORS;
   
   glGenVertexArrays(1, &g_VAO);
   GL_CHECK_ERRORS;
   glBindVertexArray(g_VAO);
   GL_CHECK_ERRORS;
   glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
   GL_CHECK_ERRORS;
   GLfloat square[4 * 2] = 
   {
      -0.5f, -0.5f,
       0.5f, -0.5f,
      -0.5f,  0.5f,
       0.5f,  0.5f,
   };
   glBufferData(GL_ARRAY_BUFFER, (sizeof(GLfloat) * 4 * 2), square, GL_STATIC_DRAW);
   GL_CHECK_ERRORS;
   {
      GLuint aCoordsLocation = glGetAttribLocation(shaderProgram, "aCoords");
      GL_CHECK_ERRORS;
      glVertexAttribPointer(aCoordsLocation, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*) 0);;
      GL_CHECK_ERRORS;
      glEnableVertexAttribArray(aCoordsLocation);
   }
   g_textures = textures;
   glTexturesArray = 0;
   g_rotationAngle = 0;
   g_cameraPosition = (struct cce_i16vec2){0, 0};
   g_pixelsPerCoordinate = 1;
   glUniform1i(glGetUniformLocation(shaderProgram, "Textures"), 0);
   GL_CHECK_ERRORS;
   glUniform1i(glGetUniformLocation(shaderProgram, "ElementInfo"), 1);
   GL_CHECK_ERRORS;
   glUniform1i(glGetUniformLocation(shaderProgram, "ElementData"), 2);
   GL_CHECK_ERRORS;
   cce__renderingFunctions.drawMap2D = drawMap2D__openGL;
   cce__renderingFunctions.map2DElementsToRenderingBuffer = map2DElementsToRenderingBuffer__openGL;
   cce__renderingFunctions.deleteMap2DRenderingBuffer = deleteMap2DRenderingBuffer__openGL;
   cce__renderingFunctions.loadTexture = loadTexture__openGL;
   cce__renderingFunctions.terminateMap2DRenderer = terminateMap2DRenderer__openGL;
   cce__renderingFunctions.getRenderingDataSize = getRenderingDataSize__openGL;
   if (GLAD_GL_NV_copy_image == 0)
   {
      glGenFramebuffers(1, &glTemporaryFBO);
      cce__renderingFunctions.reallocateTextureArray = resizeTextureArrayBegin__openGL_no_ext;
      cce__renderingFunctions.moveTextureFromOldArray = copyTextureToResizedArray__openGL_no_ext;
      cce__renderingFunctions.removeOldArray = resizeTextureArrayEnd__openGL_no_ext;
   }
   else
   {
      cce__renderingFunctions.reallocateTextureArray = resizeTextureArrayBegin__openGL_copy_image_ext;
      cce__renderingFunctions.moveTextureFromOldArray = copyTextureToResizedArray__openGL_copy_image_ext;
      cce__renderingFunctions.removeOldArray = resizeTextureArrayEnd__openGL_copy_image_ext;
   }
   updateView();
   updateCamera();
   return 0;
}
