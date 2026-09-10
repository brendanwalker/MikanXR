#version 330 core
uniform sampler2D rgbaTexture;

in vec2 vTexCoords;

out vec4 FragColor;

void main()
{
	vec4 t0 = texture(rgbaTexture, vTexCoords);
	vec3 t1 = t0.xyz;
	float t2 = (1.0 - t0.w);
	vec4 t3 = vec4(t1, t2);
	FragColor = t3;
}
