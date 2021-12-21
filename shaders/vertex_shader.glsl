// For gl version 3.3 core and above
layout (location = 0)  in ivec2 aCoords;
layout (location = 1)  in ivec2 aSize;
layout (location = 2)  in float aLayer;
layout (location = 3)  in vec4  aTexturePieceCoords;
layout (location = 4)  in int   aTextureID;
layout (location = 5)  in int   aMoveGroupsID;
layout (location = 6)  in int   aIsGlobalOffset;
layout (location = 7)  in int   aExtentionGroupID;
layout (location = 8)  in int   aRotateGroupID;
layout (location = 9)  in int   aTextureOffsetID;
layout (location = 10) in int   aColorID;

out GS_IN
{
   vec2  HalfSize;
   vec4  TexturePieceCoords;
   float RotateAngleSin;
   float RotateAngleCos;
   vec2  RotationOffset;
   int   TextureID;
   vec4  Color;
} vs_to_gs;


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

uniform vec2  Step = vec2(16, 16);
uniform ivec2 GlobalMoveCoords = ivec2(0, 0);

void main()
{
   vs_to_gs.TextureID = aTextureID;
   int isMoveGroup1 = min(1, aMoveGroupsID);
   int isGlobalMoveGroup = (aIsGlobalOffset & GLOBAL_OFFSET_CONTROL_MASK) / GLOBAL_OFFSET_CONTROL_MASK;
   vec2 offset = MoveCoords[aMoveGroupsID - isMoveGroup1] * isMoveGroup1 + GlobalMoveCoords * isGlobalMoveGroup;
   gl_Position = vec4((aCoords.xy + offset.xy) / Step.xy, aLayer, 1.0f);
   int isExtentionGroup = min(1, aExtentionGroupID);
   vs_to_gs.HalfSize.xy = ((aSize.xy + (Extention[aExtentionGroupID - isExtentionGroup].xy * isExtentionGroup)) / Step.xy) * 0.5f;
   int isRotate = min(1, aRotateGroupID);
   vs_to_gs.RotateAngleSin = RotateAngleSin[aRotateGroupID - isRotate] * isRotate;
   vs_to_gs.RotateAngleCos = RotateAngleCos[aRotateGroupID - isRotate] * isRotate + (1 - isRotate);
   vs_to_gs.RotationOffset.xy = (RotationOffset[aRotateGroupID - isRotate].xy * isRotate) / Step.xy;
   int isTextureOffset = min(1, aTextureOffsetID);
   vec2 texturePieceSize = aTexturePieceCoords.zw - aTexturePieceCoords.xy;
   vs_to_gs.TexturePieceCoords = aTexturePieceCoords + (TextureOffset[aTextureOffsetID - isTextureOffset].xyxy * isTextureOffset * texturePieceSize.xyxy);
   int isColor = min(1, aColorID);
   vs_to_gs.Color = Colors[aColorID - isColor] * isColor;
}

