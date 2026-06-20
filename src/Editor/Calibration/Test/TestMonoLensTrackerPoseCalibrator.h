#pragma once
#include "MonoLensTrackerPoseCalibrator.h"

// Test double: injects known VR-space poses and puck offset directly,
// bypassing CameraComponent / VRDevicePoseView.
class TestMonoLensTrackerPoseCalibrator : public MonoLensTrackerPoseCalibrator
{
public:
	TestMonoLensTrackerPoseCalibrator(
		const glm::dmat4& cameraPuckXform_VRSpace,
		const glm::dmat4& matPuckXform_VRSpace,
		const glm::dvec3& matPuckOffsetMM,
		class CalibrationPatternFinder* patternFinder,
		const MikanMonoIntrinsics& cameraIntrinsics,
		int desiredSampleCount)
		: MonoLensTrackerPoseCalibrator(patternFinder, cameraIntrinsics, desiredSampleCount)
		, m_cameraPuckXform(cameraPuckXform_VRSpace)
		, m_matPuckXform(matPuckXform_VRSpace)
		, m_puckOffsetMM(matPuckOffsetMM)
	{
	}

protected:
	bool fetchCameraPuckVRSpacePose(glm::dmat4& out) const override
	{
		out= m_cameraPuckXform;
		return true;
	}
	bool fetchMatPuckVRSpacePose(glm::dmat4& out) const override
	{
		out= m_matPuckXform;
		return true;
	}
	glm::dvec3 fetchMatPuckOffsetMM() const override { return m_puckOffsetMM; }

private:
	glm::dmat4 m_cameraPuckXform;
	glm::dmat4 m_matPuckXform;
	glm::dvec3 m_puckOffsetMM;
};
