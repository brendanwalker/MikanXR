#version 330 core
uniform sampler2D rgbTexture;
uniform sampler2D distortion;

in vec2 vTexCoords;

out vec4 FragColor;

void main()
{
	vec4 t0 = texture(distortion, vTexCoords);
	vec2 t1 = t0.xy;
	vec4 t2 = texture(rgbTexture, t1);
	vec3 t3 = t2.xyz;
	vec4 t4 = vec4(t3, 1.0);
	FragColor = t4;
}
