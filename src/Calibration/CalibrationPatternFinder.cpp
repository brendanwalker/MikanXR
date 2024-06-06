#include "CalibrationPatternFinder.h"
#include "CalibrationRenderHelpers.h"
#include "Colors.h"
#include "CameraMath.h"
#include "GlTextRenderer.h"
#include "MathOpenCV.h"
#include "MathTypeConversion.h"
#include "TextStyle.h"
#include "VideoFrameDistortionView.h"
#include "VideoSourceView.h"

#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"
#include <opencv2/aruco/charuco.hpp>

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
	ProfileConfigConstPtr profileConfig,
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
	case eCalibrationPatternType::mode_circlegrid:
		return
			new CalibrationPatternFinder_CircleGrid(
				distortionView,
				profileConfig->circleGridRows,
				profileConfig->circleGridCols,
				profileConfig->circleSpacingMM,
				profileConfig->circleDiameterMM);
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
	ProfileConfigConstPtr profileConfig,
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
		case eCalibrationPatternType::mode_circlegrid:
			return
				std::make_shared<CalibrationPatternFinder_CircleGrid>(
					distortionView,
					profileConfig->circleGridRows,
					profileConfig->circleGridCols,
					profileConfig->circleSpacingMM,
					profileConfig->circleDiameterMM);
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
	if (!fetchLastFoundCalibrationPattern(imagePoints, boundingQuad))
	{
		return false;
	}

	// Given an object model and the image points samples we could be able to compute 
	// a position and orientation of the calibration pattern relative to the camera
	cv::Quatd cv_cameraToPatternRot;
	cv::Vec3d cv_cameraToPatternVecMM; // Millimeters
	if (!computeOpenCVCameraRelativePatternTransform(
		cameraIntrinsics.intrinsics.mono,
		imagePoints,
		m_opencvSolvePnPGeometry.points,
		cv_cameraToPatternRot,
		cv_cameraToPatternVecMM))
	{
		return false;
	}

	// Convert OpenCV pose (in mm) to OpenGL pose (in meters)
	convertOpenCVCameraRelativePoseToGLMMat(
		cv_cameraToPatternRot, cv_cameraToPatternVecMM, SOLVEPNP_XAXIS_FLIP_ANGLE,
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

	cv::Mat* gsSourceBuffer =
		m_distortionView->isGrayscaleUndistortDisabled()
		? m_distortionView->getGrayscaleSourceBuffer()
		: m_distortionView->getGrayscaleUndistortBuffer();
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

		// Remember the last valid captured points
		m_lastValidImagePoints = m_currentImagePoints;

		return true;
	}

	return false;
}

// -- CalibrationPatternFinder_CircleGrid -----
CalibrationPatternFinder_CircleGrid::CalibrationPatternFinder_CircleGrid(
	VideoFrameDistortionView* distortionView,
	int circleGridRows,
	int circleGridCols,
	float circleSpacingMM,
	float circleDiameterMM)
	: CalibrationPatternFinder(distortionView)
	, m_circleGridRows(circleGridRows)
	, m_circleGridCols(circleGridCols)
	, m_circleSpacingMM(circleSpacingMM)
	, m_circleDiameterMM(circleDiameterMM)
{
	const int col_count = 2 * m_circleGridCols;
	const int row_count = m_circleGridRows;
	const int circle_count = m_circleGridRows * m_circleGridCols;
	const float radius_mm = m_circleDiameterMM / 2.f;

	m_opencvSolvePnPGeometry.points.clear();
	for (int row = 0; row < row_count; ++row)
	{
		for (int col = 0; col < col_count; ++col)
		{
			const bool bRowIsEven = (row % 2) == 0;
			const bool bColIsEven = (col % 2) == 0;

			if ((bRowIsEven && !bColIsEven) || (!bRowIsEven && bColIsEven))
			{
				cv::Point3f openCVPoint(
					float(col) * m_circleSpacingMM + radius_mm,
					0.f,
					-float(row) * m_circleSpacingMM - radius_mm);

				// OpenCV -> OpenGL coordinate system transform
				// Rendering world units in meters, not mm
				glm::vec3 openGLPoint(
					openCVPoint.x * k_millimeters_to_meters,
					-openCVPoint.y * k_millimeters_to_meters,
					-openCVPoint.z * k_millimeters_to_meters);

				m_opencvSolvePnPGeometry.points.push_back(openCVPoint);
				m_openglSolvePnPGeometry.points.push_back(openGLPoint);
			}
		}
	}
	assert(m_opencvSolvePnPGeometry.points.size() == circle_count);
}

bool CalibrationPatternFinder_CircleGrid::findNewCalibrationPattern(const float minSeperationDist)
{
	const int circleCount = m_circleGridCols * m_circleGridRows;
	const float newLocationErrorSum = (float)circleCount * minSeperationDist;

	// Clear out the previous images points
	bool bImagePointsValid = false;
	m_currentImagePoints.clear();

	cv::Mat* gsSourceBuffer =
		m_distortionView->isGrayscaleUndistortDisabled()
		? m_distortionView->getGrayscaleSourceBuffer()
		: m_distortionView->getGrayscaleUndistortBuffer();
	if (gsSourceBuffer == nullptr)
		return false;

	// Find circle grid centers:
	if (cv::findCirclesGrid(
		*gsSourceBuffer,
		cv::Size(m_circleGridCols, m_circleGridRows),
		m_currentImagePoints, // output centers
		cv::CALIB_CB_ASYMMETRIC_GRID))
	{
		// Append the new circle-grid pixels into the image_points matrix
		if (m_currentImagePoints.size() == circleCount)
		{
			// If there was a prior image point set, 
			// see if this new set is far enough away to be considered unique
			if (m_lastValidImagePoints.size() > 0)
			{
				float error_sum = 0.f;

				for (int corner_index = 0; corner_index < circleCount; ++corner_index)
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

bool CalibrationPatternFinder_CircleGrid::fetchLastFoundCalibrationPattern(
	t_opencv_point2d_list& outImagePoints,
	cv::Point2f outBoundingQuad[4])
{
	// If it's a valid new location, append it to the board list
	if (areCurrentImagePointsValid())
	{
		const int circleCount = m_circleGridCols * m_circleGridRows;

		// Keep track of the corners of all of the circle grids we sample
		outBoundingQuad[0] = m_currentImagePoints[0];
		outBoundingQuad[1] = m_currentImagePoints[m_circleGridCols - 1];
		outBoundingQuad[2] = m_currentImagePoints[circleCount - 1];
		outBoundingQuad[3] = m_currentImagePoints[circleCount - m_circleGridCols];

		outImagePoints.clear();
		for (const auto& imagePoint : m_currentImagePoints)
		{
			outImagePoints.push_back(imagePoint);
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
	cv::Ptr<cv::aruco::CharucoBoard> board;
	cv::Ptr<cv::aruco::Dictionary> dictionary;
	cv::Ptr<cv::aruco::DetectorParameters> params;

	std::vector<t_opencv_point2d_list> markerCorners;
	std::vector<int> markerVisibleIds;
	t_opencv_point2d_list charucoCorners;
	std::vector<int> charucoIds;
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
	m_markerData->board= new cv::aruco::CharucoBoard(
		cv::Size(charucoCols, charucoRows),
		charucoSquareLengthMM, charucoMarkerLengthMM,
		dictionary);
	m_markerData->dictionary = cv::makePtr<cv::aruco::Dictionary>(dictionary);
	m_markerData->params = cv::makePtr<cv::aruco::DetectorParameters>();
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

	// Use the original source buffer for the grayscale image (NOT the undistorted one)
	cv::Mat* gsSourceBuffer = m_distortionView->getGrayscaleSourceBuffer();
	if (gsSourceBuffer == nullptr)
		return false;

	// Find Arcuo marker corners on the small image
	m_markerData->markerCorners.clear();
	cv::aruco::detectMarkers(
		*gsSourceBuffer,
		m_markerData->dictionary,
		m_markerData->markerCorners,
		m_markerData->markerVisibleIds,
		m_markerData->params);
	const bool bFoundMarkers = m_markerData->markerVisibleIds.size() > 0;

	if (bFoundMarkers)
	{
		cv::Matx33d cvIntrinsicMatrix;
		cv::Mat cvDistCoeffsRowVector;
		bool bUseCameraCalibration = false;

		// If we have valid undistorted camera intrinsics, use them
		if (!m_distortionView->isGrayscaleUndistortDisabled())
		{
			MikanVideoSourceIntrinsics cameraIntrinsics;
			m_distortionView->getVideoSourceView()->getCameraIntrinsics(cameraIntrinsics);

			if (cameraIntrinsics.intrinsics_type == MONO_CAMERA_INTRINSICS)
			{
				// Fetch the 3x3 GLM camera intrinsic matrix and store into a 3x3 openCV matrix
				const MikanMonoIntrinsics& intrinsics = cameraIntrinsics.intrinsics.mono;
				const MikanMatrix3d& glmIntrinsicMatrix = intrinsics.camera_matrix;
				
				// Store the distortion parameters in a row vector with 8 values: [k1, k2, p1, p2, k3, k4, k5, k6]
				const MikanDistortionCoefficients& distortion_coeffs = intrinsics.distortion_coefficients;
				cv::Matx81d cvDistCoeffsColVector = Mikan_distortion_to_cv_vec8(distortion_coeffs);

				cvIntrinsicMatrix = MikanMatrix3d_to_cv_mat33d(glmIntrinsicMatrix);
				cv::transpose(cvDistCoeffsColVector, cvDistCoeffsRowVector);

				bUseCameraCalibration= true;
			}
		}

		// Compute chessboard corners from the detected markers
		if (bUseCameraCalibration)
		{
			cv::aruco::interpolateCornersCharuco(
				m_markerData->markerCorners, m_markerData->markerVisibleIds,
				*gsSourceBuffer, m_markerData->board,
				m_markerData->charucoCorners, m_markerData->charucoIds,
				cvIntrinsicMatrix, cvDistCoeffsRowVector);
		}
		else
		{
			cv::aruco::interpolateCornersCharuco(
				m_markerData->markerCorners, m_markerData->markerVisibleIds,
				*gsSourceBuffer, m_markerData->board,
				m_markerData->charucoCorners, m_markerData->charucoIds);
		}
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

		// Remember the last valid captured points
		m_lastValidImagePoints = m_currentImagePoints;

		return true;
	}

	return false;
}

bool CalibrationPatternFinder_Charuco::estimateNewCalibrationPatternPose(glm::dmat4& outCameraToPatternXform)
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

	// Fetch the 3x3 GLM camera intrinsic matrix and store into a 3x3 openCV matrix
	const MikanMonoIntrinsics& intrinsics= cameraIntrinsics.intrinsics.mono;
	const MikanMatrix3d& glmIntrinsicMatrix = intrinsics.camera_matrix;
	cv::Matx33d cvIntrinsicMatrix = MikanMatrix3d_to_cv_mat33d(glmIntrinsicMatrix);

	// Store the distortion parameters in a row vector with 8 values: [k1, k2, p1, p2, k3, k4, k5, k6]
	const MikanDistortionCoefficients& distortion_coeffs = intrinsics.distortion_coefficients;
	cv::Matx81d cvDistCoeffsColVector = Mikan_distortion_to_cv_vec8(distortion_coeffs);
	cv::Mat cvDistCoeffsRowVector;
	cv::transpose(cvDistCoeffsColVector, cvDistCoeffsRowVector);

	// Given an object model and the image points samples we could be able to compute 
	// a position and orientation of the calibration pattern relative to the camera
	cv::Mat rvec;
	cv::Mat tvecMM; // Mat position in millimeters
	if (!cv::aruco::estimatePoseCharucoBoard(
		m_markerData->charucoCorners,
		m_markerData->charucoIds,
		m_markerData->board, 
		cvIntrinsicMatrix, cvDistCoeffsRowVector,
		rvec, tvecMM))
	{
		return false;
	}
	cv::Quatd cv_cameraToPatternRot= cv::Quatd::createFromRvec(rvec);
	cv::Vec3d cv_cameraToPatternVecMM= tvecMM; 

	// Convert OpenCV pose (in mm) to OpenGL pose (in meters)
	convertOpenCVCameraRelativePoseToGLMMat(
		cv_cameraToPatternRot, cv_cameraToPatternVecMM, CHARUCO_XAXIS_FLIP_ANGLE,
		outCameraToPatternXform);

	return true;
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

	// Draw the marker corners, if any
	//TextStyle style = getDefaultTextStyle();
	//style.horizontalAlignment = eHorizontalTextAlignment::Middle;
	//style.verticalAlignment = eVerticalTextAlignment::Middle;
	//style.color = Colors::Yellow;

	//static int debugDrawIndex = -1;

	//for (int quadIndex = 0; quadIndex < m_markerData->markerCorners.size(); quadIndex++)
	//{
	//	if (debugDrawIndex != -1 && debugDrawIndex != quadIndex)
	//		continue;

	//	const t_opencv_point2d_list& corners = m_markerData->markerCorners[quadIndex];

	//	drawQuadList2d(
	//		m_frameWidth, m_frameHeight,
	//		(float*)corners.data(), // cv::point2f is just two floats 
	//		(int)corners.size(),
	//		Colors::Yellow);

	//	if (quadIndex < m_markerData->markerVisibleIds.size())
	//	{
	//		int markerId = m_markerData->markerVisibleIds[quadIndex];

	//		cv::Point2f quadCenter;
	//		opencv_point2f_compute_average(corners, quadCenter);

	//		drawTextAtTrackerPosition(
	//			style,
	//			m_frameWidth, m_frameHeight,
	//			glm::vec2(quadCenter.x, quadCenter.y),
	//			L"%d", markerId);
	//	}
	//}
}