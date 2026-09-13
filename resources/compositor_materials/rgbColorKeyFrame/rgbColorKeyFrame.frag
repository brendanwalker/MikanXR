#version 330 core
uniform sampler2D colorKeyTexture;
uniform vec3 colorKey;

in vec2 vTexCoords;

out vec4 FragColor;

void main()
{
	vec4 t0 = texture(colorKeyTexture, vTexCoords);
	vec3 t1 = t0.xyz;
	float t2 = ((t1 == colorKey) ? 0.0 : 1.0);
	vec4 t3 = vec4(t1, t2);
	FragColor = t3;
}
