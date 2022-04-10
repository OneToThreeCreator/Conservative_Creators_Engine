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

#ifndef SHADER_H
#define SHADER_H
#include "external/glad.h"
#include <stdint.h>

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
