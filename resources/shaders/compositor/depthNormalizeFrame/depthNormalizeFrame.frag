#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform float zNear;
uniform float zFar;
uniform sampler2D depthTexture;

void main()
{
	float depth = texture(depthTexture, TexCoords).x;
    float zNorm= (2.0 * zNear) / (zFar + zNear - depth * (zFar - zNear));

    FragColor = vec4(zNorm, zNorm, zNorm, 1.0);
} 