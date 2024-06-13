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
bool glm_vec3_is_nearly_equal(const glm::vec3& a, const glm::vec3& b, const float epsilon);
float glm_vec3_normalize_with_default(glm::vec3& v, const glm::vec3& default_result);
glm::vec3 glm_vec3_lerp(const glm::vec3& a, const glm::vec3& b, const float u);
glm::mat4 glm_scale_along_axis(const glm::vec3& axis, const float scale);
void glm_quat_to_euler_angles(
	const glm::quat& orientation,
	float& out_x_radians, float& out_y_radians, float& out_z_radians);
void glm_euler_angles_to_mat3(
	float x_radians, float y_radians, float z_radians,
	glm::mat3& out_orientation);
void glm_euler_angles_to_quat(
	float x_radians, float y_radians, float z_radians,
	glm::quat& out_orientation);
glm::mat4 glm_composite_xform(const glm::mat4& first, const glm::mat4& second);
glm::quat glm_composite_rotation(const glm::quat& first, const glm::quat& second);
glm::mat4 glm_relative_xform(const glm::mat4& parentWorldXform, const glm::mat4& childWorldXform);
glm::mat4 glm_mat4_from_pose(const glm::quat& orientation, const glm::vec3& position);

glm::vec3 glm_mat4_get_x_axis(const glm::mat4& xform);
glm::vec3 glm_mat4_get_y_axis(const glm::mat4& xform);
glm::vec3 glm_mat4_get_z_axis(const glm::mat4& xform);
glm::vec3 glm_mat4_get_position(const glm::mat4& xform);

void glm_mat4_set_x_axis(glm::mat4& xform, const glm::vec3& v);
void glm_mat4_set_y_axis(glm::mat4& xform, const glm::vec3& v);
void glm_mat4_set_z_axis(glm::mat4& xform, const glm::vec3& v);
void glm_mat4_set_rotation(glm::mat4& xform, const glm::mat3& R);
void glm_mat4_set_position(glm::mat4& xform, const glm::vec3& v);

void glm_xform_points(const glm::mat4& xform, glm::vec3* points, size_t point_count);
void glm_xform_vectors(const glm::mat4& xform, glm::vec3* points, size_t point_count);
bool glm_closest_point_on_ray_to_point(
	const glm::vec3& ray_start,
	const glm::vec3& ray_direction,
	const glm::vec3& point,
	float& out_ray_closest_time,
	glm::vec3& out_ray_closest_point);
bool glm_closest_point_on_ray_to_ray(
	const glm::vec3& ray1_start,
	const glm::vec3& ray1_direction,
	const glm::vec3& ray2_start,
	const glm::vec3& ray2_direction,
	float& out_ray1_closest_time,
	glm::vec3& out_ray1_closest_point);
bool glm_intersect_plane_with_ray(
	const glm::vec3& point_on_plane, const glm::vec3& plane_normal,
	const glm::vec3& ray_start, const glm::vec3& ray_direction,
	float& outIntDistance, glm::vec3& outIntPoint);
bool glm_intersect_tri_with_ray(
	const GlmTriangle& tri, 
	const glm::vec3& ray_start, const glm::vec3& ray_direction,
	float& outIntDistance, glm::vec3& outIntPoint, glm::vec3& outIntNormal);
bool glm_intersect_disk_with_ray(
	const glm::vec3 ray_start,		// Ray origin, in world space
	const glm::vec3 ray_direction,	// Ray direction, in world space. 
	const glm::vec3 disk_center,	// Disk center, in world space
	const glm::vec3 disk_normal,	// Disk normal, in world space
	const float disk_radius,
	float& outIntDistance,			// Output: distance between ray_origin and the intersection with the OBB
	glm::vec3& outIntPoint,			// Output: intersection point on the surface of the OBB
	glm::vec3& outIntNormal);		// Output: intersection normal on the surface of the OBB
bool glm_intersect_obb_with_ray(
	const glm::vec3 ray_start,		// Ray origin, in world space
	const glm::vec3 ray_direction,	// Ray direction, in world space. 
	const glm::vec3 aabb_min,		// Minimum X,Y,Z coords of the mesh when not transformed at all.
	const glm::vec3 aabb_max,		// Maximum X,Y,Z coords. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
	const glm::mat4 xform,			// Transformation applied to the mesh (which will thus be also applied to its bounding box)
	float& outIntDistance,			// Output: distance between ray_origin and the intersection with the OBB
	glm::vec3& outIntPoint,			// Output: intersection point on the surface of the OBB
	glm::vec3& outIntNormal);		// Output: intersection normal on the surface of the OBB
bool glm_intersect_aabb_with_ray(
	const glm::vec3 ray_start,		// Ray origin, in world space
	const glm::vec3 ray_direction,	// Ray direction, in world space. 
	const glm::vec3 aabb_min,		// Minimum X,Y,Z coords of the aabb
	const glm::vec3 aabb_max,		// Maximum X,Y,Z coords of the aabb. Often aabb_min*-1 if your mesh is centered, but it's not always the case.
	float& outIntDistance);			// Output: distance between ray_origin and the intersection with the AABB

void glm_sphere_union(
	const glm::vec3& c1, const float r1,
	const glm::vec3& c2, const float r2,
	glm::vec3& outC, float& outR);