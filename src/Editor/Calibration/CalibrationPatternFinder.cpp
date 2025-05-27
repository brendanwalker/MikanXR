#include "CalibrationPatternFinder.h"
#include "CalibrationRenderHelpers.h"
#include "Colors.h"
#include "CameraMath.h"
#include "MikanTextRenderer.h"
#include "Logger.h"
#include "MathOpenCV.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "TextStyle.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceView.h"

#include <algorithm>

#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include "opencv2/objdetect/aruco_detector.hpp"
#include "opencv2/objdetect/charuco_detector.hpp"

//-- CalibrationPatternFinder_Chessboard -----
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

//-- CalibrationPatternFinder_Chessboard -----
CalibrationPatternFinder_Chessboard::CalibrationPatternFinder_Chessboard(
	VideoFrameDistortionView* distortionView,
	int chessbordRows,
	int chessbordCols,
	float squareLengthMM)
	: CalibrationPatternFinder(distortionView)
	, m_chessbordRows(chessbordRows)
	, m_chessbordCols(chessbordCols)
	, m_squareLengthMM(squareLengthMM)
{
	m_opencvLensCalibrationGeometry.points.clear();
	m_opencvSolvePnPGeometry.points.clear();
	m_openglSolvePnPGeometry.points.clear();

	for (int row = 0; row < m_chessbordRows; ++row)
	{
		for (int col = 0; col < m_chessbordCols; ++col)
		{
			// Solve PnP points are on the XZ Plane
			cv::Point3f openCVSolvePnPPoint(
				float(col) * m_squareLengthMM,
				0.f,
				-float(row) * m_squareLengthMM);
			// Lens calibration points are on the XY Plane
			cv::Point3f openCVLensCalibrationPoint(
				float(col) * m_squareLengthMM,
				float(row) * m_squareLengthMM, 
				0.f);

			// OpenCV -> OpenGL coordinate system transform
			// Rendering world units in meters, not mm
			glm::vec3 openGLPoint(
				openCVSolvePnPPoint.x * k_millimeters_to_meters, 
				-openCVSolvePnPPoint.y * k_millimeters_to_meters,
				-openCVSolvePnPPoint.z * k_millimeters_to_meters); 

			m_opencvLensCalibrationGeometry.points.push_back(openCVLensCalibrationPoint);
			m_opencvSolvePnPGeometry.points.push_back(openCVSolvePnPPoint);
			m_openglSolvePnPGeometry.points.push_back(openGLPoint);
		}
	}
}

bool CalibrationPatternFinder_Chessboard::findNewCalibrationPattern(const float minSeperationDist)
{
	const int cornerCount = m_chessbordCols * m_chessbordRows;
	const float newLocationErrorSum = (float)cornerCount * minSeperationDist;

	// Clear out the previous images points
	bool bImagePointsValid = false;
	m_currentImagePoints.clear();

	// Fetch the source image buffer we are searching for the pattern in
	cv::Mat* gsSourceBuffer = getGrayscaleVideoFrameInput();
	if (gsSourceBuffer == nullptr)
		return false;

	// Find chessboard corners:
	const bool bFoundChessboard= 
		cv::findChessboardCorners(
			*gsSourceBuffer,
			cv::Size(m_chessbordCols, m_chessbordRows),
			m_currentImagePoints, // output corners
			cv::CALIB_CB_ADAPTIVE_THRESH
			+ cv::CALIB_CB_FILTER_QUADS
			// + cv::CALIB_CB_NORMALIZE_IMAGE is suuuper slow
			+ cv::CALIB_CB_FAST_CHECK);

	if (bFoundChessboard)
	{
		// Get subpixel accuracy on those corners
		cv::cornerSubPix(
			*gsSourceBuffer,
			m_currentImagePoints, // corners to refine
			cv::Size(11, 11), // winSize- Half of the side length of the search window
			cv::Size(-1, -1), // zeroZone- (-1,-1) means no dead zone in search
			cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.1));

		// Append the new chessboard corner pixels into the image_points matrix
		if (m_currentImagePoints.size() == cornerCount)
		{
			// If there was a prior image point set, 
			// see if this new set is far enough away to be considered unique
			if (m_lastValidImagePoints.size() > 0 && minSeperationDist > 0.f)
			{
				float error_sum = 0.f;

				for (int corner_index = 0; corner_index < cornerCount; ++corner_index)
				{
					float squared_error =
						(float)(cv::norm(
							m_currentImagePoints[corner_index]
							- m_lastValidImagePoints[corner_index]));

					error_sum += squared_error;
				}

				bImagePointsValid = error_sum >= newLocationErrorSum;
			}
			else
			{
				// We don't have previous capture.
				bImagePointsValid = true;
			}
		}
	}

	// Re-clear out the image points if we decided the latest captured onces are invalid
	if (!bImagePointsValid)
	{
		m_currentImagePoints.clear();
	}

	return bImagePointsValid;
}

bool CalibrationPatternFinder_Chessboard::fetchLastFoundCalibrationPattern(
	t_opencv_point2d_list& outImagePoints, 
	t_opencv_pointID_list& outImagePointIDs,
	cv::Point2f outBoundingQuad[4])
{
	// If it's a valid new location, append it to the board list
	if (areCurrentImagePointsValid())
	{
		const int cornerCount = m_chessbordCols * m_chessbordRows;

		// Keep track of the corners of all of the chessboards we sample
		outBoundingQuad[0] = m_currentImagePoints[0];
		outBoundingQuad[1] = m_currentImagePoints[m_chessbordCols - 1];
		outBoundingQuad[2] = m_currentImagePoints[cornerCount - 1];
		outBoundingQuad[3] = m_currentImagePoints[cornerCount - m_chessbordCols];

		outImagePoints.clear();
		for (const auto& imagePoint : m_currentImagePoints)
		{
			outImagePoints.push_back(imagePoint);
		}

		outImagePointIDs.clear();
		for (int i = 0; i < (int)m_currentImagePoints.size(); ++i)
		{
			outImagePointIDs.push_back(i);
		}

		// Remember the last valid captured points
		m_lastValidImagePoints = m_currentImagePoints;

		return true;
	}

	return false;
}

//-- CalibrationPatternFinder_Charuco -----
class CharucoBoardData
{
public:
	CharucoBoardData() = default;

	int rows;
	int cols;
	float squareLengthMM;
	float markerLengthMM;

	cv::Ptr<cv::aruco::CharucoDetector> detector;
	t_opencv_point2d_list charucoCorners;
	std::vector<int> charucoIds;
	std::vector<t_opencv_point2d_list> markerCorners;
	std::vector<int> markerVisibleIds;
};

CalibrationPatternFinder_Charuco::CalibrationPatternFinder_Charuco(
	VideoFrameDistortionView* distortionView,
	int charucoRows,
	int charucoCols,
	float charucoSquareLengthMM,
	float charucoMarkerLengthMM,
	eCharucoDictionaryType charucoDictionaryType)
	: CalibrationPatternFinder(distortionView)
	, m_markerData(new CharucoBoardData())
{
	m_opencvLensCalibrationGeometry.points.clear();
	m_opencvSolvePnPGeometry.points.clear();
	m_openglSolvePnPGeometry.points.clear();

	const int cornerRows = charucoRows - 1;
	const int cornerCols = charucoCols - 1;

	for (int row = 0; row < cornerRows; ++row)
	{
		for (int col = 0; col < cornerCols; ++col)
		{
			// Solve PnP points are on the XZ Plane
			cv::Point3f openCVSolvePnPPoint(
				float(col) * charucoSquareLengthMM,
				0.f,
				-float(row) * charucoSquareLengthMM);
			// Lens calibration points are on the XY Plane
			cv::Point3f openCVLensCalibrationPoint(
				float(col) * charucoSquareLengthMM,
				float(row) * charucoSquareLengthMM,
				0.f);

			// OpenCV -> OpenGL coordinate system transform
			// Rendering world units in meters, not mm
			glm::vec3 openGLPoint(
				openCVSolvePnPPoint.x * k_millimeters_to_meters,
				-openCVSolvePnPPoint.y * k_millimeters_to_meters,
				-openCVSolvePnPPoint.z * k_millimeters_to_meters);

			m_opencvLensCalibrationGeometry.points.push_back(openCVLensCalibrationPoint);
			m_opencvSolvePnPGeometry.points.push_back(openCVSolvePnPPoint);
			m_openglSolvePnPGeometry.points.push_back(openGLPoint);
		}
	}

	cv::aruco::PredefinedDictionaryType cvCharucoDictionary= cv::aruco::DICT_6X6_250;
	switch (charucoDictionaryType)
	{
		case eCharucoDictionaryType::DICT_4X4:
			cvCharucoDictionary= cv::aruco::DICT_4X4_250;
			break;
		case eCharucoDictionaryType::DICT_5X5:
			cvCharucoDictionary= cv::aruco::DICT_5X5_250;
			break;
		case eCharucoDictionaryType::DICT_6X6:
			cvCharucoDictionary= cv::aruco::DICT_6X6_250;
			break;
		case eCharucoDictionaryType::DICT_7X7:
			cvCharucoDictionary= cv::aruco::DICT_7X7_250;
			break;
		default:
			break;
	}

	cv::aruco::Dictionary dictionary= cv::aruco::getPredefinedDictionary(cvCharucoDictionary);
	cv::aruco::CharucoBoard board(
		cv::Size(charucoCols, charucoRows),
		charucoSquareLengthMM * k_millimeters_to_meters, 
		charucoMarkerLengthMM * k_millimeters_to_meters,
		dictionary);
	m_markerData->detector = cv::makePtr<cv::aruco::CharucoDetector>(board);
	m_markerData->rows = charucoRows;
	m_markerData->cols = charucoCols;
	m_markerData->squareLengthMM = charucoSquareLengthMM;
	m_markerData->markerLengthMM = charucoMarkerLengthMM;

}

CalibrationPatternFinder_Charuco::~CalibrationPatternFinder_Charuco()
{
	delete m_markerData;
}

bool CalibrationPatternFinder_Charuco::findNewCalibrationPattern(const float minSeperationDist)
{
	const int cornerCount = (m_markerData->cols - 1) * (m_markerData->rows - 1);
	const float newLocationErrorSum = (float)cornerCount * minSeperationDist;

	// Clear out the previous images points
	bool bImagePointsValid = false;
	m_currentImagePoints.clear();

	// Fetch the source image buffer we are searching for the pattern in
	cv::Mat* gsSourceBuffer = getGrayscaleVideoFrameInput();
	if (gsSourceBuffer == nullptr)
		return false;

	// Find Charuco marker corners in the source image
	m_markerData->markerCorners.clear();
	m_markerData->markerVisibleIds.clear();
	m_markerData->detector->detectBoard(
		*gsSourceBuffer, 
		m_markerData->charucoCorners,
		m_markerData->charucoIds,
		m_markerData->markerCorners, 
		m_markerData->markerVisibleIds);
	const bool bFoundMarkers = m_markerData->markerVisibleIds.size() > 0;

	if (bFoundMarkers)
	{
		// Remember the last valid captured points
		m_currentImagePoints= m_markerData->charucoCorners;

		// Append the new chessboard corner pixels into the image_points matrix
		if (m_currentImagePoints.size() == cornerCount)
		{
			// If there was a prior image point set, 
			// see if this new set is far enough away to be considered unique
			if (m_lastValidImagePoints.size() > 0 && minSeperationDist > 0.f)
			{
				float error_sum = 0.f;

				for (int corner_index = 0; corner_index < cornerCount; ++corner_index)
				{
					float squared_error =
						(float)(cv::norm(
							m_currentImagePoints[corner_index]
							- m_lastValidImagePoints[corner_index]));

					error_sum += squared_error;
				}

				bImagePointsValid = error_sum >= newLocationErrorSum;
			}
			else
			{
				// We don't have previous capture.
				bImagePointsValid = true;
			}
		}
	}

	// Re-clear out the image points if we decided the latest captured onces are invalid
	if (!bImagePointsValid)
	{
		m_currentImagePoints.clear();
	}

	return bImagePointsValid;
}

bool CalibrationPatternFinder_Charuco::fetchLastFoundCalibrationPattern(
	t_opencv_point2d_list& outImagePoints,
	t_opencv_pointID_list& outImagePointIDs,
	cv::Point2f outBoundingQuad[4])
{
	// If it's a valid new location, append it to the board list
	if (areCurrentImagePointsValid())
	{
		// The number of corners in a row is one less than the number of squares
		const int cornerCols = m_markerData->cols - 1;
		const int cornerCount = (int)m_currentImagePoints.size();

		// Keep track of the corners of all of the chessboards we sample
		outBoundingQuad[0] = m_currentImagePoints[0];
		outBoundingQuad[1] = m_currentImagePoints[cornerCols - 1];
		outBoundingQuad[2] = m_currentImagePoints[cornerCount - 1];
		outBoundingQuad[3] = m_currentImagePoints[cornerCount - cornerCols];

		outImagePoints.clear();
		for (const auto& imagePoint : m_currentImagePoints)
		{
			outImagePoints.push_back(imagePoint);
		}

		
		outImagePointIDs= m_markerData->charucoIds;


		// Remember the last valid captured points
		m_lastValidImagePoints = m_currentImagePoints;

		return true;
	}

	return false;
}

void CalibrationPatternFinder_Charuco::renderCalibrationPattern2D() const
{
	CalibrationPatternFinder::renderCalibrationPattern2D();

	// Draw the marker corners, if any
	TextStyle style = getDefaultTextStyle();
	style.horizontalAlignment = eHorizontalTextAlignment::Middle;
	style.verticalAlignment = eVerticalTextAlignment::Middle;
	style.color = Colors::Yellow;

	static int debugDrawIndex = -1;
	
	for (int quadIndex = 0; quadIndex < m_markerData->markerCorners.size(); quadIndex++)
	{
		if (debugDrawIndex != -1 && debugDrawIndex != quadIndex)
			continue;

		const t_opencv_point2d_list& corners = m_markerData->markerCorners[quadIndex];

		drawQuadList2d(
			m_frameWidth, m_frameHeight,
			(float*)corners.data(), // cv::point2f is just two floats 
			(int)corners.size(),
			Colors::Yellow);

		if (quadIndex < m_markerData->markerVisibleIds.size())
		{
			int markerId = m_markerData->markerVisibleIds[quadIndex];

			cv::Point2f quadCenter;
			opencv_point2f_compute_average(corners, quadCenter);
			
			drawTextAtTrackerPosition(
				style,
				m_frameWidth, m_frameHeight,
				glm::vec2(quadCenter.x, quadCenter.y),
				L"%d", markerId);
		}
	}
}

void CalibrationPatternFinder_Charuco::renderSolvePnPPattern3D(const glm::mat4& xform) const
{
	CalibrationPatternFinder::renderSolvePnPPattern3D(xform);
}

//-- CalibrationPatternFinder_Aruco -----
class ArucoBoardData
{
public:
	ArucoBoardData() = default;

	int desiredArucoId;
	float markerLengthMM;

	cv::Ptr<cv::aruco::ArucoDetector> detector;
	std::vector<t_opencv_point2d_list> markerCorners;
	std::vector<int> markerVisibleIds;
	t_opencv_point2d_list charucoCorners;
	std::vector<int> charucoIds;
};

CalibrationPatternFinder_Aruco::CalibrationPatternFinder_Aruco(
	VideoFrameDistortionView* distortionView,
	int desiredArucoId,
	float markerLengthMM,
	eCharucoDictionaryType charucoDictionaryType)
	: CalibrationPatternFinder(distortionView)
	, m_markerData(new ArucoBoardData())
{
	cv::aruco::PredefinedDictionaryType cvCharucoDictionary = cv::aruco::DICT_6X6_250;
	switch (charucoDictionaryType)
	{
		case eCharucoDictionaryType::DICT_4X4:
			cvCharucoDictionary = cv::aruco::DICT_4X4_250;
			break;
		case eCharucoDictionaryType::DICT_5X5:
			cvCharucoDictionary = cv::aruco::DICT_5X5_250;
			break;
		case eCharucoDictionaryType::DICT_6X6:
			cvCharucoDictionary = cv::aruco::DICT_6X6_250;
			break;
		case eCharucoDictionaryType::DICT_7X7:
			cvCharucoDictionary = cv::aruco::DICT_7X7_250;
			break;
		default:
			break;
	}
	cv::aruco::Dictionary dictionary = cv::aruco::getPredefinedDictionary(cvCharucoDictionary);

	// Use corner refinement to get the best possible corner locations
	cv::aruco::DetectorParameters detectorParams;
	detectorParams.cornerRefinementMethod= cv::aruco::CORNER_REFINE_SUBPIX;

	m_markerData->desiredArucoId = desiredArucoId;
	m_markerData->markerLengthMM = markerLengthMM;
	m_markerData->detector = cv::makePtr<cv::aruco::ArucoDetector>(dictionary, detectorParams);

	// The Aruco board is a square, so we can hardcode the points in ARUCO_CCW_CENTER style
	// Solve PnP points are on the XZ Plane
	m_opencvSolvePnPGeometry.points.clear();
	m_opencvSolvePnPGeometry.points.push_back(cv::Point3f(-markerLengthMM / 2.f, 0.f, markerLengthMM / 2.f));
	m_opencvSolvePnPGeometry.points.push_back(cv::Point3f(markerLengthMM / 2.f, 0.f, markerLengthMM / 2.f));
	m_opencvSolvePnPGeometry.points.push_back(cv::Point3f(markerLengthMM / 2.f, 0.f, -markerLengthMM / 2.f));
	m_opencvSolvePnPGeometry.points.push_back(cv::Point3f(-markerLengthMM / 2.f, 0.f, -markerLengthMM / 2.f));

	// Derive the other geometry from the OpenCV SolvePnP geometry
	m_opencvLensCalibrationGeometry.points.clear();
	m_openglSolvePnPGeometry.points.clear();
	for (int index = 0; index < 4; index++)
	{
		// Solve PnP points are on the XZ Plane
		const cv::Point3f& openCVSolvePnPPoint= m_opencvSolvePnPGeometry.points[index];

		// Lens calibration points are on the XY Plane
		cv::Point3f openCVLensCalibrationPoint(
			openCVSolvePnPPoint.x,
			openCVSolvePnPPoint.z,
			0.f);
		m_opencvLensCalibrationGeometry.points.push_back(openCVLensCalibrationPoint);

		// OpenCV -> OpenGL coordinate system transform
		// Rendering world units in meters, not mm
		glm::vec3 openGLPoint(
			openCVSolvePnPPoint.x * k_millimeters_to_meters,
			-openCVSolvePnPPoint.y * k_millimeters_to_meters,
			-openCVSolvePnPPoint.z * k_millimeters_to_meters);
		m_openglSolvePnPGeometry.points.push_back(openGLPoint);
	}
}

CalibrationPatternFinder_Aruco::~CalibrationPatternFinder_Aruco()
{
	delete m_markerData;
}

bool CalibrationPatternFinder_Aruco::findNewCalibrationPattern(const float minSeperationDist)
{
	// Clear out the previous images points
	bool bImagePointsValid = false;
	m_currentImagePoints.clear();

	// Fetch the source image buffer we are searching for the pattern in
	cv::Mat* gsSourceBuffer = getGrayscaleVideoFrameInput();
	if (gsSourceBuffer == nullptr)
		return false;

	// Find Arcuo marker corners on the small image
	m_markerData->markerCorners.clear();
	m_markerData->detector->detectMarkers(
		*gsSourceBuffer,
		m_markerData->markerCorners,
		m_markerData->markerVisibleIds);
	const bool bFoundMarkers = m_markerData->markerVisibleIds.size() > 0;

	// Re-clear out the image points if we decided the latest captured onces are invalid
	if (bFoundMarkers)
	{
		for (int index = 0; index < m_markerData->markerVisibleIds.size(); ++index)
		{
			if (m_markerData->markerVisibleIds[index] == m_markerData->desiredArucoId)
			{
				m_currentImagePoints= m_markerData->markerCorners[index];
				break;
			}
		}
	}
	else
	{
		m_currentImagePoints.clear();
	}

	return bFoundMarkers;
}

bool CalibrationPatternFinder_Aruco::fetchLastFoundCalibrationPattern(
	t_opencv_point2d_list& outImagePoints,
	t_opencv_pointID_list& outImagePointIDs,
	cv::Point2f outBoundingQuad[4])
{
	// If it's a valid new location, append it to the board list
	if (areCurrentImagePointsValid())
	{
		// Keep track of the corners of all of the chessboards we sample
		outBoundingQuad[0] = m_currentImagePoints[0];
		outBoundingQuad[1] = m_currentImagePoints[1];
		outBoundingQuad[2] = m_currentImagePoints[2];
		outBoundingQuad[3] = m_currentImagePoints[3];

		outImagePoints.clear();
		for (const auto& imagePoint : m_currentImagePoints)
		{
			outImagePoints.push_back(imagePoint);
		}

		outImagePointIDs.clear();
		outImagePointIDs.push_back(m_markerData->desiredArucoId);

		// Remember the last valid captured points
		m_lastValidImagePoints = m_currentImagePoints;

		return true;
	}

	return false;
}

void CalibrationPatternFinder_Aruco::renderCalibrationPattern2D() const
{
	CalibrationPatternFinder::renderCalibrationPattern2D();

	// Draw the marker corners, if any
	TextStyle style = getDefaultTextStyle();
	style.horizontalAlignment = eHorizontalTextAlignment::Middle;
	style.verticalAlignment = eVerticalTextAlignment::Middle;
	style.color = Colors::Yellow;

	static int debugDrawIndex = -1;

	for (int quadIndex = 0; quadIndex < m_markerData->markerCorners.size(); quadIndex++)
	{
		if (debugDrawIndex != -1 && debugDrawIndex != quadIndex)
			continue;

		const t_opencv_point2d_list& corners = m_markerData->markerCorners[quadIndex];

		drawQuadList2d(
			m_frameWidth, m_frameHeight,
			(float*)corners.data(), // cv::point2f is just two floats 
			(int)corners.size(),
			Colors::Yellow);

		if (quadIndex < m_markerData->markerVisibleIds.size())
		{
			int markerId = m_markerData->markerVisibleIds[quadIndex];

			cv::Point2f quadCenter;
			opencv_point2f_compute_average(corners, quadCenter);

			drawTextAtTrackerPosition(
				style,
				m_frameWidth, m_frameHeight,
				glm::vec2(quadCenter.x, quadCenter.y),
				L"%d", markerId);
		}
	}
}