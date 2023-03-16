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

glm::vec3 glm_closest_point_between_rays(
	const glm::vec3& ray1_start,
	const glm::vec3& ray1_direction,
	const glm::vec3& ray2_start,
	const glm::vec3& ray2_direction)
{
	// Calculate the cross product of the two direction vectors to find 
	// the normal vector of the plane containing the two rays.
	glm::vec3 plane_normal = glm::cross(ray1_direction, ray2_direction);

	// If the normal vector is (nearly) zero, it means the two rays are parallel, so there is no closest point. 
	// In this case, you can return any point on one of the rays.
	const float normal_length= glm::length(plane_normal);
	if (normal_length <= k_normal_epsilon)
	{
		return ray1_start;
	}

	// Normalize the normal vector and use it to define a plane passing through the origin.
	plane_normal /= normal_length;

	// Calculate the closest point between the two rays by finding the intersection point 
	// between the plane and a line passing through the two starting points of the rays 
	// and perpendicular to the plane.
	const glm::vec3 ray1_end = ray1_start + ray1_direction;
	const glm::vec3 ray2_end = ray2_start + ray2_direction;
	const glm::vec3 line_direction = glm::cross(plane_normal, glm::cross(ray1_direction, plane_normal));
	const glm::vec3 line = glm::normalize(line_direction);

	const float ray2_time = 
		glm::dot(plane_normal, ray1_start) 
		/ glm::dot(line, ray1_start - ray2_start);
	const glm::vec3 closest_point = ray2_start + ray2_time * (ray2_end - ray2_start);

	return closest_point;
}