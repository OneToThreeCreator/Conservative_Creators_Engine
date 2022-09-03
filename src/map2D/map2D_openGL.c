#include <stdio.h>
#include <stdlib.h>

#include <glad/gl.h>
#include "../shader.h"

#include "../../include/coffeechain/utils.h"
#include "../../include/coffeechain/os_interaction.h"

#include "map2D_internal.h"

#ifndef CCE_ELEMENT_CONVERSION_BUFFER_SIZE
#define CCE_ELEMENT_CONVERSION_BUFFER_SIZE 128 /* Counted in elements, not bytes! */
#endif // CCE_ELEMENT_CONVERSION_BUFFER_SIZE

#define CCE_TRANSFORMGROUP_OFFSET 0u
#define CCE_OFFSET_OFFSET 1u
#define CCE_COLORGROUP_OFFSET 2u
#define CCE_TEXTUREOFFSET_OFFSET 3u

#define CCE_GLOBALTRANSFORM_OFFSET 0u
#define CCE_VIEWMATRIX_OFFSET 1u

struct Map2DElementData
{
   struct cce_i32vec2 position;
   uint8_t  transformIDs    [4];
   uint8_t  textureOffsetIDs[2];
   uint8_t  colorIDs        [2];
   struct cce_u16vec2 size;
   struct cce_u16vec2 textureCoords;
   struct cce_u16vec2 textureSize;
   uint16_t textureID;
   uint8_t  flags;
}; // 32 bytes

struct RenderingData
{
   GLuint textureObject;
   uint32_t textureSize;
};

static const struct LoadedTextures **g_textures;
static uint16_t glTexturesArraySize;
static GLuint   glTexturesArray;
static GLuint   shaderProgram;
static GLuint   g_VAO, g_VBO, g_UBO;
static GLint   *bufferUniformsOffsets;
static GLint   *uniformLocations;
static GLint    g_uniformBufferSize;
static struct Map2DElementData *cce__elementConversionBuffer;

void cce__openGLErrorPrint (GLenum error, size_t line, const char *file);

#ifdef NDEBUG
#define GL_CHECK_ERRORS
#else
#define GL_CHECK_ERRORS cce__openGLErrorPrint(glGetError(), __LINE__, __FILE__)
#endif

static void drawMap2D (struct Map2D *map)
{
   glBindVertexArray(g_VAO);
   GL_CHECK_ERRORS;
   glBindBufferRange(GL_UNIFORM_BUFFER, 1u, g_UBO, 0u, g_uniformBufferSize);
   GL_CHECK_ERRORS;
   glDrawArraysInstanced(GL_TRIANGLES, 0, 6, map->elementsQuantity);
   GL_CHECK_ERRORS;
}

static void cce__setUniformBufferToDefault (GLuint UBO)
{
   glBindBuffer(GL_UNIFORM_BUFFER, UBO);
   GL_CHECK_ERRORS;
   cce_void *uboData = glMapBuffer(GL_UNIFORM_BUFFER, GL_WRITE_ONLY);
   GL_CHECK_ERRORS;
   memset(uboData + bufferUniformsOffsets[CCE_COLORGROUP_OFFSET], 0, sizeof(GLfloat) * 4 * 256);
   memset(uboData + bufferUniformsOffsets[CCE_TEXTUREOFFSET_OFFSET], 0, sizeof(GLint) * 2 * 256);
   struct mat2x3
   {
      float data[6];
   };
   struct mat2x3 unitMatrix = {1, 0, 0, 1, 0, 0};
   for (struct mat2x3 *iterator = (struct mat2x3*) (uboData + bufferUniformsOffsets[CCE_TRANSFORMGROUP_OFFSET]), *end = iterator + 256; iterator < end; ++iterator)
   {
      *iterator = unitMatrix;
   }
   glUnmapBuffer(GL_UNIFORM_BUFFER);
   GL_CHECK_ERRORS;
}

static GLuint createTextureArray (uint16_t newSize)
{
   GLuint texture;
   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_2D_ARRAY, texture);
   glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RGBA8, cceTextureSize->x, cceTextureSize->y, newSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
   GL_CHECK_ERRORS;   
   
   glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_REPEAT);
   glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
   glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
   
   return texture;
}

int cce__initMap2DOpenGLRenderer (char *cce__resourcePath, const struct LoadedTextures **textures)
{
   {
      size_t pathLength = strlen(cce__resourcePath) + 1u;
      /*strlen("define CCE_GLOBAL_OFFSET_MASK 0xXXXXXXXX\n") == 41*/
      /*strlen("const uvec2 textureSize = uvec2(0.XXXXXXXX, 0.XXXXXXXX);") == 56*/
      char vertexShaderAdditionalString[41 + 56] = "#define CCE_GLOBAL_OFFSET_MASK " CCE_MACRO_TO_STR(CCE_GLOBAL_OFFSET_MASK) "\nconst uvec2 inverseTextureSize = vec2(";
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
      cce__terminateEngine();
      free(cce__resourcePath);
      return -1;
   }
   
   uniformLocations = malloc(3 * sizeof(GLint));
   *uniformLocations = glGetUniformLocation(shaderProgram, "InverseStep");
   GL_CHECK_ERRORS;
   *(uniformLocations + 1) = glGetUniformLocation(shaderProgram, "GlobalMoveCoords");
   GL_CHECK_ERRORS;
   *(uniformLocations + 2) = glGetUniformLocation(shaderProgram, "MapOffset");
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
   glGenBuffers(1u, &(g_UBO));
   GL_CHECK_ERRORS;
   glBindBuffer(GL_UNIFORM_BUFFER, g_UBO);
   GL_CHECK_ERRORS;
   glBufferData(GL_UNIFORM_BUFFER, (g_uniformBufferSize), NULL, GL_DYNAMIC_DRAW);
   GL_CHECK_ERRORS;
   cce__setUniformBufferToDefault(g_UBO);
   glEnable(GL_BLEND);
   GL_CHECK_ERRORS;
   glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
   GL_CHECK_ERRORS;
   
   glGenVertexArrays(1, &g_VAO);
   GL_CHECK_ERRORS;
   glGenBuffers(1, &g_VBO);
   GL_CHECK_ERRORS;
   glBindVertexArray(g_VAO);
   GL_CHECK_ERRORS;
   glBindBuffer(GL_ARRAY_BUFFER, g_VBO);
   GL_CHECK_ERRORS;
   GLbyte square[6 * 2] = 
   {
      -128, -128,
       127, -128,
      -128,  127,
       127,  127,
      -128,  127,
       127, -128,
   };
   glBufferData(GL_ARRAY_BUFFER, (sizeof(GLbyte) * 6 * 2), &square, GL_STATIC_DRAW);
   GL_CHECK_ERRORS;
   glVertexAttribPointer(0, 2, GL_BYTE, GL_TRUE, 2 * sizeof(GLbyte), (void*) 0);
   GL_CHECK_ERRORS;
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   GL_CHECK_ERRORS;
   glBindVertexArray(0);
   GL_CHECK_ERRORS;
   g_textures = textures;
   cce__elementConversionBuffer = malloc(CCE_ELEMENT_CONVERSION_BUFFER_SIZE * sizeof(struct Map2DElementData));
   return 0;
}

static void cce__elementToMap2DElementData (struct Map2DElementData *buffer, struct cce_i32vec2 position, struct cce_u16vec2 size, uint8_t *transformGroups, uint8_t transformGroupsQuantity,
                                     uint8_t globalOffset, struct cce_u16vec2 texturePosition, struct cce_u16vec2 textureSize, uint16_t textureID, 
                                     uint8_t *textureOffsetGroups, uint8_t textureOffsetGroupsQuantity, uint8_t *colorGroups, uint8_t colorGroupsQuantity)
{
   struct cce_u16vec2 textureResolution = (*g_textures)[textureID - (textureID > 0)].size;
   buffer->position = position;
   buffer->size = size;
   buffer->textureCoords.x = texturePosition.x + (cceTextureSize->x - textureResolution.x);
   buffer->textureCoords.y = cceTextureSize->y - (texturePosition.y + textureSize.y);
   buffer->textureSize = textureSize;
   memcpy(buffer->transformIDs,     transformGroups,     CCE_MIN(transformGroupsQuantity,     4));
   memset(buffer->transformIDs,     0,               4 - CCE_MIN(transformGroupsQuantity,     4));
   memcpy(buffer->textureOffsetIDs, textureOffsetGroups, CCE_MIN(textureOffsetGroupsQuantity, 2));
   memset(buffer->textureOffsetIDs, 0,               2 - CCE_MIN(textureOffsetGroupsQuantity, 2));
   memcpy(buffer->colorIDs,         colorGroups,         CCE_MIN(colorGroupsQuantity,         2));
   memset(buffer->colorIDs,         0,               2 - CCE_MIN(colorGroupsQuantity,         2));
   buffer->flags = (globalOffset > 0) * CCE_GLOBAL_OFFSET_MASK;
}

static struct RenderingData* cce__initializeMap2DRendering (struct Map2DElement *elements, uint32_t elementsQuantity)
{
   GLuint texture;
   glGenTextures(1, &texture);
   glBindTexture(GL_TEXTURE_1D, texture);
   glTexImage1D(GL_TEXTURE_1D, 0, GL_RGBA32UI, elementsQuantity, 0, GL_RGBA_INTEGER, GL_UNSIGNED_INT, NULL);
   struct Map2DElement *iterator = elements;
   for (struct Map2DElement *end = elements + (elementsQuantity / CCE_ELEMENT_CONVERSION_BUFFER_SIZE);
        iterator < end; ++iterator)
   {
      struct Map2DElementData *buf = cce__elementConversionBuffer;
      for (struct Map2DElement *jiterator = iterator, *jend = iterator + CCE_ELEMENT_CONVERSION_BUFFER_SIZE; jiterator < jend; ++jiterator, ++buf)
      {
         cce__elementToMap2DElementData(buf, jiterator->position, jiterator->size, jiterator->transformGroups, 4, , jiterator->textureInfo.position, jiterator->textureInfo.size,
                                        jiterator->textureInfo.ID, jiterator->textureOffsetGroups, 2, jiterator->colorGroups, 2);
      }
      glTexSubImage1D(GL_TEXTURE_1D, 0, (iterator - elements) * CCE_ELEMENT_CONVERSION_BUFFER_SIZE * sizeof(struct Map2DElementData), CCE_ELEMENT_CONVERSION_BUFFER_SIZE,
                      GL_RGBA, GL_UNSIGNED_INT, cce__elementConversionBuffer);
   }
   struct Map2DElementData *buf = cce__elementConversionBuffer;
   for (struct Map2DElement *jiterator = iterator, *jend = iterator + (elementsQuantity % CCE_ELEMENT_CONVERSION_BUFFER_SIZE); jiterator < jend; ++jiterator, ++buf)
   {
      cce__elementToMap2DElementData(buf, jiterator->position, jiterator->size, jiterator->transformGroups, 4, , jiterator->textureInfo.position, jiterator->textureInfo.size,
                                     jiterator->textureInfo.ID, jiterator->textureOffsetGroups, 2, jiterator->colorGroups, 2);
   }
   glTexSubImage1D(GL_TEXTURE_1D, 0, (iterator - elements) * CCE_ELEMENT_CONVERSION_BUFFER_SIZE * sizeof(struct Map2DElementData),
                   elementsQuantity % CCE_ELEMENT_CONVERSION_BUFFER_SIZE, GL_RGBA, GL_UNSIGNED_INT, cce__elementConversionBuffer);
   struct RenderingData *data = malloc(sizeof(struct RenderingData));
   data->textureObject = texture;
   data->textureSize = elementsQuantity;
   return data;
}

static void cce__updateDynamicMap2Delements (struct RenderingData *data, struct DynamicMap2DElement *elements, uint32_t elementsQuantity)
{
   GLuint texture = data->textureObject;
   uint32_t textureSize = data->textureSize;
}
