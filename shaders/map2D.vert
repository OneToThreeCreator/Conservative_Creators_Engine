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
uniform isamplerBuffer ElementData;

uniform mat3 CameraTransform;

uniform mat3 ViewTransform;

out vec2 TextureCoord;
flat out int TextureID;
flat out vec4 Color;

void main()
{
   ivec2 pos, groupPos;
   uvec4 texCoordsAndSize;
   vec2  rotation;
   uint  texOffsetID;
   uint  isMovedByCamera;
   uint  texID;
   int   isFlipped;
   {
      ivec4 data = texelFetch(ElementInfo, gl_InstanceID);
      pos = data.xy;
      data.zw += (1 << 15);
      texOffsetID = uint(data.z) >> 8u;
      data = texelFetch(ElementData, data.w);
      // Unpacking
      uvec4 data2 = uvec4(uint(data.x), uint(data.y), uint(data.z), uint(data.w)) & 0xFFFFu;
      data >>= 16;
      groupPos = data.xy;
      rotation = data.zw * (1.0f/32767.0f);
      uvec4 bitwiseOps;
      bitwiseOps.xz = data2.xy >> 8u;
      bitwiseOps.yw = data2.xy & 0xFFu;
      texCoordsAndSize.xw = data2.xz & 0x0FFFu;
      texCoordsAndSize.yz = ((data2.xz & 0xF000u) >> 4u) | bitwiseOps.zw;
      Color = vec4(bitwiseOps.xyz, data2.w) * (1.0f/255.0f);
      TextureID = max(int(data2.w & 0x3FFFu) - 255, 0); // To encode alpha value when texture is not used and pass cameraMoved and flip flags
      isMovedByCamera = data2.w >> 15u;
      isFlipped = 1 - int((data2.w >> 13u) & 0x2u);
   }
   vec3 coords = vec3(aCoords * texCoordsAndSize.zw + pos, 1);
   coords.x *= isFlipped;
   mat3 transform = mat3(vec3(rotation.yx, 0), vec3(-rotation.x, rotation.y, 0), vec3(0, 0, 1));
   transform *= (CameraTransform * isMovedByCamera) + (mat3(vec3(1, 0, 0), vec3(0, 1, 0), vec3(0, 0, 1)) * (1u - isMovedByCamera));
   coords *= transform;
   coords += vec3(texCoordsAndSize.zw, 0.0f) * 0.5f;
   coords.xy += groupPos;
   coords *= ViewTransform;
   gl_Position = vec4(coords.xy, 0, 1);
   
   TextureCoord = (max(aCoords * 2, 0.0) * vec2(texCoordsAndSize.zw) + vec2(texCoordsAndSize.xy)) * inverseTextureSize;
}
