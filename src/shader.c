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

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "shader.h"
#include "path_getters.h"

/* There is EPIC workaround to set defines in GLSL at runtime */

unsigned int makeVFshaderProgram  (const char *const vertexPath, const char *const fragmentPath, uint16_t shadersVersion,
                                   const char *const vertexShaderAdditionalString, const char *const fragmentShaderAdditionalString)
{
   unsigned int vertexShader = 0u, fragmentShader = 0u, shaderProgram = 0u;
   char *shaderSrc = fileRead(vertexPath);
   if (shaderSrc == NULL)
   {
      fprintf(stderr, "OPENGL::SHADER::VERTEX::FAILED_TO_LOAD:\n%s\n", vertexPath);
      goto FINAL;
   }
   char *shaderModifiedSrc = addStringsInShader(shadersVersion, vertexShaderAdditionalString, shaderSrc);
   free(shaderSrc);
   vertexShader = compileShader(shaderModifiedSrc, GL_VERTEX_SHADER);
   free(shaderModifiedSrc);
   if (vertexShader == 0u)
      goto FINAL;
   
   shaderSrc = fileRead(fragmentPath);
   if (shaderSrc == NULL)
   {
      fprintf(stderr, "OPENGL::SHADER::FRAGMENT::FAILED_TO_LOAD:\n%s\n", fragmentPath);
      goto FINAL;
   }
   shaderModifiedSrc = addStringsInShader(shadersVersion, vertexShaderAdditionalString, shaderSrc);
   free(shaderSrc);
   fragmentShader = compileShader(shaderModifiedSrc, GL_FRAGMENT_SHADER);
   free(shaderModifiedSrc);
   if (fragmentShader == 0u)
      goto FINAL;
   
   shaderProgram = createVFshaderProgram(vertexShader, fragmentShader);
   
FINAL:
   glDeleteShader(vertexShader);
   glDeleteShader(fragmentShader);
   return shaderProgram;
}

unsigned int makeVGFshaderProgram (const char *const vertexPath, const char *const geometryPath, const char *const fragmentPath, uint16_t shadersVersion,
                                   const char *const vertexShaderAdditionalString, const char *const geometryShaderAdditionalString, const char *const fragmentShaderAdditionalString)
{
   unsigned int vertexShader = 0u, geometryShader = 0u, fragmentShader = 0u, shaderProgram = 0u;
   char *shaderSrc = fileRead(vertexPath);
   if (shaderSrc == NULL)
   {
      fprintf(stderr, "OPENGL::SHADER::VERTEX::FAILED_TO_LOAD:\n%s\n", vertexPath);
      goto FINAL;
   }
   char *shaderModifiedSrc = addStringsInShader(shadersVersion, vertexShaderAdditionalString, shaderSrc);
   free(shaderSrc);
   vertexShader = compileShader(shaderModifiedSrc, GL_VERTEX_SHADER);
   free(shaderModifiedSrc);
   if (vertexShader == 0u)
      goto FINAL;
   
   shaderSrc = fileRead(geometryPath);
   if (shaderSrc == NULL)
   {
      fprintf(stderr, "OPENGL::SHADER::GEOMETRY::FAILED_TO_LOAD:\n%s\n", fragmentPath);
      goto FINAL;
   }
   shaderModifiedSrc = addStringsInShader(shadersVersion, geometryShaderAdditionalString, shaderSrc);
   free(shaderSrc);
   geometryShader = compileShader(shaderModifiedSrc, GL_GEOMETRY_SHADER);
   free(shaderModifiedSrc);
   if (geometryShader == 0u)
      goto FINAL;
   
   shaderSrc = fileRead(fragmentPath);
   if (shaderSrc == NULL)
   {
      fprintf(stderr, "OPENGL::SHADER::FRAGMENT::FAILED_TO_LOAD:\n%s\n", fragmentPath);
      goto FINAL;
   }
   shaderModifiedSrc = addStringsInShader(shadersVersion, fragmentShaderAdditionalString, shaderSrc);
   free(shaderSrc);
   fragmentShader = compileShader(shaderModifiedSrc, GL_FRAGMENT_SHADER);
   free(shaderModifiedSrc);
   if (fragmentShader == 0u)
      goto FINAL;
   
   shaderProgram = createVGFshaderProgram(vertexShader, geometryShader, fragmentShader);
   
FINAL:
   glDeleteShader(vertexShader);
   glDeleteShader(geometryShader);
   glDeleteShader(fragmentShader);
   return shaderProgram;
}

char* fileRead (const char *const path)
{
   char *string;
   FILE *file = fopen(path, "r");
   if (file==NULL)
      return NULL;
   fseek(file, 0L, SEEK_END);
   unsigned int size = ftell(file);
   rewind(file);
   string = (char*) calloc((size+1), sizeof(char));
   for (unsigned int caret = 0u; caret < size; ++caret)
   {
      (*(string+caret))=fgetc(file);
   }
   fclose(file);
   (*(string+size))='\0';
   return string;
}

char* addStringsInShader (uint16_t shaderVersion, const char *const shaderAdditionalString, const char *const shaderSrc)
{
   /*strlen("#version xxxxx core\n") == 20*/
   size_t shaderSrcLength = strlen(shaderSrc);
   size_t additionalStringLength = strlen(shaderAdditionalString);
   char *shaderModifiedSrc = (char*) malloc((shaderSrcLength + additionalStringLength + 20 + 1/*\0*/) * sizeof(char));
   memcpy(shaderModifiedSrc, "#version ", 10);
   shortToString(shaderModifiedSrc, shaderVersion, " core\n");
   size_t versionStringLength = strlen(shaderModifiedSrc);
   memcpy(shaderModifiedSrc + versionStringLength, shaderAdditionalString, additionalStringLength);
   memcpy(shaderModifiedSrc + versionStringLength + additionalStringLength, shaderSrc, shaderSrcLength + 1); // +\0
   return shaderModifiedSrc;
}

unsigned int compileShader (const char *shaderSource, GLenum shaderType)
{
   unsigned int shader;
   shader = glCreateShader(shaderType);
   glShaderSource(shader, 1, &shaderSource, NULL);
   glCompileShader(shader);
   int success;
   glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
   if(!success)
   {
      char infoLog[512];
      glGetShaderInfoLog(shader, 512, NULL, infoLog);
      fprintf(stderr, "OPENGL::SHADER::FAILED_TO_COMPILE:\n%s\n", infoLog);
      return 0u;
   }
   return shader;
}

unsigned int createVFshaderProgram (unsigned int vertexShader, unsigned int fragmentShader)
{
   unsigned int shaderProgram = glCreateProgram();
   glAttachShader(shaderProgram, vertexShader);
   glAttachShader(shaderProgram, fragmentShader);
   glLinkProgram(shaderProgram);
   int success;
   glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
   if(!success) {
      char infoLog[512];
      glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
      fprintf(stderr, "OPENGL::SHADER::FAILED_TO_LINK:\n%s\n", infoLog);
      return 0u;
   }
   return shaderProgram;
}

unsigned int createVGFshaderProgram (unsigned int vertexShader, unsigned int geometryShader, unsigned int fragmentShader)
{
   unsigned int shaderProgram = glCreateProgram();
   glAttachShader(shaderProgram, vertexShader);
   glAttachShader(shaderProgram, geometryShader);
   glAttachShader(shaderProgram, fragmentShader);
   glLinkProgram(shaderProgram);
   int success;
   glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
   if(!success) {
      char infoLog[512];
      glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
      fprintf(stderr, "OPENGL::SHADER::FAILED_TO_LINK:\n%s\n", infoLog);
      return 0u;
   }
   return shaderProgram;
}
