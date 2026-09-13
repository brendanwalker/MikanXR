#version 330 core
uniform sampler2D clientDepth;
uniform sampler2D videoDepth;
uniform sampler2D clientRGBA;

in vec2 vTexCoords;

out vec4 FragColor;

void main()
{
	vec4 t0 = texture(clientDepth, vTexCoords);
	vec4 t1 = texture(videoDepth, vTexCoords);
	float t2 = min(t1.x, 0.9998);
	vec4 t3 = texture(clientRGBA, vTexCoords);
	vec3 t4 = t3.xyz;
	float t5 = (1.0 - t3.w);
	vec4 t6 = vec4(t4, t5);
	vec4 t7 = ((t0.x < t2) ? t6 : vec4(0.0, 0.0, 0.0, 0.0));
	FragColor = t7;
}
