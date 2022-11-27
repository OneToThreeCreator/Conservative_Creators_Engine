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

in vec2 aCoords;

uniform isamplerBuffer ElementInfo;
uniform usamplerBuffer ElementData;

layout (shared) uniform Variables
{
   mat2x4 Transform     [257]; // Don't use vec3 in uniform blocks
   ivec2  Offset        [256]; // Avoid rotation of offsets
   vec4   Colors        [128]; // Half normal group size. Mostly unused anyway
   ivec2  TextureOffset [256];
};

uniform mat3 ViewMatrix = mat3(vec3(0.0625f, 0, 0), vec3(0, 1.0f/9.0f, 0), vec3(0, 0, 1));

out vec2 TextureCoord;
flat out int TextureID;
out vec4 Color;

void main()
{
   ivec2 pos;
   uvec2 size;
   uvec4 texCoordsAndSize;
   uvec4 transformIDs;
   uint  texOffsetID;
   uint  colorID;
   uint  isGlobalOffset;
   {
      ivec4 data1 = texelFetch(ElementInfo, gl_InstanceID);
      pos = data1.xy;
      data1.zw += 1 << 15;
      uvec4 elemData = texelFetch(ElementData, data1.z);
      uvec4 data2 = texelFetch(ElementData, textureSize(ElementData) - data1.w);
      TextureID = int(data2.w);
      texCoordsAndSize.xyz = data2.xyz & 0xFFFu;
      texCoordsAndSize.w = uint(dot(data2.xyz >> 12u, uvec3(1u, 16u, 256u)));
      transformIDs = elemData & UCHAR_MAX;
      elemData = elemData >> 8u;
      size = elemData.xy;
      texOffsetID = elemData.z;
      colorID = elemData.w & 0x7Fu;
      isGlobalOffset = (elemData.w & 0x80u) << 1u;
   }
   vec3 coords = vec3(aCoords * size, 1);
   mat3 transform;
   transform[0] = vec3(1, 0, 0);
   transform[1] = vec3(0, 1, 0);
   transform[2] = vec3(0, 0, 1);
   for (uint i = 0u; i < 4u; ++i)
   {
      mat3 tmp;
      tmp[0] = Transform[transformIDs[i]][0].xyz;
      tmp[1] = Transform[transformIDs[i]][1].xyz;
      tmp[2] = vec3(0, 0, 1);
      transform *= tmp;
      pos += Offset[i];
   }
   //transform *= mat3(Transform[isGlobalOffset][0], Transform[isGlobalOffset][1], vec3(0, 0, 1)); // Global transform (applied last, separation with translation is not needed)
   coords *= transform;
   coords.xy += pos;
   coords *= ViewMatrix;
   gl_Position = vec4(coords.xy * 0.5f, 0, 1);
   
   Color = Colors[colorID];
   
   vec2 textureCoords = max(aCoords, 0.0) * vec2(float(texCoordsAndSize.z), float(texCoordsAndSize.w)) * inverseTextureSize;
   TextureCoord = textureCoords + vec2(TextureOffset[texOffsetID] * ivec2(texCoordsAndSize.zw) + ivec2(texCoordsAndSize.xy)) * inverseTextureSize;
}
