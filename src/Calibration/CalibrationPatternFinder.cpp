#include "CalibrationPatternFinder.h"
#include "CalibrationRenderHelpers.h"
#include "CameraMath.h"
#include "MathTypeConversion.h"
#include "VideoFrameDistortionView.h"

#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"

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
	const ProfileConfig* profileConfig,
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
	}

	return nullptr;
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
			m_openglGeometry.points.data(), // cv::point3f is just three floats 
			(int)m_openglGeometry.points.size(),
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
	m_openglGeometry.points.clear();

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
			m_openglGeometry.points.push_back(openGLPoint);
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

	cv::Mat* gsSmallBuffer= m_distortionView->getGrayscaleSmallBuffer();
	cv::Mat* gsSourceBuffer= m_distortionView->getGrayscaleSourceBuffer();
	if (gsSmallBuffer == nullptr || gsSourceBuffer == nullptr)
		return false;

	// Find chessboard corners:
	t_opencv_point2d_list m_smallImagePoints;	 
	const bool bFoundChessboard= 
		cv::findChessboardCorners(
			*gsSmallBuffer,
			cv::Size(m_chessbordCols, m_chessbordRows),
			m_smallImagePoints, // output corners
			cv::CALIB_CB_ADAPTIVE_THRESH
			+ cv::CALIB_CB_FILTER_QUADS
			// + cv::CALIB_CB_NORMALIZE_IMAGE is suuuper slow
			+ cv::CALIB_CB_FAST_CHECK);

	if (bFoundChessboard)
	{
		// Scale the points found in the small image to corresponding location in the source image
		const float smallToSourceScale= m_distortionView->getSmallToSourceScale();
		for (const cv::Point2f& smallPoint : m_smallImagePoints)
		{
			cv::Point2f sourcePoint=  {
				smallPoint.x * smallToSourceScale,
				smallPoint.y * smallToSourceScale 
			};

			m_currentImagePoints.push_back(sourcePoint);
		}

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
				m_openglGeometry.points.push_back(openGLPoint);
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

	// Find circle grid centers:
	if (cv::findCirclesGrid(
		*m_distortionView->getGrayscaleSourceBuffer(),
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