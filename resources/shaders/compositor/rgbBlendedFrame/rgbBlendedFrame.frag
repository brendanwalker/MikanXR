#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D rgbTexture;

void main()
{
    vec3 col = texture(rgbTexture, TexCoords).rgb;

    FragColor = vec4(col, 0.8);
} 