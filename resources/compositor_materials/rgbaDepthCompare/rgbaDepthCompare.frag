#version 330 core
uniform sampler2D depthTextureA;
uniform sampler2D depthTextureB;
uniform sampler2D rgbaTextureA;
uniform sampler2D rgbaTextureB;

in vec2 vTexCoords;

out vec4 FragColor;

void main()
{
	vec4 t0 = texture(depthTextureA, vTexCoords);
	vec4 t1 = texture(depthTextureB, vTexCoords);
	vec4 t2 = texture(rgbaTextureA, vTexCoords);
	vec4 t3 = texture(rgbaTextureB, vTexCoords);
	vec4 t4 = ((t0.x < t1.x) ? t2 : t3);
	FragColor = t4;
}
