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
//layout (location = 3) in ivec4 aMoveGroupsIDs [4];
layout (location = 3) in ivec4 aTransform; // r - rotateID, g - isGlobalOffset, z - moveID, q - extendID (I know, this is not very good idea)
layout (location = 4) in ivec2 aTextureFragmentSize;
layout (location = 5) in int  aTextureID;
layout (location = 6) in ivec2 aPaint;     // s - textureOffsetID, t - colorID
/*
layout (location = 3) in int   aTextureID;
layout (location = 4) in int   aMoveGroupsID;
layout (location = 5) in int   aIsGlobalOffset;
layout (location = 6) in int   aExtentionGroupID;
layout (location = 7) in int   aRotateGroupID;
layout (location = 8) in int   aTextureOffsetID;
layout (location = 9) in int   aColorID;
*/

layout (packed) uniform Variables
{
   vec4  Colors         [255];
   ivec2 MoveCoords     [255];
   ivec2 Extention      [255];
   ivec2 TextureOffset  [255];
   ivec2 RotationOffset [255];
   float RotateAngleSin [255];
   float RotateAngleCos [255];
};

uniform vec2  InverseStep = vec2(0.125f, 0.125f);
uniform ivec2 GlobalMoveCoords = ivec2(0, 0);
uniform ivec2 MapOffset = ivec2(0, 0);

/*out GS_IN
{
   vec2  HalfSize;
   vec4  TexturePieceCoords;
   float RotateAngleSin;
   float RotateAngleCos;
   vec2  RotationOffset;
   int   TextureID;
   vec4  Color;
} vs_to_gs;*/
out vec2 TextureCoord;
flat out int TextureID;
out vec4 Color;

void main()
{
   TextureID = aTextureID;
   int  isTextureOffset = min(aPaint.s, 1);
   TextureCoord = aTexturePieceCoords + (TextureOffset[aPaint.s - isTextureOffset] * isTextureOffset * aTextureFragmentSize);
   int  isColor = min(aPaint.t, 1);
   Color = Colors[aPaint.t - isColor] * isColor;
   
   int   isRotate = min(aTransform.r, 1);
   int   isMoveGroup1 = min(aTransform.z, 1);
   vec2  offset = MoveCoords[aTransform.z - isMoveGroup1] * isMoveGroup1 + GlobalMoveCoords * aTransform.g + MapOffset;
   int   isExtentionGroup = min(aTransform.q, 1);
   vec2  extention = Extention[aTransform.q - isExtentionGroup] * isExtentionGroup * sign(aCoords);
   float RotateAngleSin =  RotateAngleSin[aTransform.r - isRotate]    * isRotate;
   float RotateAngleCos =  RotateAngleCos[aTransform.r - isRotate]    * isRotate + (1 - isRotate);
   vec2  rotationOffset = (RotationOffset[aTransform.r - isRotate].xy * isRotate) * 0.5f * InverseStep;
   vec2  coords = aCoords * 0.5f * InverseStep + rotationOffset;
   coords = vec2(coords.x * RotateAngleCos - coords.y * RotateAngleSin, coords.y * RotateAngleCos + coords.x * RotateAngleSin) - rotationOffset;
   coords += (aPosition + offset + extention + abs(aCoords) * 0.5f) * InverseStep;
   gl_Position = vec4(coords, 0.0f, 1.0f);
}

