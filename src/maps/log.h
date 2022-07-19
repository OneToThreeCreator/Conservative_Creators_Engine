/*
    CoffeeChain - open source engine for making games.
    Copyright (C) 2020-2022 Andrey Givoronsky

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

#ifndef LOG_H
#define LOG_H

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

/* #ifdef __APPLE__
#include <OpenAL/al.h>
#include <OpenAL/alc.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif // __APPLE__
*/
#include "../external/glad.h"
#include <stdlib.h>

void cce__openGLErrorPrint (GLenum error, size_t line, const char *file);
void cce__criticalErrorPrint (const char *const msgAndFormat, ...);
void cce__infoPrint (const char *const msgAndFormat, ...);
void cce__errorPrint (const char *const msgAndFormat, ...);
//void cce__openALErrorPrint (ALCenum error);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif // LOG_H
