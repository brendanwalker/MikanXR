//-- includes -----
#include "MathGLM.h"
#include <glm/gtx/intersect.hpp>

//-- public methods -----
float glm_vec3_normalize_with_default(glm::vec3& v, const glm::vec3& default_result)
{
	const float length = glm::length(v);

	// Use the default value if v is too tiny
	v = (length > k_normal_epsilon) ? (v / length) : default_result;

	return length;
}

glm::vec3 glm_vec3_lerp(const glm::vec3& a, const glm::vec3& b, const float u)
{
	return a * (1.f - u) + b * u;
}

glm::mat4 glm_mat4_from_pose(const glm::quat& orientation, const glm::vec3& position)
{
	glm::mat4 rot = glm::mat4_cast(orientation);
	glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
	glm::mat4 transform = trans * rot;

	return transform;
}

bool glm_closest_point_on_ray_to_ray(
	const glm::vec3& ray1_start,
	const glm::vec3& ray1_direction,
	const glm::vec3& ray2_start,
	const glm::vec3& ray2_direction,
	float& out_ray1_closest_time,
	glm::vec3& out_ray1_closest_point)
{
	//see https://palitri.com/vault/stuff/maths/Rays%20closest%20point.pdf
	const glm::vec3& a= ray1_direction;
	const glm::vec3& b= ray2_direction;
	const glm::vec3& c= ray2_start - ray1_start;

	const float a_dot_a= glm::dot(a, a);
	const float a_dot_b= glm::dot(a, b);
	const float a_dot_c= glm::dot(a, c);
	const float b_dot_b= glm::dot(b, b);
	const float b_dot_c= glm::dot(b, c);

	const float denomenator = a_dot_a*b_dot_b - a_dot_b*a_dot_b;
	if (!is_nearly_zero(a_dot_a) && // i.e. ray1_direction != 0
		!is_nearly_zero(b_dot_b) && // i.e. ray2_direction != 0
		!is_nearly_zero(denomenator)) // i.e. rays not parallel
	{
		const float numerator = a_dot_c*b_dot_b - a_dot_b*b_dot_c;

		out_ray1_closest_time= numerator / denomenator;
		out_ray1_closest_point= ray1_start + ray1_direction * out_ray1_closest_time;
	}
	else
	{
		out_ray1_closest_time= 0.f;
		out_ray1_closest_point= ray1_start;
		return false;
	}
}