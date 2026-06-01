#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D videoDepth;
uniform sampler2D clientRGBA;
uniform sampler2D clientDepth;

void main()
{
	// Clip videoZ infinity slightly above client Z infinity so that video background wins
	float videoZ = min(texture(videoDepth, TexCoords).r, 0.9998);
	float clientZ = texture(clientDepth, TexCoords).r;
	vec4 clientColor = texture(clientRGBA, TexCoords).rgba;

	FragColor = (clientZ < videoZ) ? clientColor : vec4(0.0);
} 