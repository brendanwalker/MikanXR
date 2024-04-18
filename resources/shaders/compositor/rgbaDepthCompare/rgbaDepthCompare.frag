#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D rgbaTextureA;
uniform sampler2D rgbaTextureB;
uniform sampler2D depthTextureA;
uniform sampler2D depthTextureB;

void main()
{
    vec4 colorA = texture(rgbaTextureA, TexCoords).rgba;
	vec4 colorB = texture(rgbaTextureB, TexCoords).rgba;
	float depthA = float( texture(depthTextureA, TexCoords).r );
	float depthB = float( texture(depthTextureB, TexCoords).r );

    FragColor = (depthA < depthB) ? colorA : colorB;
} 