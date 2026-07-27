//-- includes -----
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "MathGLM.h"
#include "MathUtility.h"
#include "Transform.h"

#include "unit_test.h"

//-- public interface -----
bool run_math_glm_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("math_glm")
	UNIT_TEST_MODULE_CALL_TEST(math_glm_test_intersect_obb_with_ray);
	UNIT_TEST_MODULE_CALL_TEST(math_glm_test_intersect_aabb_with_ray);
	UNIT_TEST_MODULE_CALL_TEST(math_glm_test_mat4_composite);
	UNIT_TEST_MODULE_CALL_TEST(math_glm_test_closest_point_on_triangle);
	UNIT_TEST_MODULE_CALL_TEST(math_glm_test_point_aabb_distance_sq);
	UNIT_TEST_MODULE_END()
}

//-- private functions -----
bool intersect_unit_obb_outside(const glm::mat4& xform, const glm::vec3& normal)
{
	bool success= false;

	glm::vec3 xformedNormal= glm::normalize(xform * glm::vec4(normal, 0.f));
	glm::vec3 rayStart= xform * glm::vec4(normal * 2.f, 1.f);
	glm::vec3 rayDirection= xform * glm::vec4(normal * -1.f, 0.f);

	float intDistance;
	glm::vec3 intPoint;
	glm::vec3 intNormal;
	bool bIntersects= glm_intersect_obb_with_ray(rayStart, rayDirection, glm::vec3(-1, -1, -1), // Minimum X,Y,Z
												 glm::vec3(1, 1, 1),                            // Maximum X,Y,Z
												 xform, intDistance, intPoint, intNormal);

	success= bIntersects;
	assert(success);
	success= is_nearly_equal(intDistance, 1.f, k_normal_epsilon);
	assert(success);
	success= glm_vec3_is_nearly_equal(intPoint, xformedNormal, k_normal_epsilon);
	assert(success);
	success= glm_vec3_is_nearly_equal(intNormal, xformedNormal, k_normal_epsilon);

	return success;
}

bool intersect_unit_aabb_outside(const glm::vec3& normal)
{
	bool success= false;

	glm::vec3 rayStart= glm::vec4(normal * 2.f, 1.f);
	glm::vec3 rayDirection= normal * -1.f;

	float intDistance;
	glm::vec3 intPoint;
	bool bIntersects= glm_intersect_aabb_with_ray(rayStart, rayDirection, glm::vec3(-1, -1, -1), // Minimum X,Y,Z
												  glm::vec3(1, 1, 1),                            // Maximum X,Y,Z
												  intDistance);

	success= bIntersects;
	assert(success);
	success= is_nearly_equal(intDistance, 1.f, k_normal_epsilon);

	return success;
}

bool intersect_unit_obb_inside(const glm::mat4& xform, const glm::vec3& rayDirection)
{
	bool success= false;

	glm::vec3 xformRayStart= xform * glm::vec4(glm::vec3(0.f), 1.f);
	glm::vec3 xformedRayDirection= xform * glm::vec4(rayDirection, 0.f);
	glm::vec3 xformedNormal= xform * glm::vec4(rayDirection * -1.f, 0.f);

	float intDistance;
	glm::vec3 intPoint;
	glm::vec3 intNormal;
	bool bIntersects=
		glm_intersect_obb_with_ray(xformRayStart, xformedRayDirection, glm::vec3(-1, -1, -1), // Minimum X,Y,Z
								   glm::vec3(1, 1, 1),                                        // Maximum X,Y,Z
								   xform, intDistance, intPoint, intNormal);

	success= bIntersects;
	assert(success);
	success= is_nearly_equal(intDistance, 0.f, k_normal_epsilon);
	assert(success);
	success= glm_vec3_is_nearly_equal(intPoint, xformRayStart, k_normal_epsilon);
	assert(success);
	success= glm_vec3_is_nearly_equal(intNormal, xformedNormal, k_normal_epsilon);

	return success;
}

bool intersect_unit_aabb_inside(const glm::vec3& rayDirection)
{
	bool success= false;

	glm::vec3 rayStart= glm::vec4(glm::vec3(0.f), 1.f);

	float intDistance;

	bool bIntersects= glm_intersect_aabb_with_ray(rayStart, rayDirection, glm::vec3(-1, -1, -1), // Minimum X,Y,Z
												  glm::vec3(1, 1, 1),                            // Maximum X,Y,Z
												  intDistance);

	success= bIntersects;
	assert(success);
	success= is_nearly_equal(intDistance, 0.f, k_normal_epsilon);

	return success;
}

bool no_intersect_unit_obb(const glm::mat4& xform, const glm::vec3& normal, const glm::vec3& rayDir)
{
	bool success= false;

	glm::vec3 rayStart= xform * glm::vec4(normal * 2.f, 1.f);
	glm::vec3 rayDirection= xform * glm::vec4(rayDir, 0.f);

	float intDistance;
	glm::vec3 intPoint;
	glm::vec3 intNormal;
	bool bIntersects= glm_intersect_obb_with_ray(rayStart, rayDirection, glm::vec3(-1, -1, -1), // Minimum X,Y,Z
												 glm::vec3(1, 1, 1),                            // Maximum X,Y,Z
												 xform, intDistance, intPoint, intNormal);

	success= !bIntersects;
	assert(success);

	return success;
}

bool no_intersect_unit_aabb(const glm::vec3& normal, const glm::vec3& rayDir)
{
	bool success= false;

	glm::vec3 rayStart= glm::vec4(normal * 2.f, 1.f);

	float intDistance;
	bool bIntersects= glm_intersect_aabb_with_ray(rayStart, rayDir, glm::vec3(-1, -1, -1), // Minimum X,Y,Z
												  glm::vec3(1, 1, 1),                      // Maximum X,Y,Z
												  intDistance);

	success= !bIntersects;
	assert(success);

	return success;
}

bool math_glm_test_intersect_obb_with_ray()
{
	UNIT_TEST_BEGIN("intersect obb with ray")

	const int k_xform_count= 4;
	glm::mat4 test_xforms[k_xform_count]= {
		glm::mat4(1.f),
		glm::rotate(glm::mat4(1.f), k_real_quarter_pi, glm::vec3(1.f, 0.f, 0.f)),
		glm::rotate(glm::mat4(1.f), k_real_quarter_pi, glm::vec3(0.f, 1.f, 0.f)),
		glm::rotate(glm::mat4(1.f), k_real_quarter_pi, glm::vec3(0.f, 0.f, 1.f)),
	};

	// Test unit cube intersection from outside
	for (int i= 0; i < k_xform_count; ++i)
	{
		const glm::mat4& xform= test_xforms[i];

		success= intersect_unit_obb_outside(xform, glm::vec3(1.f, 0.f, 0.f));
		assert(success);
		success= intersect_unit_obb_outside(xform, glm::vec3(-1.f, 0.f, 0.f));
		assert(success);
		success= intersect_unit_obb_outside(xform, glm::vec3(0.f, 1.f, 0.f));
		assert(success);
		success= intersect_unit_obb_outside(xform, glm::vec3(0.f, -1.f, 0.f));
		assert(success);
		success= intersect_unit_obb_outside(xform, glm::vec3(0.f, 0.f, 1.f));
		assert(success);
		success= intersect_unit_obb_outside(xform, glm::vec3(0.f, 0.f, -1.f));
		assert(success);
	}

	// Test unit cube from inside
	for (int i= 0; i < k_xform_count; ++i)
	{
		const glm::mat4& xform= test_xforms[i];

		success= intersect_unit_obb_inside(xform, glm::vec3(1.f, 0.f, 0.f));
		assert(success);
		success= intersect_unit_obb_inside(xform, glm::vec3(-1.f, 0.f, 0.f));
		assert(success);
		success= intersect_unit_obb_inside(xform, glm::vec3(0.f, 1.f, 0.f));
		assert(success);
		success= intersect_unit_obb_inside(xform, glm::vec3(0.f, -1.f, 0.f));
		assert(success);
		success= intersect_unit_obb_inside(xform, glm::vec3(0.f, 0.f, 1.f));
		assert(success);
		success= intersect_unit_obb_inside(xform, glm::vec3(0.f, 0.f, -1.f));
		assert(success);
	}

	// Test unit cube no intersection with ray pointing away from cube
	for (int i= 0; i < k_xform_count; ++i)
	{
		const glm::mat4& xform= test_xforms[i];

		success= no_intersect_unit_obb(xform, glm::vec3(1.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f));
		assert(success);
		success= no_intersect_unit_obb(xform, glm::vec3(-1.f, 0.f, 0.f), glm::vec3(-1.f, 0.f, 0.f));
		assert(success);
		success= no_intersect_unit_obb(xform, glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
		assert(success);
		success= no_intersect_unit_obb(xform, glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
		assert(success);
		success= no_intersect_unit_obb(xform, glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 1.f));
		assert(success);
		success= no_intersect_unit_obb(xform, glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 0.f, -1.f));
		assert(success);
	}

	// Test unit cube no intersection with ray pointing parallel to cube
	for (int i= 0; i < k_xform_count; ++i)
	{
		const glm::mat4& xform= test_xforms[i];

		success= no_intersect_unit_obb(xform, glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
		assert(success);
		success= no_intersect_unit_obb(xform, glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
		assert(success);
		success= no_intersect_unit_obb(xform, glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
		assert(success);
		success= no_intersect_unit_obb(xform, glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
		assert(success);
		success= no_intersect_unit_obb(xform, glm::vec3(0.f, 0.f, 1.f), glm::vec3(1.f, 0.f, 0.f));
		assert(success);
		success= no_intersect_unit_obb(xform, glm::vec3(0.f, 0.f, -1.f), glm::vec3(1.f, 0.f, 0.f));
		assert(success);
	}

	UNIT_TEST_COMPLETE()
}

bool math_glm_test_intersect_aabb_with_ray()
{
	UNIT_TEST_BEGIN("intersect aabb with ray")

	// Test unit cube intersection from outside
	success= intersect_unit_aabb_outside(glm::vec3(1.f, 0.f, 0.f));
	assert(success);
	success= intersect_unit_aabb_outside(glm::vec3(-1.f, 0.f, 0.f));
	assert(success);
	success= intersect_unit_aabb_outside(glm::vec3(0.f, 1.f, 0.f));
	assert(success);
	success= intersect_unit_aabb_outside(glm::vec3(0.f, -1.f, 0.f));
	assert(success);
	success= intersect_unit_aabb_outside(glm::vec3(0.f, 0.f, 1.f));
	assert(success);
	success= intersect_unit_aabb_outside(glm::vec3(0.f, 0.f, -1.f));
	assert(success);

	// Test unit cube from inside
	success= intersect_unit_aabb_inside(glm::vec3(1.f, 0.f, 0.f));
	assert(success);
	success= intersect_unit_aabb_inside(glm::vec3(-1.f, 0.f, 0.f));
	assert(success);
	success= intersect_unit_aabb_inside(glm::vec3(0.f, 1.f, 0.f));
	assert(success);
	success= intersect_unit_aabb_inside(glm::vec3(0.f, -1.f, 0.f));
	assert(success);
	success= intersect_unit_aabb_inside(glm::vec3(0.f, 0.f, 1.f));
	assert(success);
	success= intersect_unit_aabb_inside(glm::vec3(0.f, 0.f, -1.f));
	assert(success);

	// Test unit cube no intersection with ray pointing away from cube
	success= no_intersect_unit_aabb(glm::vec3(1.f, 0.f, 0.f), glm::vec3(1.f, 0.f, 0.f));
	assert(success);
	success= no_intersect_unit_aabb(glm::vec3(-1.f, 0.f, 0.f), glm::vec3(-1.f, 0.f, 0.f));
	assert(success);
	success= no_intersect_unit_aabb(glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	assert(success);
	success= no_intersect_unit_aabb(glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, -1.f, 0.f));
	assert(success);
	success= no_intersect_unit_aabb(glm::vec3(0.f, 0.f, 1.f), glm::vec3(0.f, 0.f, 1.f));
	assert(success);
	success= no_intersect_unit_aabb(glm::vec3(0.f, 0.f, -1.f), glm::vec3(0.f, 0.f, -1.f));
	assert(success);

	// Test unit cube no intersection with ray pointing parallel to cube
	success= no_intersect_unit_aabb(glm::vec3(1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	assert(success);
	success= no_intersect_unit_aabb(glm::vec3(-1.f, 0.f, 0.f), glm::vec3(0.f, 1.f, 0.f));
	assert(success);
	success= no_intersect_unit_aabb(glm::vec3(0.f, 1.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
	assert(success);
	success= no_intersect_unit_aabb(glm::vec3(0.f, -1.f, 0.f), glm::vec3(0.f, 0.f, 1.f));
	assert(success);
	success= no_intersect_unit_aabb(glm::vec3(0.f, 0.f, 1.f), glm::vec3(1.f, 0.f, 0.f));
	assert(success);
	success= no_intersect_unit_aabb(glm::vec3(0.f, 0.f, -1.f), glm::vec3(1.f, 0.f, 0.f));
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool math_glm_test_closest_point_on_triangle()
{
	UNIT_TEST_BEGIN("closest point on triangle")

	// Triangle in the z=0 plane: v0=(0,0,0), v1=(1,0,0), v2=(0,1,0)
	GlmTriangle tri;
	tri.v0= glm::vec3(0.f, 0.f, 0.f);
	tri.v1= glm::vec3(1.f, 0.f, 0.f);
	tri.v2= glm::vec3(0.f, 1.f, 0.f);

	// Point above the face projects straight down onto the face
	success= glm_vec3_is_nearly_equal(glm_closest_point_on_triangle(tri, glm::vec3(0.25f, 0.25f, 1.f)),
									  glm::vec3(0.25f, 0.25f, 0.f), k_normal_epsilon);
	assert(success);

	// Point in the vertex region outside v0 clamps to v0
	success= glm_vec3_is_nearly_equal(glm_closest_point_on_triangle(tri, glm::vec3(-1.f, -1.f, 0.f)),
									  glm::vec3(0.f, 0.f, 0.f), k_normal_epsilon);
	assert(success);

	// Point beyond v1 clamps to v1
	success= glm_vec3_is_nearly_equal(glm_closest_point_on_triangle(tri, glm::vec3(2.f, 0.f, 0.f)),
									  glm::vec3(1.f, 0.f, 0.f), k_normal_epsilon);
	assert(success);

	// Point below edge v0-v1 projects onto that edge
	success= glm_vec3_is_nearly_equal(glm_closest_point_on_triangle(tri, glm::vec3(0.5f, -1.f, 0.f)),
									  glm::vec3(0.5f, 0.f, 0.f), k_normal_epsilon);
	assert(success);

	// Midpoint of the hypotenuse edge lies on the triangle boundary and maps to itself
	success= glm_vec3_is_nearly_equal(glm_closest_point_on_triangle(tri, glm::vec3(0.5f, 0.5f, 0.f)),
									  glm::vec3(0.5f, 0.5f, 0.f), k_normal_epsilon);

	UNIT_TEST_COMPLETE()
}

bool math_glm_test_point_aabb_distance_sq()
{
	UNIT_TEST_BEGIN("point aabb distance squared")

	const glm::vec3 aabbMin(-1.f, -1.f, -1.f);
	const glm::vec3 aabbMax(1.f, 1.f, 1.f);

	// Inside the box -> zero distance
	success=
		is_nearly_equal(glm_point_aabb_distance_sq(glm::vec3(0.f, 0.f, 0.f), aabbMin, aabbMax), 0.f, k_normal_epsilon);
	assert(success);

	// Outside along a single axis -> squared axis gap (3 units past the +x face = 9)
	success=
		is_nearly_equal(glm_point_aabb_distance_sq(glm::vec3(4.f, 0.f, 0.f), aabbMin, aabbMax), 9.f, k_normal_epsilon);
	assert(success);

	// Outside a corner -> sum of squared axis gaps (2^2 + 2^2 + 2^2 = 12)
	success=
		is_nearly_equal(glm_point_aabb_distance_sq(glm::vec3(3.f, 3.f, 3.f), aabbMin, aabbMax), 12.f, k_normal_epsilon);

	UNIT_TEST_COMPLETE()
}

bool math_glm_test_mat4_composite()
{
	UNIT_TEST_BEGIN("mat4 composite")

	GlmTransform parentTransform(glm::vec3(0.f, 0.f, 0.f), glm::angleAxis(k_real_half_pi, glm::vec3(0.f, 0.f, 1.f)));
	GlmTransform childTransform(glm::vec3(1.f, 0.f, 0.f));
	glm::mat4 compositeTransform= glm_composite_xform(childTransform.getMat4(), parentTransform.getMat4());
	glm::vec3 x_axis= glm_mat4_get_x_axis(compositeTransform);
	glm::vec3 y_axis= glm_mat4_get_y_axis(compositeTransform);
	glm::vec3 z_axis= glm_mat4_get_z_axis(compositeTransform);
	glm::vec3 position= glm_mat4_get_position(compositeTransform);

	success= glm_vec3_is_nearly_equal(position, glm::vec3(0.f, 1.f, 0.f), k_normal_epsilon);
	assert(success);
	success= glm_vec3_is_nearly_equal(x_axis, glm::vec3(0.f, 1.f, 0.f), k_normal_epsilon);
	assert(success);
	success= glm_vec3_is_nearly_equal(y_axis, glm::vec3(-1.f, 0.f, 0.f), k_normal_epsilon);
	assert(success);
	success= glm_vec3_is_nearly_equal(z_axis, glm::vec3(0.f, 0.f, 1.f), k_normal_epsilon);
	assert(success);

	UNIT_TEST_COMPLETE()
}