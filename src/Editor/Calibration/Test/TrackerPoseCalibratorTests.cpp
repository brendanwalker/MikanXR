#include "TrackerPoseCalibratorTests.h"
#include "CameraMath.h"
#include "MathGLM.h"
#include "MathUtility.h"
#include "MathTypeConversion.h"
#include "unit_test.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

#include <assert.h>
#include <math.h>

// ---- Helpers ----

static const double k_test_epsilon = 1e-6;

static bool dmat4_is_nearly_equal(const glm::dmat4& a, const glm::dmat4& b, double eps = k_test_epsilon)
{
	bool equal = true;
	for (int col = 0; col < 4; ++col)
		for (int row = 0; row < 4; ++row)
			if (fabs(a[col][row] - b[col][row]) > eps)
				equal = false;

	if (!equal)
	{
		fprintf(stdout, "    dmat4_is_nearly_equal FAILED (eps=%.2e)\n", eps);
		fprintf(stdout, "    Matrix A:\n");
		for (int row = 0; row < 4; ++row)
			fprintf(stdout, "      [%10.6f, %10.6f, %10.6f, %10.6f]\n",
				a[0][row], a[1][row], a[2][row], a[3][row]);
		fprintf(stdout, "    Matrix B:\n");
		for (int row = 0; row < 4; ++row)
			fprintf(stdout, "      [%10.6f, %10.6f, %10.6f, %10.6f]\n",
				b[0][row], b[1][row], b[2][row], b[3][row]);
		fprintf(stdout, "    Differing elements [col][row]:\n");
		for (int col = 0; col < 4; ++col)
			for (int row = 0; row < 4; ++row)
				if (fabs(a[col][row] - b[col][row]) > eps)
					fprintf(stdout, "      [%d][%d]: A=%.10f  B=%.10f  diff=%.2e\n",
						col, row, a[col][row], b[col][row], fabs(a[col][row] - b[col][row]));
	}

	return equal;
}

// puckYawRot180 is the canonical optical orientation: the calibration pattern
// faces the camera, so it is rotated 180 degrees about Y relative to the puck.
static glm::dmat4 make_puck_yaw_rot180()
{
	return glm::rotate(glm::dmat4(1.0), k_real64_pi, glm::dvec3(0.0, 1.0, 0.0));
}

// ---- Test cases ----

// Test 1: When the camera tracking puck and mat puck are both at the origin,
// zero physical offset, and the optical measurement is pure 180-degree Y rotation
// (the camera sees the pattern facing it from the same position), the result
// should be identity — the camera coincides exactly with its tracking puck.
bool tracker_pose_calibrator_test_identity()
{
	UNIT_TEST_BEGIN("identity: camera coincides with its puck")

	const glm::dmat4 cameraPuckXform_VRSpace = glm::dmat4(1.0);
	const glm::dmat4 matPuckXform_VRSpace    = glm::dmat4(1.0);
	const glm::dmat4 cameraToPatternXform    = make_puck_yaw_rot180();
	const glm::dvec3 matPuckOffsetMM         = glm::dvec3(0.0, 0.0, 0.0);

	glm::dmat4 result;
	success = computeCameraToPuckXformFromPoses(
		cameraPuckXform_VRSpace,
		matPuckXform_VRSpace,
		cameraToPatternXform,
		matPuckOffsetMM,
		result);
	assert(success);

	success = dmat4_is_nearly_equal(result, glm::dmat4(1.0));
	assert(success);

	UNIT_TEST_COMPLETE()
}

// Test 2: Camera puck is 1m to the right (+X) of origin. The camera is optically
// at the origin (same position as the pattern / mat puck). The result should be
// translate(-1, 0, 0): the camera is 1m behind its puck in X.
bool tracker_pose_calibrator_test_known_translation()
{
	UNIT_TEST_BEGIN("known translation: camera 1m behind puck in X")

	const glm::dmat4 cameraPuckXform_VRSpace =
		glm::translate(glm::dmat4(1.0), glm::dvec3(1.0, 0.0, 0.0));
	const glm::dmat4 matPuckXform_VRSpace = glm::dmat4(1.0);
	const glm::dmat4 cameraToPatternXform = make_puck_yaw_rot180();
	const glm::dvec3 matPuckOffsetMM      = glm::dvec3(0.0, 0.0, 0.0);

	glm::dmat4 result;
	success = computeCameraToPuckXformFromPoses(
		cameraPuckXform_VRSpace,
		matPuckXform_VRSpace,
		cameraToPatternXform,
		matPuckOffsetMM,
		result);
	assert(success);

	const glm::dmat4 expected =
		glm::translate(glm::dmat4(1.0), glm::dvec3(-1.0, 0.0, 0.0));
	success = dmat4_is_nearly_equal(result, expected);
	assert(success);

	UNIT_TEST_COMPLETE()
}

// Test 3: The physical calibration pattern is offset 100mm (+X) from the mat
// puck center. Both tracking pucks are at the origin. The camera sees the
// pattern at exactly puckYawRot180 (zero position offset in camera space).
//
// Because the pattern is 0.1m to the right of the mat puck (after the Y/Z swap,
// matPuckOffsetMM.x -> VR X), and the camera is co-located with the pattern,
// the camera ends up at (0.1, 0, 0) in VR space. With the camera puck at origin,
// the resulting offset is translate(0.1, 0, 0).
bool tracker_pose_calibrator_test_puck_offset()
{
	UNIT_TEST_BEGIN("puck offset: 100mm X shifts camera 0.1m from puck")

	const glm::dmat4 cameraPuckXform_VRSpace = glm::dmat4(1.0);
	const glm::dmat4 matPuckXform_VRSpace    = glm::dmat4(1.0);
	const glm::dmat4 cameraToPatternXform    = make_puck_yaw_rot180();
	const glm::dvec3 matPuckOffsetMM         = glm::dvec3(100.0, 0.0, 0.0);

	glm::dmat4 result;
	success = computeCameraToPuckXformFromPoses(
		cameraPuckXform_VRSpace,
		matPuckXform_VRSpace,
		cameraToPatternXform,
		matPuckOffsetMM,
		result);
	assert(success);

	const glm::dmat4 expected =
		glm::translate(glm::dmat4(1.0), glm::dvec3(0.1, 0.0, 0.0));
	success = dmat4_is_nearly_equal(result, expected);
	assert(success);

	UNIT_TEST_COMPLETE()
}

// ---- Module entry ----

bool run_tracker_pose_calibrator_unit_tests()
{
	UNIT_TEST_MODULE_BEGIN("tracker_pose_calibrator")
		UNIT_TEST_MODULE_CALL_TEST(tracker_pose_calibrator_test_identity);
		UNIT_TEST_MODULE_CALL_TEST(tracker_pose_calibrator_test_known_translation);
		UNIT_TEST_MODULE_CALL_TEST(tracker_pose_calibrator_test_puck_offset);
	UNIT_TEST_MODULE_END()
}
