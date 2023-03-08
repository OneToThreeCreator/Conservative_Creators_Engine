/*
   Conservative Creator's Engine - open source engine for making games.
   Copyright © 2020-2023 Andrey Gaivoronskiy

   This file is part of Conservative Creator's Engine.

   Conservative Creator's Engine is free software: you can redistribute it and/or modify it under 
   the terms of the GNU Lesser General Public License as published by the Free Software Foundation,
   either version 2 of the License, or (at your option) any later version.

   Conservative Creator's Engine is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR 
   PURPOSE. See the GNU Lesser General Public License for more details.

   You should have received a copy of the GNU Lesser General Public License along
   with Conservative Creator's Engine. If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef SHADER_H
#define SHADER_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

#include <stdint.h>
#include <stddef.h>
#include <glad/gl.h>

GLuint cce__makeVFshaderProgram  (const char *vertexPath, const char *fragmentPath,
                                  const char *vertexShaderAdditionalString, const char *const fragmentShaderAdditionalString);
GLuint cce__makeVGFshaderProgram (const char *vertexPath, const char *geometryPath, const char *fragmentPath,
                                  const char *vertexShaderAdditionalString, const char *geometryShaderAdditionalString, const char *fragmentShaderAdditionalString);
char* cce__readTextFile (const char *const path, size_t spaceToLeave);
GLuint cce__compileShader (const char *shaderSource, GLenum shaderType);
GLuint cce__createVFshaderProgram (GLuint vertexShader, GLuint fragmentShader);
GLuint cce__createVGFshaderProgram (GLuint vertexShader, GLuint geometryShader, GLuint fragmentShader);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // SHADER_H
