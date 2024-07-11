#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D videoRGB;
uniform sampler2D videoDepth;
uniform sampler2D clientRGBA;
uniform sampler2D clientDepth;

const float gamma = 2.2;

vec3 toGamma(vec3 v) {
  return pow(v, vec3(1.0 / gamma));
}

void main()
{
    vec3 videoColor = texture(videoRGB, TexCoords).rgb;
	float videoZ = float( texture(videoDepth, TexCoords).r );
	vec3 clientColor = texture(clientRGBA, TexCoords).rgb;
	vec3 clientGammaColor= toGamma(clientColor);
	float clientInvAlpha = 1.0 - texture(clientRGBA, TexCoords).a;
	float clientZ = min(float( texture(clientDepth, TexCoords).r ), 0.9999);

	vec3 resultColor= (clientZ < videoZ) ? mix(videoColor, clientGammaColor, clientInvAlpha) : videoColor;
    FragColor = vec4(resultColor, 1.0);
} 