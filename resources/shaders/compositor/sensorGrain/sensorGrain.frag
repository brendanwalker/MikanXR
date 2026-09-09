#version 330 core
uniform sampler2D rgbaTexture;
uniform float time;
uniform float grainIntensity;

in vec2 vTexCoords;

out vec4 FragColor;

void main()
{
	vec4 t0 = texture(rgbaTexture, vTexCoords);
	vec3 t1 = t0.xyz;
	vec2 t2 = vec2(textureSize(rgbaTexture, 0));
	vec2 t3 = (vTexCoords * t2);
	vec2 t4 = (t3 * vec2(443.8975, 397.2973));
	vec2 t5 = (t4 + vec2(time));
	vec2 t6 = fract(t5);
	vec2 t7 = (t6 + vec2(19.19));
	float t8 = dot(t6, t7);
	vec2 t9 = (t6 + vec2(t8));
	float t10 = t9.x;
	float t11 = t9.y;
	float t12 = (t10 + t11);
	float t13 = (t12 * t10);
	float t14 = fract(t13);
	float t15 = (t14 * 2.0);
	float t16 = (t15 - 1.0);
	float t17 = (grainIntensity * 0.03);
	float t18 = (t16 * t17);
	vec2 t19 = (t3 / vec2(8.0));
	vec2 t20 = floor(t19);
	vec2 t21 = (t20 * vec2(8.0));
	vec2 t22 = (t21 * vec2(443.8975, 397.2973));
	vec2 t23 = (t22 + vec2(time));
	vec2 t24 = fract(t23);
	vec2 t25 = (t24 + vec2(19.19));
	float t26 = dot(t24, t25);
	vec2 t27 = (t24 + vec2(t26));
	float t28 = t27.x;
	float t29 = t27.y;
	float t30 = (t28 + t29);
	float t31 = (t30 * t28);
	float t32 = fract(t31);
	float t33 = (t32 * 2.0);
	float t34 = (t33 - 1.0);
	float t35 = (grainIntensity * 0.04);
	float t36 = (t34 * t35);
	float t37 = (t18 + t36);
	float t38 = dot(t1, vec3(0.299, 0.587, 0.114));
	float t39 = (t38 * 4.0);
	float t40 = (1.0 - t38);
	float t41 = (t39 * t40);
	float t42 = (t37 * t41);
	vec3 t43 = (t1 + vec3(t42));
	vec3 t44 = clamp(t43, vec3(0.0), vec3(1.0));
	vec4 t45 = vec4(t44, t0.w);
	FragColor = t45;
}
