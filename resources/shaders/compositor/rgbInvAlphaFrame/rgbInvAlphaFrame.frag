#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D rgbaTexture;

void main()
{
    vec3 col = texture(rgbaTexture, TexCoords).rgb;
    float inv_alpha= 1.0 - texture(rgbaTexture, TexCoords).a;

    FragColor = vec4(col, inv_alpha);
} 