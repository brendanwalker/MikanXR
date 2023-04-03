#pragma once

//-- includes -----
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "MathUtility.h"

//-- macros -----
#define assert_glm_vector3f_is_normalized(v) assert(is_nearly_equal(v.length(), 1.f, k_normal_epsilon))
#define assert_glm_vectors_are_perpendicular(a,b) assert(is_nearly_equal(glm::dot(a,b), 0.f, k_normal_epsilon))

//-- types -----
struct GlmTriangle
{
	glm::vec3 v0;
	glm::vec3 v1;
	glm::vec3 v2;
};

//-- interface -----
float glm_vec3_normalize_with_default(glm::vec3& v, const glm::vec3& default_result);
glm::vec3 glm_vec3_lerp(const glm::vec3& a, const glm::vec3& b, const float u);
glm::mat4 glm_scale_along_axis(const glm::vec3& axis, const float scale);
void glm_quat_to_euler_angles(
	const glm::quat& orientation,
	float& out_x_angle, float& out_y_angle, float& out_z_angle);
glm::mat4 glm_composite_xform(const glm::mat4& first, const glm::mat4& second);
glm::mat4 glm_mat4_from_pose(const glm::quat& orientation, const glm::vec3& position);
glm::vec3 glm_mat4_forward(const glm::mat4& xform);
glm::vec3 glm_mat4_up(const glm::mat4& xform);
glm::vec3 glm_mat4_right(const glm::mat4& xform);
glm::vec3 glm_mat4_position(const glm::mat4& xform);
void glm_xform_points(const glm::mat4& xform, glm::vec3* points, size_t point_count);
void glm_xform_vectors(const glm::mat4& xform, glm::vec3* points, size_t point_count);
bool glm_closest_point_on_ray_to_ray(
	const glm::vec3& ray1_start,
	const glm::vec3& ray1_direction,
	const glm::vec3& ray2_start,
	const glm::vec3& ray2_direction,
	float& out_ray1_closest_time,
	glm::vec3& out_ray1_closest_point);
bool glm_intersect_tri_with_ray(
	const GlmTriangle& tri, 
	const glm::vec3& ray_start, const glm::vec3& ray_direction,
	float& outIntDistance, glm::vec3& outIntPoint, glm::vec3& outIntNormal);