//-- includes -----
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "MathUtility.h"
#include "unit_test.h"

//-- public interface -----
bool run_math_utility_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("math_utility")
		UNIT_TEST_MODULE_CALL_TEST(math_utility_test_remap_float_to_float);
		UNIT_TEST_MODULE_CALL_TEST(math_utility_test_remap_float_to_int);
		UNIT_TEST_MODULE_CALL_TEST(math_utility_test_remap_int_to_float);
		UNIT_TEST_MODULE_CALL_TEST(math_utility_test_remap_int_to_int);
		UNIT_TEST_MODULE_CALL_TEST(math_utility_test_wrap_lerpf);
	UNIT_TEST_MODULE_END()
}

//-- private functions -----
bool math_utility_test_remap_float_to_float()
{
	UNIT_TEST_BEGIN("remap float to float")

	// Test basic remapping from [0, 1] to [0, 100]
	float result = remap_float_to_float(0.f, 1.f, 0.f, 100.f, 0.5f);
	success = is_nearly_equal(result, 50.f, k_normal_epsilon);
	assert(success);

	// Test remapping with different input range [0, 10] to [0, 100]
	result = remap_float_to_float(0.f, 10.f, 0.f, 100.f, 5.f);
	success = is_nearly_equal(result, 50.f, k_normal_epsilon);
	assert(success);

	// Test remapping to negative range [0, 1] to [-100, 100]
	result = remap_float_to_float(0.f, 1.f, -100.f, 100.f, 0.5f);
	success = is_nearly_equal(result, 0.f, k_normal_epsilon);
	assert(success);

	// Test remapping with inverted input range [10, 0] to [0, 100]
	result = remap_float_to_float(10.f, 0.f, 0.f, 100.f, 5.f);
	success = is_nearly_equal(result, 50.f, k_normal_epsilon);
	assert(success);

	// Test remapping with inverted output range [0, 1] to [100, 0]
	result = remap_float_to_float(0.f, 1.f, 100.f, 0.f, 0.25f);
	success = is_nearly_equal(result, 75.f, k_normal_epsilon);
	assert(success);

	// Test clamping at lower bound
	result = remap_float_to_float(0.f, 10.f, 0.f, 100.f, -5.f);
	success = is_nearly_equal(result, 0.f, k_normal_epsilon);
	assert(success);

	// Test clamping at upper bound
	result = remap_float_to_float(0.f, 10.f, 0.f, 100.f, 15.f);
	success = is_nearly_equal(result, 100.f, k_normal_epsilon);
	assert(success);

	// Test zero input range (should return average of output range)
	result = remap_float_to_float(5.f, 5.f, 0.f, 100.f, 5.f);
	success = is_nearly_equal(result, 50.f, k_normal_epsilon);
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool math_utility_test_remap_float_to_int()
{
	UNIT_TEST_BEGIN("remap float to int")

	// Test basic remapping from [0, 1] to [0, 100]
	int result = remap_float_to_int(0.f, 1.f, 0, 100, 0.5f);
	success = (result == 50);
	assert(success);

	// Test remapping with different input range [0, 10] to [0, 100]
	result = remap_float_to_int(0.f, 10.f, 0, 100, 7.5f);
	success = (result == 75);
	assert(success);

	// Test remapping to negative range [0, 1] to [-100, 100]
	result = remap_float_to_int(0.f, 1.f, -100, 100, 0.75f);
	success = (result == 50);
	assert(success);

	// Test truncation behavior (0.49 should truncate to 49)
	result = remap_float_to_int(0.f, 1.f, 0, 100, 0.49f);
	success = (result == 49);
	assert(success);

	// Test clamping at lower bound
	result = remap_float_to_int(0.f, 10.f, 0, 100, -5.f);
	success = (result == 0);
	assert(success);

	// Test clamping at upper bound
	result = remap_float_to_int(0.f, 10.f, 0, 100, 15.f);
	success = (result == 100);
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool math_utility_test_remap_int_to_float()
{
	UNIT_TEST_BEGIN("remap int to float")

	// Test basic remapping from [0, 10] to [0, 100]
	float result = remap_int_to_float(0, 10, 0.f, 100.f, 5);
	success = is_nearly_equal(result, 50.f, k_normal_epsilon);
	assert(success);

	// Test remapping to negative range [0, 100] to [-1, 1]
	result = remap_int_to_float(0, 100, -1.f, 1.f, 50);
	success = is_nearly_equal(result, 0.f, k_normal_epsilon);
	assert(success);

	// Test remapping with negative input range [-10, 10] to [0, 100]
	result = remap_int_to_float(-10, 10, 0.f, 100.f, 0);
	success = is_nearly_equal(result, 50.f, k_normal_epsilon);
	assert(success);

	// Test remapping with inverted input range [10, 0] to [0, 100]
	result = remap_int_to_float(10, 0, 0.f, 100.f, 5);
	success = is_nearly_equal(result, 50.f, k_normal_epsilon);
	assert(success);

	// Test clamping at lower bound
	result = remap_int_to_float(0, 10, 0.f, 100.f, -5);
	success = is_nearly_equal(result, 0.f, k_normal_epsilon);
	assert(success);

	// Test clamping at upper bound
	result = remap_int_to_float(0, 10, 0.f, 100.f, 15);
	success = is_nearly_equal(result, 100.f, k_normal_epsilon);
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool math_utility_test_remap_int_to_int()
{
	UNIT_TEST_BEGIN("remap int to int")

	// Test basic remapping from [0, 10] to [0, 100]
	int result = remap_int_to_int(0, 10, 0, 100, 5);
	success = (result == 50);
	assert(success);

	// Test remapping to negative range [0, 100] to [-50, 50]
	result = remap_int_to_int(0, 100, -50, 50, 50);
	success = (result == 0);
	assert(success);

	// Test remapping with negative input range [-10, 10] to [0, 100]
	result = remap_int_to_int(-10, 10, 0, 100, 0);
	success = (result == 50);
	assert(success);

	// Test remapping with inverted input range [10, 0] to [0, 100]
	result = remap_int_to_int(10, 0, 0, 100, 5);
	success = (result == 50);
	assert(success);

	// Test remapping with inverted output range [0, 10] to [100, 0]
	result = remap_int_to_int(0, 10, 100, 0, 2);
	success = (result == 80);
	assert(success);

	// Test clamping at lower bound
	result = remap_int_to_int(0, 10, 0, 100, -5);
	success = (result == 0);
	assert(success);

	// Test clamping at upper bound
	result = remap_int_to_int(0, 10, 0, 100, 15);
	success = (result == 100);
	assert(success);

	// Test zero input range (should return average of output range)
	result = remap_int_to_int(5, 5, 0, 100, 5);
	success = (result == 50);
	assert(success);

	UNIT_TEST_COMPLETE()
}

bool math_utility_test_wrap_lerpf()
{
	UNIT_TEST_BEGIN("wrap lerpf")

	for (float u = 0.f; success && u <= 1.f; u += 0.1f)
	{
		float wrap_lerp_result = wrap_lerpf(0, 60.f, u, -180.f, 180.f);
		float lerp_result = lerpf(0.f, 60.f, u);

		success = is_nearly_equal(wrap_lerp_result, lerp_result, k_normal_epsilon);
		assert(success);
	}

	for (float u = 0.f; success && u <= 1.f; u += 0.1f)
	{
		float wrap_lerp_result = wrap_lerpf(-60, 60.f, u, -180.f, 180.f);
		float lerp_result = lerpf(-60.f, 60.f, u);

		success = is_nearly_equal(wrap_lerp_result, lerp_result, k_normal_epsilon);
		assert(success);
	}

	for (float u = 0.f; success && u <= 0.5f; u += 0.1f)
	{
		float wrap_lerp_result = wrap_lerpf(-170.f, 170.f, u, -180.f, 180.f);
		float lerp_result = wrap_range(lerpf(-170.f, -190.f, u), -180.f, 180.f);

		success = is_nearly_equal(wrap_lerp_result, lerp_result, k_normal_epsilon);
		assert(success);
	}
	for (float u = 0.5f; success && u <= 1.f; u += 0.1f)
	{
		float wrap_lerp_result = wrap_lerpf(-170.f, 170.f, u, -180.f, 180.f);
		float lerp_result = wrap_range(lerpf(190.f, 170.f, u), -180.f, 180.f);

		success = is_nearly_equal(wrap_lerp_result, lerp_result, k_normal_epsilon);
		assert(success);
	}

	UNIT_TEST_COMPLETE()
}