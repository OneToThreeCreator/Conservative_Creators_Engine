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
