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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../include/cce/os_interaction.h"

#include "shader.h"

/* Workaround to set defines in GLSL at engine start */
char* cce__prependStringToShader (char *shaderSrc, const char *const shaderAdditionalString)
{
   size_t shaderSrcLength = strlen(shaderSrc);
   size_t additionalStringLength = strlen(shaderAdditionalString);
   char *toInsert = strstr(shaderSrc, "#version");
   if (toInsert == NULL)
      toInsert = shaderSrc;
   else
      toInsert = memchr(toInsert, '\n', shaderSrcLength - (toInsert - shaderSrc)) + 1;
   
   memmove(toInsert + additionalStringLength + 1, toInsert, shaderSrcLength - (toInsert - shaderSrc) + 1 /*\0*/);
   memcpy(toInsert, shaderAdditionalString, additionalStringLength);
   toInsert[additionalStringLength] = '\n';
   return shaderSrc;
}

GLuint cce__makeVFshaderProgram  (const char *vertexPath, const char *fragmentPath,
                                  const char *vertexShaderAdditionalString, const char *const fragmentShaderAdditionalString)
{
   GLuint vertexShader = 0u, fragmentShader = 0u, shaderProgram = 0u;
   size_t additionalStringLength = 0;
   if (vertexShaderAdditionalString != NULL)
      additionalStringLength = strlen(vertexShaderAdditionalString);
   
   char *shaderSrc = cce__readTextFile(vertexPath, additionalStringLength + 1);
   if (shaderSrc == NULL)
   {
      fprintf(stderr, "OPENGL::SHADER::VERTEX::FAILED_TO_LOAD:\n%s\n", vertexPath);
      goto FINAL;
   }
   if (additionalStringLength > 0)
      cce__prependStringToShader(shaderSrc, vertexShaderAdditionalString);
   vertexShader = cce__compileShader(shaderSrc, GL_VERTEX_SHADER);
   if (vertexShader == 0u)
      goto FINAL;
   free(shaderSrc);
   
   additionalStringLength = 0;
   if (fragmentShaderAdditionalString != NULL)
      additionalStringLength = strlen(fragmentShaderAdditionalString);
   shaderSrc = cce__readTextFile(fragmentPath, additionalStringLength + 1);
   if (shaderSrc == NULL)
   {
      fprintf(stderr, "OPENGL::SHADER::FRAGMENT::FAILED_TO_LOAD:\n%s\n", fragmentPath);
      goto FINAL;
   }
   if (additionalStringLength > 0)
      cce__prependStringToShader(shaderSrc, fragmentShaderAdditionalString);
   fragmentShader = cce__compileShader(shaderSrc, GL_FRAGMENT_SHADER);
   if (fragmentShader == 0u)
      goto FINAL;
   
   shaderProgram = cce__createVFshaderProgram(vertexShader, fragmentShader);
   
FINAL:
   free(shaderSrc);
   glDeleteShader(vertexShader);
   glDeleteShader(fragmentShader);
   return shaderProgram;
}

GLuint cce__makeVGFshaderProgram (const char *vertexPath, const char *geometryPath, const char *fragmentPath,
                                  const char *vertexShaderAdditionalString, const char *geometryShaderAdditionalString, const char *fragmentShaderAdditionalString)
{
   GLuint vertexShader = 0u, geometryShader = 0u, fragmentShader = 0u, shaderProgram = 0u;
   size_t additionalStringLength = 0;
   if (vertexShaderAdditionalString != NULL)
      additionalStringLength = strlen(vertexShaderAdditionalString);
   
   char *shaderSrc = cce__readTextFile(vertexPath, additionalStringLength + 1);
   if (shaderSrc == NULL)
   {
      fprintf(stderr, "OPENGL::SHADER::VERTEX::FAILED_TO_LOAD:\n%s\n", vertexPath);
      goto FINAL;
   }
   if (additionalStringLength > 0)
      cce__prependStringToShader(shaderSrc, vertexShaderAdditionalString);
   vertexShader = cce__compileShader(shaderSrc, GL_VERTEX_SHADER);
   if (vertexShader == 0u)
      goto FINAL;
   free(shaderSrc);
   
   additionalStringLength = 0;
   if (fragmentShaderAdditionalString != NULL)
      additionalStringLength = strlen(geometryShaderAdditionalString);
   shaderSrc = cce__readTextFile(geometryPath, additionalStringLength + 1);
   if (shaderSrc == NULL)
   {
      fprintf(stderr, "OPENGL::SHADER::GEOMETRY::FAILED_TO_LOAD:\n%s\n", fragmentPath);
      goto FINAL;
   }
   if (additionalStringLength > 0)
      cce__prependStringToShader(shaderSrc, geometryShaderAdditionalString);
   geometryShader = cce__compileShader(shaderSrc, GL_GEOMETRY_SHADER);
   if (geometryShader == 0u)
      goto FINAL;
   free(shaderSrc);
   
   additionalStringLength = 0;
   if (fragmentShaderAdditionalString != NULL)
      additionalStringLength = strlen(fragmentShaderAdditionalString);
   shaderSrc = cce__readTextFile(fragmentPath, additionalStringLength + 1);
   if (shaderSrc == NULL)
   {
      fprintf(stderr, "OPENGL::SHADER::FRAGMENT::FAILED_TO_LOAD:\n%s\n", fragmentPath);
      goto FINAL;
   }
   if (additionalStringLength > 0)
      cce__prependStringToShader(shaderSrc, fragmentShaderAdditionalString);
   fragmentShader = cce__compileShader(shaderSrc, GL_FRAGMENT_SHADER);
   if (fragmentShader == 0u)
      goto FINAL;
   
   shaderProgram = cce__createVGFshaderProgram(vertexShader, geometryShader, fragmentShader);
   
FINAL:
   free(shaderSrc);
   glDeleteShader(vertexShader);
   glDeleteShader(geometryShader);
   glDeleteShader(fragmentShader);
   return shaderProgram;
}

char* cce__readTextFile (const char *const path, size_t spaceToLeave)
{
   FILE *file = fopen(path, "r");
   if (file == NULL)
      return NULL;
   fseek(file, 0L, SEEK_END);
   size_t size = ftell(file);
   rewind(file);
   char *text = malloc((size + spaceToLeave + 1 /*\0*/) * sizeof(char));
   fread(text, sizeof(char), size, file);
   fclose(file);
   *(text + size) = '\0';
   return text;
}

GLuint cce__compileShader (const char *shaderSource, GLenum shaderType)
{
   GLuint shader;
   shader = glCreateShader(shaderType);
   glShaderSource(shader, 1, &shaderSource, NULL);
   glCompileShader(shader);
   GLint success;
   glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
   if (success == GL_FALSE)
   {
      char infoLog[512];
      glGetShaderInfoLog(shader, 512, NULL, infoLog);
      #ifndef NDEBUG
      fprintf(stderr, "Broken shader:\n%s\n", shaderSource);
      #endif
      fprintf(stderr, "OPENGL::SHADER::FAILED_TO_COMPILE:\n%s\n", infoLog);
      return 0u;
   }
   return shader;
}

GLuint cce__createVFshaderProgram (GLuint vertexShader, GLuint fragmentShader)
{
   GLuint shaderProgram = glCreateProgram();
   glAttachShader(shaderProgram, vertexShader);
   glAttachShader(shaderProgram, fragmentShader);
   glLinkProgram(shaderProgram);
   GLint success;
   glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
   if (success == GL_FALSE) {
      char infoLog[512];
      glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
      fprintf(stderr, "OPENGL::SHADER::FAILED_TO_LINK:\n%s\n", infoLog);
      return 0u;
   }
   return shaderProgram;
}

GLuint cce__createVGFshaderProgram (GLuint vertexShader, GLuint geometryShader, GLuint fragmentShader)
{
   GLuint shaderProgram = glCreateProgram();
   glAttachShader(shaderProgram, vertexShader);
   glAttachShader(shaderProgram, geometryShader);
   glAttachShader(shaderProgram, fragmentShader);
   glLinkProgram(shaderProgram);
   int success;
   glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
   if (!success) {
      char infoLog[512];
      glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
      fprintf(stderr, "OPENGL::SHADER::FAILED_TO_LINK:\n%s\n", infoLog);
      return 0u;
   }
   return shaderProgram;
}
