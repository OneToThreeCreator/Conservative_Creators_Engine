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

#version 150 core

#define UCHAR_MAX 0xFFu
#define PI 3.14159265f

in vec2 aCoords;

uniform isamplerBuffer ElementInfo;
uniform usamplerBuffer ElementData;

uniform ivec2 TextureOffset[256];

uniform mat3 globalTransform = mat3(vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1));

uniform mat3 ViewMatrix = mat3(vec3(0.0625f, 0, 0), vec3(0, 1.0f/9.0f, 0), vec3(0, 0, 1));

out vec2 TextureCoord;
flat out int TextureID;
flat out vec4 Color;

void main()
{
   ivec2 pos;
   uvec4 texCoordsAndSize;
   uvec4 transformIDs;
   vec2  rotation;
   uint  texOffsetID;
   uint  isGlobalOffset;
   uint  texID;
   {
      ivec4 data1 = texelFetch(ElementInfo, gl_InstanceID);
      pos = data1.xy;
      data1.zw += (1 << 15);
      texOffsetID = uint(data1.z) >> 8u;
      rotation = (uvec2(data1.z + 64, data1.z) & 0xFFu) * (PI/128.0f);
      uvec4 data2 = texelFetch(ElementData, data1.w & 0x7FFF);
      isGlobalOffset = uint(data1.w) >> 15u;
      // Unpacking data2
      uvec4 bitwiseOps;
      bitwiseOps.xz = data2.xy >> 8u;
      bitwiseOps.yw = data2.xy & 0xFFu;
      texCoordsAndSize.xw = data2.xz & 0x0FFFu;
      texCoordsAndSize.yz = ((data2.xz & 0xF000u) >> 4u) | bitwiseOps.zw;
      Color = vec4(bitwiseOps.xyz, data2.w) * (1.0f/255.0f);
      TextureID = max(int(data2.w) - 255, 0); // To encode alpha value when texture is not used
   }
   vec3 coords = vec3(aCoords * texCoordsAndSize.zw, 1);
   mat3 transform;
   transform[0] = vec3(sin(rotation), 0); // cos(x rad) = sin((x + pi/2) rad)
   transform[1] = vec3(-transform[0].y, transform[0].x, 0);
   transform[2] = vec3(0, 0, 1);
   transform *= (globalTransform * isGlobalOffset) + (mat3(vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)) * (1u - isGlobalOffset));
   coords *= transform;
   coords += vec3(texCoordsAndSize.zw, 0.0f) * 0.5f;
   coords.xy += pos;
   coords *= ViewMatrix;
   gl_Position = vec4(coords.xy * 0.5f, 0, 1);
   
   vec2 textureCoords = max(aCoords * 2, 0.0) * vec2(float(texCoordsAndSize.z), float(texCoordsAndSize.w)) * inverseTextureSize;
   TextureCoord = textureCoords + vec2(TextureOffset[texOffsetID] * ivec2(texCoordsAndSize.zw) + ivec2(texCoordsAndSize.xy)) * inverseTextureSize;
}
