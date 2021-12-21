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
      fprintf(stderr, "OPENGL::SHADER::VERTEX::FAILED_TO_LOAD:\n%s", vertexPath);
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
      fprintf(stderr, "OPENGL::SHADER::FRAGMENT::FAILED_TO_LOAD:\n%s", fragmentPath);
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
      fprintf(stderr, "OPENGL::SHADER::VERTEX::FAILED_TO_LOAD:\n%s", vertexPath);
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
      fprintf(stderr, "OPENGL::SHADER::GEOMETRY::FAILED_TO_LOAD:\n%s", fragmentPath);
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
      fprintf(stderr, "OPENGL::SHADER::FRAGMENT::FAILED_TO_LOAD:\n%s", fragmentPath);
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
   size_t modifiedStringLength = strlen(shaderSrc) + strlen(shaderAdditionalString) + 20;
   char *shaderModifiedSrc = (char*) malloc((modifiedStringLength + 1/*\0*/) * sizeof(char));
   strcpy(shaderModifiedSrc, "#version ");
   shortToString(shaderModifiedSrc, shaderVersion, " core\n");
   strcat(shaderModifiedSrc, shaderAdditionalString);
   strcat(shaderModifiedSrc, shaderSrc);
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
      fprintf(stderr, "OPENGL::SHADER::FAILED_TO_COMPILE:\n%s", infoLog);
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
      fprintf(stderr, "OPENGL::SHADER::FAILED_TO_LINK:\n%s", infoLog);
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
      fprintf(stderr, "OPENGL::SHADER::FAILED_TO_LINK:\n%s", infoLog);
      return 0u;
   }
   return shaderProgram;
}
