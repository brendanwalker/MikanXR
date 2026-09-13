#version 330 core
uniform sampler2D rgbaTexture;
uniform sampler2D depthTexture;
uniform float zThreshold;

in vec2 vTexCoords;

out vec4 FragColor;

void main()
{
	vec4 t0 = texture(rgbaTexture, vTexCoords);
	vec3 t1 = t0.xyz;
	vec4 t2 = texture(depthTexture, vTexCoords);
	float t3 = ((t2.x < zThreshold) ? t0.w : 0.0);
	vec4 t4 = vec4(t1, t3);
	FragColor = t4;
}
