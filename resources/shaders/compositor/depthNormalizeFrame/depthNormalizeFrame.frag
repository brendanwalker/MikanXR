#version 330 core
uniform float zNear;
uniform float zFar;
uniform sampler2D depthTexture;

in vec2 vTexCoords;

out vec4 FragColor;

void main()
{
	float t0 = (zNear * 2.0);
	float t1 = (zFar + zNear);
	vec4 t2 = texture(depthTexture, vTexCoords);
	float t3 = (zFar - zNear);
	float t4 = (t2.x * t3);
	float t5 = (t1 - t4);
	float t6 = (t0 / t5);
	vec2 t7 = vec2(t6, t6);
	vec2 t8 = vec2(t6, 1.0);
	vec4 t9 = vec4(t7, t8);
	FragColor = t9;
}
