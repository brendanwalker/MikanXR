#version 330 core
uniform sampler2D rgbaTexture;
uniform float time;
uniform float grainIntensity;

in vec2 vTexCoords;

float hash(vec2 p, float t)
{
	vec2 t0 = (p * vec2(443.8975, 397.2973));
	vec2 t1 = (t0 + vec2(t));
	vec2 t2 = fract(t1);
	vec2 t3 = (t2 + vec2(19.19));
	float t4 = dot(t2, t3);
	vec2 t5 = (t2 + vec2(t4));
	float t6 = t5.x;
	float t7 = t5.y;
	float t8 = (t6 + t7);
	float t9 = (t8 * t6);
	float t10 = fract(t9);
	return t10;
}

float noise(vec2 uv, float t)
{
	float t0 = hash(uv, t);
	float t1 = (t0 * 2.0);
	float t2 = (t1 - 1.0);
	return t2;
}

out vec4 FragColor;

void main()
{
	vec4 t0 = texture(rgbaTexture, vTexCoords);
	vec3 t1 = t0.xyz;
	vec2 t2 = vec2(textureSize(rgbaTexture, 0));
	vec2 t3 = (vTexCoords * t2);
	float t4 = noise(t3, time);
	float t5 = (grainIntensity * 0.03);
	float t6 = (t4 * t5);
	vec2 t7 = (t3 / vec2(8.0));
	vec2 t8 = floor(t7);
	vec2 t9 = (t8 * vec2(8.0));
	float t10 = noise(t9, time);
	float t11 = (grainIntensity * 0.04);
	float t12 = (t10 * t11);
	float t13 = (t6 + t12);
	float t14 = dot(t1, vec3(0.299, 0.587, 0.114));
	float t15 = (t14 * 4.0);
	float t16 = (1.0 - t14);
	float t17 = (t15 * t16);
	float t18 = (t13 * t17);
	vec3 t19 = (t1 + vec3(t18));
	vec3 t20 = clamp(t19, vec3(0.0), vec3(1.0));
	vec4 t21 = vec4(t20, t0.w);
	FragColor = t21;
}
