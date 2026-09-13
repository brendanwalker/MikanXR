#version 330 core
uniform sampler2D clientDepth;
uniform sampler2D videoDepth;
uniform sampler2D videoRGB;
uniform sampler2D clientRGBA;

in vec2 vTexCoords;

out vec4 FragColor;

void main()
{
	vec4 t0 = texture(clientDepth, vTexCoords);
	float t1 = min(t0.x, 0.9999);
	vec4 t2 = texture(videoDepth, vTexCoords);
	vec4 t3 = texture(videoRGB, vTexCoords);
	vec3 t4 = t3.xyz;
	vec4 t5 = texture(clientRGBA, vTexCoords);
	vec3 t6 = t5.xyz;
	vec3 t7 = mix(t4, t6, vec3(t5.w));
	vec3 t8 = ((t1 < t2.x) ? t7 : t4);
	vec4 t9 = vec4(t8, 1.0);
	FragColor = t9;
}
