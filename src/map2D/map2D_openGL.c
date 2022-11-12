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
   uint16_t elementDataQuantity;
};

static const struct LoadedTextures **g_textures;
static GLuint   glTexturesArray;
static GLuint   glOldTexturesArray;
static GLuint   glTemporaryFBO;
static GLuint   shaderProgram;
static GLuint   g_VAO, g_VBO, g_UBO;
static GLint   *bufferUniformsOffsets;
static GLint   *uniformLocations;
static GLint    g_uniformBufferSize;
static struct Map2DElementData *cce__elementConversionBuffer;

static void cce__openGLErrorPrint (GLenum error, size_t line, const char *file)
{
   switch (error)
   {
      case GL_NO_ERROR: break;
      case GL_INVALID_ENUM:
      {
         fprintf(stderr, "%s: %ld: OPENGL::INVALID_ENUM:\nan unacceptable value is specified for an enumerated argument\n", file, line);
         break;
      }
      case GL_INVALID_VALUE:
      {
         fprintf(stderr, "%s: %ld: OPENGL::INVALID_VALUE:\na numeric argument is out of range\n", file, line);
         break;
      }
      case GL_INVALID_OPERATION:
      {
         fprintf(stderr, "%s: %ld: OPENGL::INVALID_OPERATION:\nthe specified operation is not allowed in the current state\n", file, line);
         break;
      }
      case GL_INVALID_FRAMEBUFFER_OPERATION:
      {
         fprintf(stderr, "%s: %ld: OPENGL::INVALID_OPERATION::FRAMEBUFFER:\nthe framebuffer object is not complete\n", file, line);
         break;
      }
      case GL_OUT_OF_MEMORY:
      {
         fprintf(stderr, "%s: %ld: OPENGL::OUT_OF_MEMORY:\nthere is not enough memory left to execute the command\n", file, line);
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
         fprintf(stderr, "%s: %ld: OPENGL::UNKNOWN:\n%d\n", file, line, error);
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
   glBindBufferRange(GL_UNIFORM_BUFFER, 1u, g_UBO, 0u, g_uniformBufferSize);
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

static void setUniformBufferToDefault (GLuint UBO)
{
   glBindBuffer(GL_UNIFORM_BUFFER, UBO);
   do
   {
      GL_CHECK_ERRORS;
      cce_void *uboData = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
      GL_CHECK_ERRORS;
      memset(uboData + bufferUniformsOffsets[CCE_TEXTUREOFFSET_OFFSET], 0, sizeof(GLint) * 2 * 256);
      memset(uboData + bufferUniformsOffsets[CCE_OFFSET_OFFSET], 0, sizeof(GLint) * 2 * 256);
      for (struct cce_f32vec4 *iterator = (struct cce_f32vec4*)(uboData + bufferUniformsOffsets[CCE_COLORGROUP_OFFSET]), *end = iterator + 128; iterator < end; ++iterator)
      {
         *iterator = (struct cce_f32vec4) {1.0f, 1.0f, 1.0f, 1.0f};
      }
      struct mat2x3
      {
         float data[6];
      };
      struct mat2x3 unitMatrix = {1, 0, 0, 1, 0, 0};
      for (struct mat2x3 *iterator = (struct mat2x3*) (uboData + bufferUniformsOffsets[CCE_TRANSFORMGROUP_OFFSET]), *end = iterator + 257; iterator < end; ++iterator)
      {
         *iterator = unitMatrix;
      }
   }
   while (glUnmapBuffer(GL_UNIFORM_BUFFER) == GL_FALSE);
   GL_CHECK_ERRORS;
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
                                                                     const struct cce_texture2D *textureInfo, uint16_t textureInfoQuantity,
                                                                     const struct Map2DElementData *elementData, uint16_t elementDataQuantity)
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
      glBufferData(GL_TEXTURE_BUFFER, elements->dataQuantity * sizeof(struct Map2DElementPosition), NULL, GL_STATIC_DRAW);
      GL_CHECK_ERRORS;
      do
      {
         struct cce_i16vec4 *iterator = glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY);
         struct Map2DElementPosition *element = elements->data;
         for (struct cce_i16vec4 *end = iterator + elements->dataQuantity; iterator < end; ++iterator, ++element)
         {
            iterator->x = element->position.x;
            iterator->y = element->position.y;
            iterator->z = element->elementDataID - (1 << (sizeof(uint16_t) * 8 - 1)); // We need to get it back as unsigned in glsl (simple reinterpret cast don't work - glsl don't have 16-bit types)
            iterator->w = element->textureDataID - (1 << (sizeof(uint16_t) * 8 - 1));
         }
      }
      while (glUnmapBuffer(GL_TEXTURE_BUFFER) == GL_FALSE);
      glBindTexture(GL_TEXTURE_BUFFER, *titerator);
      GL_CHECK_ERRORS;
      glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA16I, *ebiterator);
      GL_CHECK_ERRORS;
      diterator->elementBufferSize = elements->dataQuantity;
      diterator->textureInfoQuantity = textureInfoQuantity;
      diterator->elementDataQuantity = elementDataQuantity;
      diterator->elementBuffer = *ebiterator;
      diterator->elementTexture = *titerator;
      diterator->metaDataBuffer = elementsBuffers[0];
      diterator->metaDataTexture = textures[0];
   }
   
   glBindBuffer(GL_TEXTURE_BUFFER, elementsBuffers[0]);
   GL_CHECK_ERRORS;
   glBufferData(GL_TEXTURE_BUFFER, (textureInfoQuantity + elementDataQuantity) * 8, NULL, GL_STATIC_DRAW);
   GL_CHECK_ERRORS;
   uint16_t  positionY;
   uint16_t *iterator;
   do
   {
      iterator = glMapBuffer(GL_TEXTURE_BUFFER, GL_WRITE_ONLY);
      GL_CHECK_ERRORS;
      for (uint16_t *end = iterator + elementDataQuantity * 4; iterator < end; iterator += 4, ++elementData)
      {
         iterator[0] = (elementData->size.x                    << 8) | elementData->transformGroups[0];
         iterator[1] = (elementData->size.y                    << 8) | elementData->transformGroups[1];
         iterator[2] = (elementData->textureOffsetGroup        << 8) | elementData->transformGroups[2];
         iterator[3] = (elementData->colorGroupAndGlobalOffset << 8) | elementData->transformGroups[3];
      }
      for (uint16_t *jiterator = iterator + (textureInfoQuantity - 1) * 4; jiterator >= iterator; jiterator -= 4, ++textureInfo)
      {
         positionY = (*g_textures)[textureInfo->ID].size.y - (textureInfo->position.y + textureInfo->size.y);
         jiterator[0] = (textureInfo->position.x & 0xFFF) | (textureInfo->size.y << 12);
         jiterator[1] = (positionY               & 0xFFF) | ((textureInfo->size.y << 8) & 0xF);
         jiterator[2] = (textureInfo->size.x     & 0xFFF) | ((textureInfo->size.y << 4) & 0xF);
         jiterator[3] = textureInfo->ID;
      }
      elementData -= elementDataQuantity;
      textureInfo -= textureInfoQuantity;
   }
   while (glUnmapBuffer(GL_TEXTURE_BUFFER) != GL_TRUE);
   glBindTexture(GL_TEXTURE_BUFFER, textures[0]);
   GL_CHECK_ERRORS;
   glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA16UI, elementsBuffers[0]);
   GL_CHECK_ERRORS;
   return data;
}

/*
static struct ElementInfo* renderingBufferToMap2DElements__openGL (struct RenderingData *data)
{
   struct ElementInfo *result = malloc(sizeof(struct ElementInfo) + data->elementBufferSize * sizeof(struct Map2DElementPosition)
                                                                  + data->elementDataQuantity * sizeof(struct Map2DElementData)
                                                                  + data->textureInfoQuantity * sizeof(struct cce_texture2D));
   result->elementsQuantity = data->elementBufferSize;
   result->elementDataQuantity = data->elementDataQuantity;
   result->textureInfoQuantity = data->textureInfoQuantity;
   result->elements = (struct Map2DElementPosition*)(result + 1);
   result->elementData = (struct Map2DElementData*)(result->elements + result->elementsQuantity);
   result->textureInfo = (struct cce_texture2D*)(result->elementData + result->elementDataQuantity);
   glBindBuffer(GL_TEXTURE_BUFFER, data->elementBuffer);
   glGetBufferSubData(GL_TEXTURE_BUFFER, 0, result->elementsQuantity * sizeof(struct Map2DElementPosition), result->elements);
   glBindBuffer(GL_TEXTURE_BUFFER, data->metaDataBuffer);
   {
      struct GLdata
      {
         uint16_t data[4];
      };
      struct GLdata *iterator = glMapBuffer(GL_TEXTURE_BUFFER, GL_READ_ONLY);
      struct Map2DElementData *elementData = result->elementData;
      struct GLdata buf;
      for (struct GLdata *end = iterator + result->elementDataQuantity; iterator < end; ++iterator, ++elementData)
      {
         buf = *iterator;
         elementData->size.x = buf.data[0] >> 8;
         elementData->transformGroups[0] = buf.data[0] & UINT8_MAX;
         elementData->size.y = buf.data[1] >> 8;
         elementData->transformGroups[1] = buf.data[1] & UINT8_MAX;
         elementData->textureOffsetGroup = buf.data[2] >> 8;
         elementData->transformGroups[2] = buf.data[2] & UINT8_MAX;
         elementData->colorGroupAndGlobalOffset = buf.data[3] >> 8;
         elementData->transformGroups[3] = buf.data[3] & UINT8_MAX;
      }
      struct cce_texture2D *textureInfo = result->textureInfo;
      for (struct GLdata *jiterator = iterator + result->textureInfoQuantity - 1; jiterator >= iterator; --jiterator, ++textureInfo)
      {
         buf = *jiterator;
         textureInfo->position.x = buf.data[0] & 0xFFF;
         textureInfo->position.y = buf.data[1] & 0xFFF;
         textureInfo->size.x     = buf.data[2] & 0xFFF;
         textureInfo->size.y     = buf.data[0] & 0xF000 | (buf.data[1] & 0xF000) >> 4 | (buf.data[2] & 0xF000) >> 8;
         textureInfo->ID         = buf.data[3];
      }
      glUnmapBuffer(GL_TEXTURE_BUFFER);
   }
   return result;
}
*/

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
      for (struct cce_u8vec4 *iterator = data, *jiterator = data + width * (height - cceTextureSize->y), *jend = data + width * height; jiterator < jend;)
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
   free(bufferUniformsOffsets);
   if (GLAD_GL_NV_copy_image == 0)
   {
      glDeleteFramebuffers(1, &glTemporaryFBO);
      GL_CHECK_ERRORS;
   }
   {
      GLuint buffers[2] = {g_UBO, g_VBO};
      glDeleteBuffers(2, buffers);
      GL_CHECK_ERRORS;
   }
   glDeleteVertexArrays(1, &g_VAO);
   glDeleteTextures(1, &glTexturesArray);
   glDeleteProgram(shaderProgram);
   gladLoaderUnloadGL();
}

int initMap2DRenderer__openGL (char *cce__resourcePath, const struct LoadedTextures **textures, struct RendereringFunctions *funcStruct)
{
   if (!gladLoaderLoadGL())
   {
      fprintf(stderr, "GLAD::INITIALIZATION::FAILED:\nOpenGL could not be loaded by GLAD.\n");
      return -1;
   }
   if (!GLAD_GL_VERSION_3_2)
   {
      fprintf(stderr, "GLAD::INITIALIZATION::FAILED:\nOpenGL 3.2 (minimum required) could not be loaded by GLAD.\n");
      return -1;
   }
   {
      size_t pathLength = strlen(cce__resourcePath) + 1u;
      /*strlen("define CCE_GLOBAL_OFFSET_MASK 0xXXXXXXXX\n") == 41*/
      /*strlen("const uvec2 textureSize = uvec2(0.XXXXXXXX, 0.XXXXXXXX);") == 55*/
      char vertexShaderAdditionalString[41 + 55] = "#define CCE_GLOBAL_OFFSET_MASK " CCE_MACRO_TO_STR(CCE_GLOBAL_OFFSET_MASK) "\nconst vec2 inverseTextureSize = vec2(";
      sprintf(vertexShaderAdditionalString + strlen(vertexShaderAdditionalString), "%.8f, %.8f);", 1.0f / cceTextureSize->x, 1.0f / cceTextureSize->y);
      
      #ifdef SYSTEM_RESOURCE_PATH
      shaderProgram = cce__makeVFshaderProgram(SYSTEM_RESOURCE_PATH "shaders/vertex_shader.glsl", SYSTEM_RESOURCE_PATH "shaders/fragment_shader.glsl", vertexShaderAdditionalString, NULL);
      if (shaderProgram == 0u)
      #endif // SYSTEM_RESOURCE_PATH
      {
         cceAppendPath(cce__resourcePath, pathLength + 11u, "shaders");
         char *vertexPath   = cceCreateNewPathFromOldPath(cce__resourcePath, "vertex_shader.glsl",   0u);
         char *fragmentPath = cceCreateNewPathFromOldPath(cce__resourcePath, "fragment_shader.glsl", 0u);
         shaderProgram = cce__makeVFshaderProgram(vertexPath, fragmentPath, vertexShaderAdditionalString, NULL);
         free(vertexPath);
         free(fragmentPath);
         *(cce__resourcePath + pathLength) = '\0';
      }
   }
   if (!shaderProgram)
   {
      fputs("ENGINE::INIT::SHADERS_CANNOT_BE_LOADED", stderr);
      free(cce__resourcePath);
      return -1;
   }
   uniformLocations = malloc(1 * sizeof(GLint));
   GL_CHECK_ERRORS;
   *(uniformLocations) = glGetUniformLocation(shaderProgram, "ViewMatrix");
   GL_CHECK_ERRORS;
   {
      const GLchar *uniformNames[] = {"Transform", "Offset", "Colors", "TextureOffset"};
      GLuint indices[6];
      glGetUniformIndices(shaderProgram, 4, uniformNames, indices);
      GL_CHECK_ERRORS;
      bufferUniformsOffsets = (GLint*) malloc(4 * sizeof(GLint));
      glGetActiveUniformsiv(shaderProgram, 4, indices, GL_UNIFORM_OFFSET, bufferUniformsOffsets);
      GL_CHECK_ERRORS;
      glUniformBlockBinding(shaderProgram, glGetUniformBlockIndex(shaderProgram, "Variables"), 1u);
      GL_CHECK_ERRORS;
      g_uniformBufferSize = bufferUniformsOffsets[2] + sizeof(GLint) * 2 * 256;
   }
   glUseProgram(shaderProgram);
   GL_CHECK_ERRORS;
   {
      GLuint buffers[2];
      glGenBuffers(2, buffers);
      GL_CHECK_ERRORS;
      g_UBO = buffers[0];
      g_VBO = buffers[1];
   }
   glBindBuffer(GL_UNIFORM_BUFFER, g_UBO);
   GL_CHECK_ERRORS;
   glBufferData(GL_UNIFORM_BUFFER, (g_uniformBufferSize), NULL, GL_DYNAMIC_DRAW);
   GL_CHECK_ERRORS;
   setUniformBufferToDefault(g_UBO);
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
