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
#include <stdlib.h>

#include <glad/gl.h>
#include "../shader.h"

#include "../external/stb_image.h"

#include "../../include/cce/engine_common_IO.h"
#include "../../include/cce/utils.h"
#include "../../include/cce/os_interaction.h"

#include "map2D_internal.h"

#define CCE_TRANSFORMGROUP_OFFSET 0u
#define CCE_OFFSET_OFFSET 1u
#define CCE_COLORGROUP_OFFSET 2u
#define CCE_TEXTUREOFFSET_OFFSET 3u

#define CCE_GLOBALTRANSFORM_OFFSET 0u
#define CCE_VIEWMATRIX_OFFSET 1u

struct RenderingData
{
   GLuint elementBuffer;
   GLuint elementTexture;
   GLuint metaDataBuffer;
   GLuint metaDataTexture;
   uint32_t elementBufferSize;
   uint16_t textureInfoQuantity;
};

static const struct LoadedTextures **g_textures;
static GLuint   glTexturesArray;
static GLuint   glOldTexturesArray;
static GLuint   glTemporaryFBO;
static GLuint   shaderProgram;
static GLuint   g_VAO, g_VBO;
static GLint   *uniformLocations;

static void cce__openGLErrorPrint (GLenum error, size_t line, const char *file)
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
/*    case GL_STACK_UNDERFLOW:
      {
         fprintf(stderr, "OPENGL::STACK::OVERFLOW:\nan attempt has been made to perform an operation that would cause an internal stack to underflow\n");
         break;
      }
      case GL_STACK_OVERFLOW:
      {
         fprintf(stderr, "OPENGL::STACK::OVERFLOW:\nan attempt has been made to perform an operation that would cause an internal stack to overflow\n");
         break;
      }*/
      default:
      {
         fprintf(stderr, "%s: %zu: OPENGL::UNKNOWN:\n%d\n", file, line, error);
      }
   }
}

#ifdef NDEBUG
#define GL_CHECK_ERRORS
#else
#define GL_CHECK_ERRORS cce__openGLErrorPrint(glGetError(), __LINE__, __FILE__)
#endif

static size_t getRenderingDataSize__openGL (void)
{
   return sizeof(struct RenderingData);
}

static void drawMap2D__openGL (struct RenderingData **maps, uint32_t mapsQuantity)
{
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
   for (struct RenderingData **iterator = maps, **end = maps + mapsQuantity; iterator < end; ++iterator)
   {
      if (*iterator == NULL)
         continue;
      glActiveTexture(GL_TEXTURE1);
      GL_CHECK_ERRORS;
      glBindTexture(GL_TEXTURE_BUFFER, (**iterator).elementTexture);
      GL_CHECK_ERRORS;
      glActiveTexture(GL_TEXTURE2);
      GL_CHECK_ERRORS;
      glBindTexture(GL_TEXTURE_BUFFER, (**iterator).metaDataTexture);
      GL_CHECK_ERRORS;
      
      glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (**iterator).elementBufferSize);
      GL_CHECK_ERRORS;
   }
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

static struct RenderingData* map2DElementsToRenderingBuffer__openGL (const struct Map2DElementPositionArray *elements, uint8_t layersQuantity,
                                                                     const struct ElementInfo *info, uint16_t textureInfoQuantity)
{
   GLuint elementsBuffers[256];
   GLuint textures[256];
   glGenBuffers(1 + layersQuantity, elementsBuffers);
   GL_CHECK_ERRORS;
   glGenTextures(1 + layersQuantity, textures);
   GL_CHECK_ERRORS;
   struct RenderingData *data = malloc(layersQuantity * sizeof(struct RenderingData)), *diterator = data;
   GLuint *ebiterator = elementsBuffers + 1, *titerator = textures + 1;
   for (const struct Map2DElementPositionArray *elementsEnd = elements + layersQuantity; elements < elementsEnd; ++elements, ++ebiterator, ++titerator, ++diterator)
   {
      glBindBuffer(GL_TEXTURE_BUFFER, *ebiterator);
      GL_CHECK_ERRORS;
      glBufferData(GL_TEXTURE_BUFFER, elements->dataQuantity * sizeof(struct Map2DElementPosition), NULL, GL_DYNAMIC_DRAW); // Expected to be modified
      GL_CHECK_ERRORS;
      do
      {
         struct cce_i16vec4 *iterator = glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY);
         struct Map2DElementPosition *element = elements->data;
         for (struct cce_i16vec4 *end = iterator + elements->dataQuantity; iterator < end; ++iterator, ++element)
         {
            iterator->x = element->position.x;
            iterator->y = element->position.y;
            iterator->z = (element->rotation | (element->textureDataOffsetGroup << 8)) - (1 << (sizeof(uint16_t) * 8 - 1));
            iterator->w = element->textureDataID - (1 << (sizeof(uint16_t) * 8 - 1)); // We need to get it back as unsigned in glsl (simple reinterpret cast won't work - glsl doesn't have 16-bit types)
         }
      }
      while (glUnmapBuffer(GL_TEXTURE_BUFFER) == GL_FALSE);
      glBindTexture(GL_TEXTURE_BUFFER, *titerator);
      GL_CHECK_ERRORS;
      glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA16I, *ebiterator);
      GL_CHECK_ERRORS;
      diterator->elementBufferSize = elements->dataQuantity;
      diterator->textureInfoQuantity = textureInfoQuantity;
      diterator->elementBuffer = *ebiterator;
      diterator->elementTexture = *titerator;
      diterator->metaDataBuffer = elementsBuffers[0];
      diterator->metaDataTexture = textures[0];
   }
   
   glBindBuffer(GL_TEXTURE_BUFFER, elementsBuffers[0]);
   GL_CHECK_ERRORS;
   glBufferData(GL_TEXTURE_BUFFER, (textureInfoQuantity) * 8, NULL, GL_STATIC_DRAW); // Rarely modified
   GL_CHECK_ERRORS;
   struct cce_u16vec4 *iterator;
   if (textureInfoQuantity > 0)
   {
      do
      {
         iterator = glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY);
         GL_CHECK_ERRORS;
         for (struct cce_u16vec4 *end = iterator + textureInfoQuantity; iterator < end; ++iterator, ++info)
         {
            if (info->textureID == 0) // Fragment has fixed color if no texture is applied
            {
               iterator->x = (info->data.rgba.x << 8) | info->data.rgba.y;
               iterator->y = (info->data.rgba.z << 8) | (info->size.x & 0xFF);
               iterator->w = info->data.rgba.w;
            }
            else
            {
               iterator->x = (info->data.texturePosition.x & 0xFFF) | ((info->data.texturePosition.y << 4) & 0xF000);
               iterator->y = ((info->data.texturePosition.y & 0xFF) << 8) | (info->size.x & 0xFF);
               iterator->w = info->textureID + 255;
            }
            iterator->z = ((info->size.x << 4) & 0xF000) | (info->size.y & 0xFFF);
         }
         info -= textureInfoQuantity;
      }
      while (glUnmapBuffer(GL_TEXTURE_BUFFER) != GL_TRUE);
   }
   glBindTexture(GL_TEXTURE_BUFFER, textures[0]);
   GL_CHECK_ERRORS;
   glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA16UI, elementsBuffers[0]);
   GL_CHECK_ERRORS;
   return data;
}

static void deleteMap2DRenderingBuffer__openGL (struct RenderingData *data)
{
   GLuint objects[2] = {data->elementBuffer, data->metaDataBuffer};
   glDeleteBuffers(2, objects);
   GL_CHECK_ERRORS;
   objects[0] = data->elementTexture;
   objects[1] = data->metaDataTexture;
   glDeleteTextures(2, objects);
   GL_CHECK_ERRORS;
   free(data);
}

static void loadTexture__openGL (void *data, uint16_t width, uint16_t height, uint16_t textureID)
{
   glBindTexture(GL_TEXTURE_2D_ARRAY, glTexturesArray);
   GL_CHECK_ERRORS;
   if (width > cceTextureSize->x || height > cceTextureSize->y)
   {
      // Texture is truncated downwards, but openGL expects 0 to be at the bottom. So, we move texture parts line by line to the bottom
      for (struct cce_u8vec4 *iterator = data, *jiterator = (struct cce_u8vec4*)data + width * (height - cceTextureSize->y), *jend = (struct cce_u8vec4*)data + width * height;
           jiterator < jend;)
      {
         for (struct cce_u8vec4 *end = iterator + cceTextureSize->x; iterator < end; ++iterator, ++jiterator)
         {
            *iterator = *jiterator;
         }
         jiterator += width - cceTextureSize->y;
      }
      width = cceTextureSize->x;
      height = cceTextureSize->y;
   }
   glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, textureID, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
   GL_CHECK_ERRORS;
}

void terminateMap2DRenderer__openGL (void)
{
   free(uniformLocations);
   if (GLAD_GL_NV_copy_image == 0)
   {
      glDeleteFramebuffers(1, &glTemporaryFBO);
      GL_CHECK_ERRORS;
   }
   glDeleteBuffers(1, &g_VBO);
   GL_CHECK_ERRORS;
   glDeleteVertexArrays(1, &g_VAO);
   glDeleteTextures(1, &glTexturesArray);
   glDeleteProgram(shaderProgram);
}

int initMap2DRenderer__openGL (const struct LoadedTextures **textures, struct RendereringFunctions *funcStruct)
{
   {
      /*strlen("define CCE_GLOBAL_OFFSET_MASK 0xXXXXXXXX\n") == 42*/
      /*strlen("const uvec2 textureSize = uvec2(0.XXXXXXXX, 0.XXXXXXXX);") == 56*/
      char vertexShaderAdditionalString[42 + 56 + 1] = "#define CCE_GLOBAL_OFFSET_MASK " CCE_MACRO_TO_STR(CCE_GLOBAL_OFFSET_MASK) "\nconst vec2 inverseTextureSize = vec2(";
      sprintf(vertexShaderAdditionalString + strlen(vertexShaderAdditionalString), "%.8f, %.8f);", 1.0f / cceTextureSize->x, 1.0f / cceTextureSize->y);
      
      #ifdef SYSTEM_RESOURCE_PATH
      shaderProgram = cce__makeVFshaderProgram(SYSTEM_RESOURCE_PATH "shaders/vertex_shader.glsl", SYSTEM_RESOURCE_PATH "shaders/fragment_shader.glsl", vertexShaderAdditionalString, NULL);
      if (shaderProgram == 0u)
      #endif // SYSTEM_RESOURCE_PATH
      {
         shaderProgram = cce__makeVFshaderProgram("shaders/vertex_shader.glsl", "shaders/fragment_shader.glsl", vertexShaderAdditionalString, NULL);
      }
   }
   if (!shaderProgram)
   {
      fputs("ENGINE::INIT::SHADERS_CANNOT_BE_LOADED", stderr);
      return -1;
   }
   uniformLocations = malloc(1 * sizeof(GLint));
   GL_CHECK_ERRORS;
   *(uniformLocations) = glGetUniformLocation(shaderProgram, "ViewMatrix");
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
   stbi_set_flip_vertically_on_load(1);
   glUniform1i(glGetUniformLocation(shaderProgram, "Textures"), 0);
   GL_CHECK_ERRORS;
   glUniform1i(glGetUniformLocation(shaderProgram, "ElementInfo"), 1);
   GL_CHECK_ERRORS;
   glUniform1i(glGetUniformLocation(shaderProgram, "ElementData"), 2);
   GL_CHECK_ERRORS;
   funcStruct->drawMap2D = drawMap2D__openGL;
   funcStruct->map2DElementsToRenderingBuffer = map2DElementsToRenderingBuffer__openGL;
   //funcStruct->renderingBufferToMap2DElements = renderingBufferToMap2DElements__openGL;
   funcStruct->deleteMap2DRenderingBuffer = deleteMap2DRenderingBuffer__openGL;
   funcStruct->loadTexture = loadTexture__openGL;
   funcStruct->terminateMap2DRenderer = terminateMap2DRenderer__openGL;
   funcStruct->getRenderingDataSize = getRenderingDataSize__openGL;
   if (GLAD_GL_NV_copy_image == 0)
   {
      glGenFramebuffers(1, &glTemporaryFBO);
      funcStruct->reallocateTextureArray = resizeTextureArrayBegin__openGL_no_ext;
      funcStruct->moveTextureFromOldArray = copyTextureToResizedArray__openGL_no_ext;
      funcStruct->removeOldArray = resizeTextureArrayEnd__openGL_no_ext;
   }
   else
   {
      funcStruct->reallocateTextureArray = resizeTextureArrayBegin__openGL_copy_image_ext;
      funcStruct->moveTextureFromOldArray = copyTextureToResizedArray__openGL_copy_image_ext;
      funcStruct->removeOldArray = resizeTextureArrayEnd__openGL_copy_image_ext;
   }
   return 0;
}
