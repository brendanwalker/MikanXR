#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform float zMin;
uniform float zMax;
uniform sampler2D depthTexture;

void main()
{
	float depth = float( texture(depthTexture, TexCoords).r );
    float zNorm= 1.0 - max(min((depth - zMin) / (zMax - zMin), 1), 0);

    FragColor = vec4(zNorm, zNorm, zNorm, 1.0);
} 