// Main implementation file that includes all calibration pattern finder implementations
#include "CalibrationPatternFinder.h"
#include "CalibrationPatternFinder_Aruco.h"
#include "CalibrationPatternFinder_Chessboard.h"
#include "CalibrationPatternFinder_Charuco.h"
#include "CalibrationRenderHelpers.h"
#include "CameraComponent.h"
#include "Colors.h"
#include "CameraMath.h"
#include "MikanTextRenderer.h"
#include "Logger.h"
#include "MarkerDefinition.h"
#include "MathOpenCV.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "StageComponent.h"
#include "TextStyle.h"
#include "TrackingSystemDefinition.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceView.h"

#include <algorithm>

#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/objdetect/aruco_detector.hpp"
#include "opencv2/objdetect/charuco_detector.hpp"

//-- CalibrationPatternFinder -----
CalibrationPatternFinder::CalibrationPatternFinder(
	VideoFrameDistortionView* distortionView)
	: m_distortionView(distortionView)
	, m_frameWidth(distortionView->getFrameWidth())
	, m_frameHeight(distortionView->getFrameHeight())
{
}

CalibrationPatternFinder::~CalibrationPatternFinder()
{
}

CalibrationPatternFinder* CalibrationPatternFinder::allocatePatternFinder(
	ProjectConfigConstPtr profileConfig,
	VideoFrameDistortionView* distortionView)
{
	switch (profileConfig->calibrationPatternType)
	{
	case eCalibrationPatternType::mode_chessboard:
		return
			new CalibrationPatternFinder_Chessboard(
				distortionView,
				profileConfig->chessbordRows,
				profileConfig->chessbordCols,
				profileConfig->squareLengthMM);
	case eCalibrationPatternType::mode_charuco:
		return
			new CalibrationPatternFinder_Charuco(
				distortionView,
				profileConfig->charucoRows,
				profileConfig->charucoCols,
				profileConfig->charucoSquareLengthMM,
				profileConfig->charucoMarkerLengthMM,
				profileConfig->charucoDictionaryType);
	}

	return nullptr;
}

CalibrationPatternFinderPtr CalibrationPatternFinder::allocatePatternFinderSharedPtr(
	ProjectConfigConstPtr profileConfig,
	VideoFrameDistortionView* distortionView)
{
	switch (profileConfig->calibrationPatternType)
	{
	case eCalibrationPatternType::mode_chessboard:
		return
			std::make_shared<CalibrationPatternFinder_Chessboard>(
				distortionView,
				profileConfig->chessbordRows,
				profileConfig->chessbordCols,
				profileConfig->squareLengthMM);
	case eCalibrationPatternType::mode_charuco:
		return
			std::make_shared<CalibrationPatternFinder_Charuco>(
				distortionView,
				profileConfig->charucoRows,
				profileConfig->charucoCols,
				profileConfig->charucoSquareLengthMM,
				profileConfig->charucoMarkerLengthMM,
				profileConfig->charucoDictionaryType);
	}

	return nullptr;
}

cv::Mat* CalibrationPatternFinder::getGrayscaleVideoFrameInput() const
{
	// By default use the undistorted grayscale image unless explicitly disabled
	// (which should only be the case during distortion calibration)
	return
		m_distortionView->isGrayscaleUndistortDisabled()
		? m_distortionView->getGrayscaleSourceBuffer()
		: m_distortionView->getGrayscaleUndistortBuffer();
}

bool CalibrationPatternFinder::estimateNewCalibrationPatternPose(glm::dmat4& outCameraToPatternXform)
{
	// Make sure mono camera intrinsics are available
	MikanVideoSourceIntrinsics cameraIntrinsics;
	m_distortionView->getVideoSourceView()->getCameraIntrinsics(cameraIntrinsics);
	if (cameraIntrinsics.intrinsics_type != MONO_CAMERA_INTRINSICS)
	{
		return false;
	}

	// Look for the calibration pattern in the latest video frame
	if (!findNewCalibrationPattern())
	{
		return false;
	}

	// Get the image points of the calibration pattern
	cv::Point2f boundingQuad[4];
	t_opencv_point2d_list imagePoints;
	t_opencv_pointID_list imagePointIDs;
	if (!fetchLastFoundCalibrationPattern(imagePoints, imagePointIDs, boundingQuad))
	{
		return false;
	}

	// Make a local copy of the mono camera intrinsics
	MikanMonoIntrinsics monoIntrinsics = cameraIntrinsics.getMonoIntrinsics();

	// Given an object model and the image points samples we could be able to compute 
	// a position and orientation of the calibration pattern relative to the camera
	cv::Quatd cv_cameraToPatternRot;
	cv::Vec3d cv_cameraToPatternVecMM; // Millimeters
	double meanReprojectionError = 0.0;
	if (!computeOpenCVCameraRelativePatternTransform(
		monoIntrinsics,
		imagePoints,
		m_opencvSolvePnPGeometry.points,
		cv_cameraToPatternRot,
		cv_cameraToPatternVecMM,
		&meanReprojectionError))
	{
		return false;
	}

	// Convert OpenCV pose (in mm) to OpenGL pose (in meters)
	convertOpenCVCameraRelativePoseToGLMMat(
		cv_cameraToPatternRot, cv_cameraToPatternVecMM,
		outCameraToPatternXform);

	return true;
}

bool CalibrationPatternFinder::areCurrentImagePointsValid() const
{
	return m_currentImagePoints.size() > 0;
}

void CalibrationPatternFinder::renderCalibrationPattern2D() const
{
	if (areCurrentImagePointsValid())
	{
		drawOpenCVChessBoard2D(
			m_frameWidth, m_frameHeight,
			(float*)m_currentImagePoints.data(), // cv::point2f is just two floats 
			(int)m_currentImagePoints.size(),
			true);
	}
}

void CalibrationPatternFinder::renderSolvePnPPattern3D(const glm::mat4& xform) const
{
	if (areCurrentImagePointsValid())
	{
		drawOpenCVChessBoard3D(
			xform,
			m_openglSolvePnPGeometry.points.data(), // cv::point3f is just three floats 
			(int)m_openglSolvePnPGeometry.points.size(),
			true);
	}
}