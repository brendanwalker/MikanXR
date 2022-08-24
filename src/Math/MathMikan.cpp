#include "MathMikan.h"
#include "MathUtility.h"
#include "MathTypeConversion.h"
#include <glm/gtx/euler_angles.hpp>

void MikanOrientationToEulerAngles(
	const MikanVector3f& x_axis, const MikanVector3f& y_axis, const MikanVector3f& z_axis,
	float& out_x_angle, float& out_y_angle, float& out_z_angle)
{
	const glm::mat3 R= glm::mat3(
		MikanVector3f_to_glm_vec3(x_axis), 
		MikanVector3f_to_glm_vec3(y_axis),
		MikanVector3f_to_glm_vec3(z_axis));

	float xRadians= 0, yRadians= 0, zRadians= 0;
	glm::extractEulerAngleXYZ(glm::mat4(R), xRadians, yRadians, zRadians);

	out_x_angle = xRadians * k_radians_to_degreees;
	out_y_angle = yRadians * k_radians_to_degreees;
	out_z_angle = zRadians * k_radians_to_degreees;
}

void EulerAnglesToMikanOrientation(
	const float x_angle, const float y_angle, const float z_angle,
	MikanVector3f& out_x_axis, MikanVector3f& out_y_axis, MikanVector3f& out_z_axis)
{
	const float xRadians = x_angle * k_degrees_to_radians;
	const float yRadians = y_angle * k_degrees_to_radians;
	const float zRadians = z_angle * k_degrees_to_radians;
	const glm::mat3 R = glm::mat3(glm::eulerAngleXYZ(xRadians, yRadians, zRadians));

	out_x_axis = glm_vec3_to_MikanVector3f(R[0]);
	out_y_axis = glm_vec3_to_MikanVector3f(R[1]);
	out_z_axis = glm_vec3_to_MikanVector3f(R[2]);
}
