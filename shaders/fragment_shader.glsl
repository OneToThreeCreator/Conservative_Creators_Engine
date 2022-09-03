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

#version 140 core

out vec4 FragColor;

in vec4     Color;
flat in int TextureID; // From 1
in vec2     TextureCoord;

uniform sampler2DArray Textures;
const vec4 white = vec4(1.0f, 1.0f, 1.0f, 1.0f);

void main()
{
   int isTexture = min(TextureID, 1);
   FragColor = mix(white, texture(Textures, vec3(TextureCoord.xy, TextureID - isTexture)), isTexture) * Color;
}
