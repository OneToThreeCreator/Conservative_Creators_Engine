// For gl version 3.3 core and above
layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

out vec2     TextureCoord;
flat out int TextureID;
out vec4     Color;

in GS_IN
{
    vec2  HalfSize;
    vec4  TexturePieceCoords;
    float RotateAngleSin;
    float RotateAngleCos;
    vec2  RotationOffset;
    int   TextureID;
    vec4  Color;
} vs_to_gs[];

void convertToSquare (vec4 texturePieceCoords, vec4 glPosition)
{
   // n - negative, p - positive, d - distance, m - multiplied by
   vec2 ndmsin = (-(vs_to_gs[0].HalfSize) - vs_to_gs[0].RotationOffset) * vs_to_gs[0].RotateAngleSin;
   vec2 ndmcos = (-(vs_to_gs[0].HalfSize) - vs_to_gs[0].RotationOffset) * vs_to_gs[0].RotateAngleCos;
   vec2 pdmsin = ( (vs_to_gs[0].HalfSize) - vs_to_gs[0].RotationOffset) * vs_to_gs[0].RotateAngleSin;
   vec2 pdmcos = ( (vs_to_gs[0].HalfSize) - vs_to_gs[0].RotationOffset) * vs_to_gs[0].RotateAngleCos;
   
   vec2 rotationCenter = glPosition.xy + vs_to_gs[0].HalfSize + vs_to_gs[0].RotationOffset;
   TextureID = vs_to_gs[0].TextureID;
   Color     = vs_to_gs[0].Color;
   gl_Position = vec4(rotationCenter.x + ndmcos.x - ndmsin.y, rotationCenter.y + ndmsin.x + ndmcos.y, glPosition.zw);
   TextureCoord = texturePieceCoords.xy;
   EmitVertex();
   TextureID = vs_to_gs[0].TextureID;
   Color     = vs_to_gs[0].Color;
   gl_Position = vec4(rotationCenter.x + pdmcos.x - ndmsin.y, rotationCenter.y + pdmsin.x + ndmcos.y, glPosition.zw);
   TextureCoord = texturePieceCoords.zy;
   EmitVertex();
   TextureID = vs_to_gs[0].TextureID;
   Color     = vs_to_gs[0].Color;
   gl_Position = vec4(rotationCenter.x + ndmcos.x - pdmsin.y, rotationCenter.y + ndmsin.x + pdmcos.y, glPosition.zw);
   TextureCoord = texturePieceCoords.xw;
   EmitVertex();
   TextureID = vs_to_gs[0].TextureID;
   Color     = vs_to_gs[0].Color;
   gl_Position = vec4(rotationCenter.x + pdmcos.x - pdmsin.y, rotationCenter.y + pdmsin.x + pdmcos.y, glPosition.zw);
   TextureCoord = texturePieceCoords.zw;
   EmitVertex();
   EndPrimitive();
}

void main()
{
   convertToSquare(vs_to_gs[0].TexturePieceCoords, gl_in[0].gl_Position);
}
