// For gl version 3.3 core and above
out vec4 FragColor;

in vec4     Color;
flat in int TextureID; // From 1
in vec2     TextureCoord;

uniform sampler2DArray Textures;
const vec4 white = vec4(1.0f, 1.0f, 1.0f, 1.0f);

void main()
{
   int isTexture = min(TextureID, 1);
   FragColor = mix(white, texture(Textures, vec3(TextureCoord.xy, TextureID - isTexture)), isTexture) * mix(white, vec4(Color.xyz, 1.0f), Color.w);
}
