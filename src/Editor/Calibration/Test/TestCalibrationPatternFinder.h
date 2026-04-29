#pragma once
#include "CalibrationPatternFinder.h"
#include "glm/ext/matrix_double4x4.hpp"

// Test double: returns a fixed camera-to-pattern transform; never needs a video frame.
class TestCalibrationPatternFinder : public CalibrationPatternFinder
{
public:
	TestCalibrationPatternFinder(int frameWidth, int frameHeight, const glm::dmat4& apertureToPatternXform)
		: CalibrationPatternFinder(frameWidth, frameHeight)
		, m_cameraToPatternXform(apertureToPatternXform) {}

	bool estimateNewCalibrationPatternPose(glm::dmat4& outXform) override
	{
		outXform = m_cameraToPatternXform;
		return true;
	}

	// Unused pure-virtual stubs
	eCalibrationPatternType getCalibrationPatternType() const override { return eCalibrationPatternType::mode_charuco; }
	bool findNewCalibrationPattern(float) override { return false; }
	bool fetchLastFoundCalibrationPattern(t_opencv_point2d_list&, t_opencv_pointID_list&, cv::Point2f[4]) override { return false; }

private:
	glm::dmat4 m_cameraToPatternXform;
};
