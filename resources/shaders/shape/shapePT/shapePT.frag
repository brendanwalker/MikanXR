#version 330 core
uniform sampler2D rgbaTexture;

in vec2 vTexCoords;

out vec4 FragColor;

void main()
{
	vec4 t0 = texture(rgbaTexture, vTexCoords);
	FragColor = t0;
}
