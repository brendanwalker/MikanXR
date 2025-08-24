#pragma once

#include "CalibrationPatternFinder.h"

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
