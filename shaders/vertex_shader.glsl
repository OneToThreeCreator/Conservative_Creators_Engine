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

// For gl version 3.2 core and above
layout (location = 0) in ivec2 aCoords;
layout (location = 1) in ivec2 aPosition;
layout (location = 2) in vec2  aTexturePieceCoords;
layout (location = 3) in ivec4 aMoveIDs;
layout (location = 4) in ivec4 aExtendIDs;
layout (location = 5) in ivec2 aTransform; // r - rotateID, g - isGlobalOffset
layout (location = 6) in vec2  aTextureFragmentSize;
layout (location = 7) in int   aTextureID;
layout (location = 8) in ivec4 aTextureOffsetIDs;
layout (location = 9) in ivec4 aColorIDs;

layout (packed) uniform Variables
{
   ivec2 MoveCoords     [255];
   ivec2 Extension      [255];
   vec4  Colors         [255];
   ivec2 TextureOffset  [255];
   ivec2 RotationOffset [255];
   vec2  RotateAngleSinCos [255]; // x - sin, y - cos
};

uniform vec2  InverseStep = vec2(0.125f, 0.125f);
uniform ivec2 GlobalMoveCoords = ivec2(0, 0);
uniform ivec2 MapOffset = ivec2(0, 0);

out vec2 TextureCoord;
flat out int TextureID;
out vec4 Color;

void main()
{
   TextureID = aTextureID;
   {
      ivec4 isTextureOffset = min(aTextureOffsetIDs, 1);
      ivec4 textureOffsetIDs = aTextureOffsetIDs - isTextureOffset;
      mat4x2 textureOffsets;
      textureOffsets[0] = TextureOffset[textureOffsetIDs.x];
      textureOffsets[1] = TextureOffset[textureOffsetIDs.y];
      textureOffsets[2] = TextureOffset[textureOffsetIDs.z];
      textureOffsets[3] = TextureOffset[textureOffsetIDs.w];
      TextureCoord = aTexturePieceCoords + ((textureOffsets * isTextureOffset) * aTextureFragmentSize);
   }
   {
      ivec4 isColor = min(aColorIDs, 1);
      ivec4 colorIDs = aColorIDs - isColor;
      mat4 colors;
      colors[0] = Colors[colorIDs.x];
      colors[1] = Colors[colorIDs.y];
      colors[2] = Colors[colorIDs.z];
      colors[3] = Colors[colorIDs.w];
      Color = colors * isColor;
   }
   vec2 offset;
   {
      ivec4  isMoveGroup = min(aMoveIDs, 1);
      ivec4  moveIDs = aMoveIDs - isMoveGroup;
      mat4x2 moveOffsets;
      moveOffsets[0] = MoveCoords[moveIDs.x];
      moveOffsets[1] = MoveCoords[moveIDs.y];
      moveOffsets[2] = MoveCoords[moveIDs.z];
      moveOffsets[3] = MoveCoords[moveIDs.w];

      offset = moveOffsets * isMoveGroup + GlobalMoveCoords * aTransform.g + MapOffset;
   }
   vec2 extension;
   {
      ivec4 isExtensionGroup = min(aExtendIDs, 1);
      ivec4 extendIDs = aExtendIDs - isExtensionGroup;
      mat4x2 extensions;
      extensions[0] = Extension[extendIDs.x];
      extensions[1] = Extension[extendIDs.y];
      extensions[2] = Extension[extendIDs.z];
      extensions[3] = Extension[extendIDs.w];
      extension = extensions * isExtensionGroup * sign(aCoords) * 0.5f;
   }
   {
       int  isRotate = min(aTransform.r, 1);
       mat2 rotate;
       rotate[0].yx = RotateAngleSinCos[aTransform.r - isRotate] * isRotate;
       rotate[0].x += (1 - isRotate);
       rotate[1] = rotate[0].yx;
       rotate[1].x *= -1;
       vec2  rotationOffset = (RotationOffset[aTransform.r - isRotate].xy * isRotate) * 0.5f * InverseStep;
       vec2 coords = aCoords * 0.5f * InverseStep + rotationOffset;
       //coords = vec2(coords.x * RotateAngleCos - coords.y * RotateAngleSin, coords.y * RotateAngleCos + coords.x * RotateAngleSin) - rotationOffset;
       vec2 position = (aPosition + offset + extension + abs(aCoords) * 0.5f) * InverseStep;
       gl_Position = vec4(coords * rotate + position - rotationOffset, 0.0f, 1.0f);
   }
}

