#version 330 core
uniform sampler2D rgbTexture;

in vec2 vTexCoords;

out vec4 FragColor;

void main()
{
	vec4 t0 = texture(rgbTexture, vTexCoords);
	vec3 t1 = t0.xyz;
	vec4 t2 = vec4(t1, 1.0);
	FragColor = t2;
}
