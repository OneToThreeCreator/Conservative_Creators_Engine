#ifndef SHADER_H
#define SHADER_H
#include "external/glad.h"
unsigned int makeVFshaderProgram  (const char *const vertexPath, const char *const fragmentPath, uint16_t shadersVersion,
                                   const char *const vertexShaderAdditionalString, const char *const fragmentShaderAdditionalString);
unsigned int makeVGFshaderProgram (const char *const vertexPath, const char *const geometryPath, const char *const fragmentPath, uint16_t shadersVersion,
                                   const char *const vertexShaderAdditionalString, const char *const geometryShaderAdditionalString, const char *const fragmentShaderAdditionalString);
char* fileRead (const char *const path);
char* addStringsInShader (uint16_t shadersVersion, const char *const shaderAdditionalString, const char *const shaderSrc);
unsigned int compileShader (const char *shaderSource, GLenum shaderType);
unsigned int createVFshaderProgram (unsigned int vertexShader, unsigned int fragmentShader);
unsigned int createVGFshaderProgram (unsigned int vertexShader, unsigned int geometryShader, unsigned int fragmentShader);
#endif // SHADER_H
