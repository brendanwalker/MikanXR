#version 330 core
uniform sampler2D rgbTexture;
uniform float shadowStrength;

in vec2 vTexCoords;

out vec4 FragColor;

void main()
{
	vec4 t0 = texture(rgbTexture, vTexCoords);
	vec3 t1 = t0.xyz;
	vec3 t2 = mix(vec3(1.0), t1, vec3(shadowStrength));
	vec3 t3 = clamp(t2, vec3(0.0), vec3(1.0));
	vec4 t4 = vec4(t3, 1.0);
	FragColor = t4;
}
