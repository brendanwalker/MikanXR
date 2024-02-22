#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform float zThreshold;
uniform sampler2D rgbaTexture;
uniform sampler2D depthTexture;

void main()
{
    vec3 col_rgb = texture(rgbaTexture, TexCoords).rgb;
	float col_a = texture(rgbaTexture, TexCoords).a;
	float depth = float( texture(depthTexture, TexCoords).r );
    float alpha= (depth < zThreshold) ? col_a : 0;

    FragColor = vec4(col_rgb, alpha);
} 