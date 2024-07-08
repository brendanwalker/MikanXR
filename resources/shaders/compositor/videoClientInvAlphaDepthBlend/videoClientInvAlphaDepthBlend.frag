#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D videoRGB;
uniform sampler2D videoDepth;
uniform sampler2D clientRGBA;
uniform sampler2D clientDepth;

void main()
{
    vec3 videoColor = texture(videoRGB, TexCoords).rgb;
	float videoZ = float( texture(videoDepth, TexCoords).r );
	vec3 clientColor = texture(clientRGBA, TexCoords).rgb;
	float clientInvAlpha = 1.0 - texture(clientRGBA, TexCoords).a;
	float clientZ = min(float( texture(clientDepth, TexCoords).r ), 0.9999);

	vec3 resultColor= (clientZ < videoZ) ? mix(videoColor, clientColor, clientInvAlpha) : videoColor;
    FragColor = vec4(resultColor, 1.0);
} 