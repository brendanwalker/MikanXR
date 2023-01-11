#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D rgbTexture;
uniform sampler2D distortion;

void main()
{
    vec2 offset = texture(distortion, TexCoords.xy).rg;
    vec3 col = texture(rgbTexture, offset).rgb;

    FragColor = vec4(col, 1.0);
} 