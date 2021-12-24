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

#include "engine_common.h"

void cce_setWindowParameters__glfw (cce_enum parameter, uint32_t a, uint32_t b);
void showWindow__glfw (void);
void toWindow__glfw (void);
void toFullscreen__glfw (void);
void swapBuffers__glfw (void);
struct cce_uvec2 getCurrentAspectRatio__glfw (void);

int initEngine__glfw (const char *label, uint16_t globalBoolsQuantity);
void engineUpdate__glfw (void);
void terminateEngine__glfw (void);