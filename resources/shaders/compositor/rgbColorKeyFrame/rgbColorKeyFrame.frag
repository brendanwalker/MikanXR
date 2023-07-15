#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform vec3 colorKey;
uniform sampler2D colorKeyTexture;

void main()
{
    vec3 col = texture(colorKeyTexture, TexCoords).rgb;
    float alpha= (col == colorKey) ? 0.0 : 1.0;

    FragColor = vec4(col, alpha);
} 