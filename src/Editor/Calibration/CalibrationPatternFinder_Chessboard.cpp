#include "CalibrationPatternFinder_Chessboard.h"
#include "CalibrationRenderHelpers.h"
#include "CameraMath.h"
#include "MathTypeConversion.h"
#include "MathUtility.h"
#include "VideoFrameDistortionView.h"

#include "opencv2/opencv.hpp"
#include "opencv2/calib3d/calib3d.hpp"

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

	for (int row= 0; row < m_chessbordRows; ++row)
	{
		for (int col= 0; col < m_chessbordCols; ++col)
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
	const int cornerCount= m_chessbordCols * m_chessbordRows;
	const float newLocationErrorSum= (float)cornerCount * minSeperationDist;

	// Clear out the previous images points
	bool bImagePointsValid= false;
	m_currentImagePoints.clear();

	// Fetch the source image buffer we are searching for the pattern in
	cv::Mat* gsSourceBuffer= getGrayscaleVideoFrameInput();
	if (gsSourceBuffer == nullptr)
		return false;

	// Find chessboard corners:
	const bool bFoundChessboard=
		cv::findChessboardCorners(
			*gsSourceBuffer,
			cv::Size(m_chessbordCols, m_chessbordRows),
			m_currentImagePoints, // output corners
			cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_FILTER_QUADS
				// + cv::CALIB_CB_NORMALIZE_IMAGE is suuuper slow
				+ cv::CALIB_CB_FAST_CHECK);

	if (bFoundChessboard)
	{
		// Get subpixel accuracy on those corners
		cv::cornerSubPix(
			*gsSourceBuffer,
			m_currentImagePoints, // corners to refine
			cv::Size(11, 11),     // winSize- Half of the side length of the search window
			cv::Size(-1, -1),     // zeroZone- (-1,-1) means no dead zone in search
			cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.1));

		// Append the new chessboard corner pixels into the image_points matrix
		if (m_currentImagePoints.size() == cornerCount)
		{
			// If there was a prior image point set,
			// see if this new set is far enough away to be considered unique
			if (m_lastValidImagePoints.size() > 0 && minSeperationDist > 0.f)
			{
				float error_sum= 0.f;

				for (int corner_index= 0; corner_index < cornerCount; ++corner_index)
				{
					float squared_error=
						(float)(cv::norm(
							m_currentImagePoints[corner_index] - m_lastValidImagePoints[corner_index]));

					error_sum+= squared_error;
				}

				bImagePointsValid= error_sum >= newLocationErrorSum;
			}
			else
			{
				// We don't have previous capture.
				bImagePointsValid= true;
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
		const int cornerCount= m_chessbordCols * m_chessbordRows;

		// Keep track of the corners of all of the chessboards we sample
		outBoundingQuad[0]= m_currentImagePoints[0];
		outBoundingQuad[1]= m_currentImagePoints[m_chessbordCols - 1];
		outBoundingQuad[2]= m_currentImagePoints[cornerCount - 1];
		outBoundingQuad[3]= m_currentImagePoints[cornerCount - m_chessbordCols];

		outImagePoints.clear();
		for (const auto& imagePoint : m_currentImagePoints)
		{
			outImagePoints.push_back(imagePoint);
		}

		outImagePointIDs.clear();
		for (int i= 0; i < (int)m_currentImagePoints.size(); ++i)
		{
			outImagePointIDs.push_back(i);
		}

		// Remember the last valid captured points
		m_lastValidImagePoints= m_currentImagePoints;

		return true;
	}

	return false;
}
