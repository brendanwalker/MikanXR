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
	vec3 clientColor = texture(clientRGBA, TexCoords).rgb;
	float clientInvAlpha = 1.0 - texture(clientRGBA, TexCoords).a;

	FragColor = (clientZ < videoZ) ? vec4(clientColor, clientInvAlpha) : vec4(0.0);
}