#pragma once

#include "ObjectSystemConfigFwd.h"
#include "OpenCVFwd.h"
#include "CameraMath.h"
#include "ProfileConfig.h"

#include <memory>
#include "opencv2/core/types.hpp"

#include "glm/ext/vector_float3.hpp"

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class CalibrationPatternFinder;
typedef std::shared_ptr<CalibrationPatternFinder> CalibrationPatternFinderPtr;

// Helper use to implement OpenCV camera lens intrinsic/distortion calibration method.
// See https://docs.opencv.org/3.3.0/dc/dbb/tutorial_py_calibration.html for details.
class CalibrationPatternFinder
{
public:
	CalibrationPatternFinder(class VideoFrameDistortionView* distortionView);
	virtual ~CalibrationPatternFinder();

	static CalibrationPatternFinder* allocatePatternFinder(
		ProfileConfigConstPtr profileConfig, 
		class VideoFrameDistortionView* distortionView);
	static CalibrationPatternFinderPtr allocatePatternFinderSharedPtr(
		ProfileConfigConstPtr profileConfig,
		class VideoFrameDistortionView* distortionView);

	cv::Mat* getGrayscaleVideoFrameInput() const;

	virtual eCalibrationPatternType getCalibrationPatternType() const = 0;
	virtual bool findNewCalibrationPattern(const float minSeperationDist= 0.f) = 0;
	virtual bool estimateNewCalibrationPatternPose(glm::dmat4& outCameraToPatternXform);
	virtual bool fetchLastFoundCalibrationPattern(
		t_opencv_point2d_list& outImagePoints, 
		t_opencv_pointID_list& outImagePointIDs,
		cv::Point2f outBoundingQuad[4]) = 0;

	bool areCurrentImagePointsValid() const;
	inline float getFrameWidth() const { return m_frameWidth; }
	inline float getFrameHeight() const { return m_frameHeight; }
	inline VideoFrameDistortionView* getDistortionView() const { return m_distortionView; }
	inline void getOpenCVLensCalibrationGeometry(OpenCVCalibrationGeometry* outGeometry) const { *outGeometry = m_opencvLensCalibrationGeometry; };
	inline void getOpenCVSolvePnPGeometry(OpenCVCalibrationGeometry* outGeometry) const { *outGeometry= m_opencvSolvePnPGeometry; };
	inline void getOpenGLSolvePnPGeometry(OpenGLCalibrationGeometry* outGeometry) const { *outGeometry= m_openglSolvePnPGeometry; };
	virtual void renderCalibrationPattern2D() const;
	virtual void renderSolvePnPPattern3D(const glm::mat4& xform) const;

protected:
	// Video buffer state
	class VideoFrameDistortionView* m_distortionView;

	float m_frameWidth;
	float m_frameHeight;

	// Internal Calibration State
	OpenCVCalibrationGeometry m_opencvLensCalibrationGeometry;
	OpenCVCalibrationGeometry m_opencvSolvePnPGeometry;
	OpenGLCalibrationGeometry m_openglSolvePnPGeometry;
	t_opencv_point2d_list m_lastValidQuad;
	t_opencv_point2d_list m_lastValidImagePoints;
	t_opencv_point2d_list m_currentImagePoints;
};

class CalibrationPatternFinder_Chessboard : public CalibrationPatternFinder
{
public:
	CalibrationPatternFinder_Chessboard(
		VideoFrameDistortionView* distortionView,
		int m_chessbordRows,
		int m_chessbordCols,
		float squareLengthMM);

	virtual eCalibrationPatternType getCalibrationPatternType() const override { return eCalibrationPatternType::mode_chessboard; }
	virtual bool findNewCalibrationPattern(const float minSeperationDist = 0.f) override;
	virtual bool fetchLastFoundCalibrationPattern(
		t_opencv_point2d_list& outImagePoints,
		t_opencv_pointID_list& outImagePointIDs,
		cv::Point2f outBoundingQuad[4]) override;

protected:
	int m_chessbordRows;
	int m_chessbordCols;
	float m_squareLengthMM;
};

class CalibrationPatternFinder_Charuco : public CalibrationPatternFinder
{
public:
	CalibrationPatternFinder_Charuco(
		VideoFrameDistortionView* distortionView,
		int charucoRows,
		int charucoCols,
		float charucoSquareLengthMM,
		float charucoMarkerLengthMM,
		eCharucoDictionaryType charucoDictionaryType);
	virtual ~CalibrationPatternFinder_Charuco();

	virtual eCalibrationPatternType getCalibrationPatternType() const override { return eCalibrationPatternType::mode_charuco; }
	virtual bool findNewCalibrationPattern(const float minSeperationDist = 0.f) override;
	virtual bool fetchLastFoundCalibrationPattern(
		t_opencv_point2d_list& outImagePoints, 
		t_opencv_pointID_list& outImagePointIDs,
		cv::Point2f outBoundingQuad[4]) override;
	virtual void renderCalibrationPattern2D() const override;
	virtual void renderSolvePnPPattern3D(const glm::mat4& xform) const override;

protected:
	class CharucoBoardData* m_markerData;
};

class CalibrationPatternFinder_Aruco : public CalibrationPatternFinder
{
public:
	CalibrationPatternFinder_Aruco(
		VideoFrameDistortionView* distortionView,
		int desiredArucoId,
		float markerLengthMM,
		eCharucoDictionaryType charucoDictionaryType);
	virtual ~CalibrationPatternFinder_Aruco();

	virtual eCalibrationPatternType getCalibrationPatternType() const override 
	{ return eCalibrationPatternType::mode_aruco; }
	virtual bool findNewCalibrationPattern(const float minSeperationDist = 0.f) override;
	virtual bool fetchLastFoundCalibrationPattern(
		t_opencv_point2d_list& outImagePoints,
		t_opencv_pointID_list& outImagePointIDs,
		cv::Point2f outBoundingQuad[4]) override;
	virtual void renderCalibrationPattern2D() const override;

protected:
	class ArucoBoardData* m_markerData;
};