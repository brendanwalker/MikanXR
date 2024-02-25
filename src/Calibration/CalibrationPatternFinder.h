#pragma once

#include "ObjectSystemConfigFwd.h"
#include "ProfileConfig.h"

#include <memory>
#include "opencv2/core/types.hpp"

#include "glm/ext/vector_float3.hpp"

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

typedef std::vector<cv::Point2f> t_opencv_point2d_list;
typedef std::vector<cv::Point3f> t_opencv_point3d_list;

typedef std::vector<glm::vec3> t_opengl_point3d_list;

struct OpenCVCalibrationGeometry
{
	t_opencv_point3d_list points;
};

struct OpenGLCalibrationGeometry
{
	t_opengl_point3d_list points;
};

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

	virtual eCalibrationPatternType getCalibrationPatternType() const = 0;
	virtual bool findNewCalibrationPattern(const float minSeperationDist= 0.f) = 0;
	virtual bool fetchLastFoundCalibrationPattern(
		t_opencv_point2d_list& outImagePoints, cv::Point2f outBoundingQuad[4]) = 0;

	bool areCurrentImagePointsValid() const;
	inline float getFrameWidth() const { return m_frameWidth; }
	inline float getFrameHeight() const { return m_frameHeight; }
	inline VideoFrameDistortionView* getDistortionView() const { return m_distortionView; }
	inline void getOpenCVLensCalibrationGeometry(OpenCVCalibrationGeometry* outGeometry) { *outGeometry = m_opencvLensCalibrationGeometry; };
	inline void getOpenCVSolvePnPGeometry(OpenCVCalibrationGeometry* outGeometry) { *outGeometry= m_opencvSolvePnPGeometry; };
	inline void getOpenGLSolvePnPGeometry(OpenGLCalibrationGeometry* outGeometry) { *outGeometry= m_openglSolvePnPGeometry; };
	void renderCalibrationPattern2D() const;
	void renderSolvePnPPattern3D(const glm::mat4& xform) const;

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

	float* getSquareLengthMMPtr() { return &m_squareLengthMM; }

	virtual eCalibrationPatternType getCalibrationPatternType() const override { return eCalibrationPatternType::mode_chessboard; }
	virtual bool findNewCalibrationPattern(const float minSeperationDist) override;
	virtual bool fetchLastFoundCalibrationPattern(t_opencv_point2d_list& outImagePoints, cv::Point2f outBoundingQuad[4]) override;

protected:
	int m_chessbordRows;
	int m_chessbordCols;
	float m_squareLengthMM;
};

class CalibrationPatternFinder_CircleGrid : public CalibrationPatternFinder
{
public:
	CalibrationPatternFinder_CircleGrid(
		VideoFrameDistortionView* distortionView,
		int circleGridRows,
		int circleGridCols,
		float circleSpacingMM,
		float circleDiameterMM);

	float* getCircleSpacingMMPtr() { return &m_circleSpacingMM; }
	float* getCircleDiameterMMPtr() { return &m_circleDiameterMM; }

	virtual eCalibrationPatternType getCalibrationPatternType() const override { return eCalibrationPatternType::mode_circlegrid; }
	virtual bool findNewCalibrationPattern(const float minSeperationDist) override;
	virtual bool fetchLastFoundCalibrationPattern(t_opencv_point2d_list& outImagePoints, cv::Point2f outBoundingQuad[4]) override;

protected:
	int m_circleGridRows;
	int m_circleGridCols;
	float m_circleSpacingMM;
	float m_circleDiameterMM;
};