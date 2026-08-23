#include "TrackerPoseCalibratorTests.h"
#include "CameraMath.h"
#include "MathGLM.h"
#include "MathUtility.h"
#include "MathTypeConversion.h"
#include "MikanVideoSourceTypes.h"
#include "TestCalibrationPatternFinder.h"
#include "TestMonoLensTrackerPoseCalibrator.h"
#include "unit_test.h"

#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/quaternion.hpp"

#include <assert.h>
#include <math.h>

// ---- Helpers ----

static const double k_test_epsilon= 1e-6;

static bool dmat4_is_nearly_equal(const glm::dmat4& a, const glm::dmat4& b, double eps= k_test_epsilon)
{
	bool equal= true;
	for (int col= 0; col < 4; ++col)
		for (int row= 0; row < 4; ++row)
			if (fabs(a[col][row] - b[col][row]) > eps)
				equal= false;

	if (!equal)
	{
		fprintf(stdout, "    dmat4_is_nearly_equal FAILED (eps=%.2e)\n", eps);
		fprintf(stdout, "    Matrix A:\n");
		for (int row= 0; row < 4; ++row)
			fprintf(stdout, "      [%10.6f, %10.6f, %10.6f, %10.6f]\n", a[0][row], a[1][row], a[2][row], a[3][row]);
		fprintf(stdout, "    Matrix B:\n");
		for (int row= 0; row < 4; ++row)
			fprintf(stdout, "      [%10.6f, %10.6f, %10.6f, %10.6f]\n", b[0][row], b[1][row], b[2][row], b[3][row]);
		fprintf(stdout, "    Differing elements [col][row]:\n");
		for (int col= 0; col < 4; ++col)
			for (int row= 0; row < 4; ++row)
				if (fabs(a[col][row] - b[col][row]) > eps)
					fprintf(stdout, "      [%d][%d]: A=%.10f  B=%.10f  diff=%.2e\n", col, row, a[col][row], b[col][row],
							fabs(a[col][row] - b[col][row]));
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

	const glm::dmat4 cameraPuckXform_VRSpace= glm::dmat4(1.0);
	const glm::dmat4 matPuckXform_VRSpace= glm::dmat4(1.0);
	const glm::dmat4 apertureToPatternXform= make_puck_yaw_rot180();
	const glm::dvec3 matPuckOffsetMM= glm::dvec3(0.0, 0.0, 0.0);

	CameraPuckToApertureResults results;
	computeCameraPuckToApertureXform(cameraPuckXform_VRSpace, matPuckXform_VRSpace, apertureToPatternXform,
									 matPuckOffsetMM, results);
	assert(success);

	success= dmat4_is_nearly_equal(results.cameraPuckToApertureXform, glm::dmat4(1.0));
	assert(success);

	UNIT_TEST_COMPLETE()
}

// Test 2: Camera puck is 1m to the right (+X) of origin. The camera is optically
// at the origin (same position as the pattern / mat puck). The result should be
// translate(-1, 0, 0): the camera is 1m behind its puck in X.
bool tracker_pose_calibrator_test_known_translation()
{
	UNIT_TEST_BEGIN("known translation: camera 1m behind puck in X")

	const glm::dmat4 cameraPuckXform_VRSpace= glm::translate(glm::dmat4(1.0), glm::dvec3(1.0, 0.0, 0.0));
	const glm::dmat4 matPuckXform_VRSpace= glm::dmat4(1.0);
	const glm::dmat4 apertureToPatternXform= make_puck_yaw_rot180();
	const glm::dvec3 matPuckOffsetMM= glm::dvec3(0.0, 0.0, 0.0);

	CameraPuckToApertureResults results;
	computeCameraPuckToApertureXform(cameraPuckXform_VRSpace, matPuckXform_VRSpace, apertureToPatternXform,
									 matPuckOffsetMM, results);
	assert(success);

	const glm::dmat4 expected= glm::translate(glm::dmat4(1.0), glm::dvec3(-1.0, 0.0, 0.0));
	success= dmat4_is_nearly_equal(results.cameraPuckToApertureXform, expected);
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

	const glm::dmat4 cameraPuckXform_VRSpace= glm::dmat4(1.0);
	const glm::dmat4 matPuckXform_VRSpace= glm::dmat4(1.0);
	const glm::dmat4 apertureToPatternXform= make_puck_yaw_rot180();
	const glm::dvec3 matPuckOffsetMM= glm::dvec3(100.0, 0.0, 0.0);

	CameraPuckToApertureResults results;
	computeCameraPuckToApertureXform(cameraPuckXform_VRSpace, matPuckXform_VRSpace, apertureToPatternXform,
									 matPuckOffsetMM, results);
	assert(success);

	const glm::dmat4 expected= glm::translate(glm::dmat4(1.0), glm::dvec3(0.1, 0.0, 0.0));
	success= dmat4_is_nearly_equal(results.cameraPuckToApertureXform, expected);
	assert(success);

	UNIT_TEST_COMPLETE()
}

// ---- Integration test helpers ----

static MikanMonoIntrinsics make_default_mono_intrinsics()
{
	MikanMonoIntrinsics intrinsics= {};
	intrinsics.pixel_width= 1920;
	intrinsics.pixel_height= 1080;
	intrinsics.hfov= 90.0;
	intrinsics.vfov= 60.0;
	intrinsics.znear= 0.1;
	intrinsics.zfar= 100.0;
	return intrinsics;
}

// ---- Integration tests (MonoLensTrackerPoseCalibrator) ----

// Integration Test 1: identity — all poses are identity, no puck offset.
// computeCalibrationSample() should succeed and produce an identity cameraPuckToApertureXform.
bool tracker_pose_calibrator_integration_test_identity()
{
	UNIT_TEST_BEGIN("integration identity: full pipeline produces identity offset")

	const glm::dmat4 cameraPuckXform_VRSpace= glm::dmat4(1.0);
	const glm::dmat4 matPuckXform_VRSpace= glm::dmat4(1.0);
	const glm::dvec3 matPuckOffsetMM= glm::dvec3(0.0, 0.0, 0.0);
	const glm::dmat4 apertureToPatternXform= make_puck_yaw_rot180();

	TestMonoLensTrackerPoseCalibrator calibrator(cameraPuckXform_VRSpace, matPuckXform_VRSpace, matPuckOffsetMM,
												 new TestCalibrationPatternFinder(1920, 1080, apertureToPatternXform),
												 make_default_mono_intrinsics(), 1);

	success= calibrator.computeCalibrationSample();
	assert(success);
	assert(calibrator.hasValidCameraPuckToApertureXform());

	calibrator.recordCalibrationSample();
	assert(calibrator.hasFinishedSampling());

	MikanQuatd outRotation;
	MikanVector3d outTranslation;
	success= calibrator.computeAverageCameraPuckToApertureOffset(outRotation, outTranslation);
	assert(success);

	// Identity rotation: (w=1, x=0, y=0, z=0)
	success= fabs(outRotation.w - 1.0) < k_test_epsilon && fabs(outRotation.x) < k_test_epsilon
			 && fabs(outRotation.y) < k_test_epsilon && fabs(outRotation.z) < k_test_epsilon;
	assert(success);

	// Zero translation
	success= fabs(outTranslation.x) < k_test_epsilon && fabs(outTranslation.y) < k_test_epsilon
			 && fabs(outTranslation.z) < k_test_epsilon;
	assert(success);

	UNIT_TEST_COMPLETE()
}

// Integration Test 2: camera puck is 1m to the right (+X).
// The calibrated offset should be translate(-1, 0, 0).
bool tracker_pose_calibrator_integration_test_known_translation()
{
	UNIT_TEST_BEGIN("integration known translation: full pipeline produces (-1,0,0) offset")

	const glm::dmat4 cameraPuckXform_VRSpace= glm::translate(glm::dmat4(1.0), glm::dvec3(1.0, 0.0, 0.0));
	const glm::dmat4 matPuckXform_VRSpace= glm::dmat4(1.0);
	const glm::dvec3 matPuckOffsetMM= glm::dvec3(0.0, 0.0, 0.0);
	const glm::dmat4 apertureToPatternXform= make_puck_yaw_rot180();

	TestMonoLensTrackerPoseCalibrator calibrator(cameraPuckXform_VRSpace, matPuckXform_VRSpace, matPuckOffsetMM,
												 new TestCalibrationPatternFinder(1920, 1080, apertureToPatternXform),
												 make_default_mono_intrinsics(), 1);

	success= calibrator.computeCalibrationSample();
	assert(success);

	calibrator.recordCalibrationSample();
	assert(calibrator.hasFinishedSampling());

	MikanQuatd outRotation;
	MikanVector3d outTranslation;
	success= calibrator.computeAverageCameraPuckToApertureOffset(outRotation, outTranslation);
	assert(success);

	success= fabs(outTranslation.x - (-1.0)) < k_test_epsilon && fabs(outTranslation.y) < k_test_epsilon
			 && fabs(outTranslation.z) < k_test_epsilon;
	assert(success);

	UNIT_TEST_COMPLETE()
}

// Integration Test 3: puck offset of 100mm in X shifts camera 0.1m in +X.
bool tracker_pose_calibrator_integration_test_puck_offset()
{
	UNIT_TEST_BEGIN("integration puck offset: full pipeline produces (0.1,0,0) offset")

	const glm::dmat4 cameraPuckXform_VRSpace= glm::dmat4(1.0);
	const glm::dmat4 matPuckXform_VRSpace= glm::dmat4(1.0);
	const glm::dvec3 matPuckOffsetMM= glm::dvec3(100.0, 0.0, 0.0);
	const glm::dmat4 apertureToPatternXform= make_puck_yaw_rot180();
	TestMonoLensTrackerPoseCalibrator calibrator(cameraPuckXform_VRSpace, matPuckXform_VRSpace, matPuckOffsetMM,
												 new TestCalibrationPatternFinder(1920, 1080, apertureToPatternXform),
												 make_default_mono_intrinsics(), 1);

	success= calibrator.computeCalibrationSample();
	assert(success);

	calibrator.recordCalibrationSample();
	assert(calibrator.hasFinishedSampling());

	MikanQuatd outRotation;
	MikanVector3d outTranslation;
	success= calibrator.computeAverageCameraPuckToApertureOffset(outRotation, outTranslation);
	assert(success);

	success= fabs(outTranslation.x - 0.1) < k_test_epsilon && fabs(outTranslation.y) < k_test_epsilon
			 && fabs(outTranslation.z) < k_test_epsilon;
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
	UNIT_TEST_MODULE_CALL_TEST(tracker_pose_calibrator_integration_test_identity);
	UNIT_TEST_MODULE_CALL_TEST(tracker_pose_calibrator_integration_test_known_translation);
	UNIT_TEST_MODULE_CALL_TEST(tracker_pose_calibrator_integration_test_puck_offset);
	UNIT_TEST_MODULE_END()
}
