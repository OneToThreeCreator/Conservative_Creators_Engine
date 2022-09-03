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

#define USHORT_MAX 0xffff
#define UCHAR_MAX 0xff

layout (location = 0) in vec2 aCoords;

in int gl_InstanceID;

uniform usamplerBuffer ElementData;

layout (shared) uniform Variables
{
   layout(row_major) mat2x3 Transform     [257]; // Don't use vec3 in uniform blocks
   ivec2                    Offset        [256]; // Avoid rotation of offsets
   vec4                     Colors        [256];
   ivec2                    TextureOffset [256];
};

uniform mat3 ViewMatrix = mat3(vec3(0.125f, 0, 0), vec3(0, 0.125f, 0), vec3(0, 0, 1));

out vec2 TextureCoord;
flat out int TextureID;
out vec4 Color;

void main()
{
   ivec2 pos;
   uvec2 size;
   uvec2 texCoords;
   uvec4 transformIDs;
   uvec2 texOffsetIDs;
   uvec2 texSize;
   uvec2 colorIDs;
   uint  flags;
   {
      uint samplerPos = gl_InstanceID * 2;
      uvec4 data = texelFetch(ElementData, samplerPos);
      pos = ivec2(data.xy);
      mat2x4 intFromShort;
      intFromShort[0] = uvec2(data.zw, data.zw >> 8);
      intFromShort[1] = intFromShort[0] >> 16;
      intFromShort[0] = intFromShort[0] & UCHAR_MAX;
      intFromShort[1] = intFromShort[1] & UCHAR_MAX;
      transformIDs = uvec4(intFromShort[0].xz, intFromShort[1].xz);
      texOffsetIDs = intFromShort[0].yw;
      colorIDs = intFromShort[1].yw;
      
      data = texelFetch(ElementData, samplerPos + 1);
      intFromShort[0] = data.xyzw & USHORT_MAX;
      intFromShort[1] = data.xyzw >> 16;
      size = uvec2(intFromShort[0].x, intFromShort[1].x);
      texCoords = uvec2(intFromShort[0].y, intFromShort[1].y);
      texSize = uvec2(intFromShort[0].z, intFromShort[1].z);
      TextureID = intFromShort[0].w;
      flags = intFromShort[1].w & UCHAR_MAX
   }
   vec3 coords = vec3(aCoords * size, 1);
   mat3 transform;
   transform[0] = vec3(1, 0, 0);
   transform[1] = vec3(0, 1, 0);
   transform[2] = vec3(0, 0, 1);
   for (uint i = 0; i < 4; ++i)
   {
      mat3 tmp;
      tmp[0] = Transform[transformIDs[i]][0];
      tmp[1] = Transform[transformIDs[i]][1];
      tmp[2] = vec3(0, 0, 1);
      transform *= tmp;
      pos += Offset[i]
   }
   transform *= (Transform[min(flags & CCE_GLOBAL_OFFSET_MASK, 1) * 256]) // Global transform (applied last, separation with translation is not needed)
   coords = coords * transform;
   coords.xy += pos;
   coords *= ViewMatrix;
   gl_Position = vec4(coords.xy, 0, 1);
   
   vec3 color = vec3(1.0f, 1.0f, 1.0f);
   color = mix(color, Colors[colorIDs.x].xyz, Colors[colorIDs.x].w);
   color = mix(color, Colors[colorIDs.y].xyz, Colors[colorIDs.y].w);
   Color = vec4(color, 1.0f);
   
   vec3 textureCoords = min(aCoords, 0.0) + texSize * inverseTextureSize
   TextureCoord = textureCoords + (TextureOffset[texOffsetIDs.x] * texSize + TextureOffset[texOffsetIDs.y] * texSize + texCoords) * inverseTextureSize
}
