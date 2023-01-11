#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D rgbaTexture;

void main()
{
    FragColor = texture(rgbaTexture, TexCoords).rgba;
} 