#pragma once

#include "MikanMathTypes.h"
#include "ObjectSystemConfigFwd.h"

#include <filesystem>
#include <memory>

class VideoFrameDistortionView;
typedef std::shared_ptr<VideoFrameDistortionView> VideoFrameDistortionViewPtr;

class CalibrationPatternFinder;
typedef std::shared_ptr<CalibrationPatternFinder> CalibrationPatternFinderPtr;

class SyntheticDepthEstimator;
typedef std::shared_ptr<SyntheticDepthEstimator> SyntheticDepthEstimatorPtr;

class DepthMeshGenerator
{
public:
	DepthMeshGenerator(
		IGlWindow* ownerWindow,
		ProfileConfigConstPtr profileConfig,
		VideoFrameDistortionViewPtr distortionView,
		SyntheticDepthEstimatorPtr depthEstimator);
	virtual ~DepthMeshGenerator();

	inline CalibrationPatternFinderPtr getPatternFinder() const { return m_patternFinder; }

	void setTargetModelStencilDefinition(ModelStencilDefinitionPtr targetModelStencilDefinition);
	bool loadMeshFromStencilDefinition();
	bool saveMeshToStencilDefinition();

	bool hasFinishedSampling() const;
	void resetCalibrationState();

	bool captureMesh();

	void renderCameraSpaceCalibrationState();
	void renderVRSpaceCalibrationState();

protected:

	float frameWidth;
	float frameHeight;

	// Internal Capture State
	struct DepthMeshCaptureState* m_calibrationState;

	// The target model stencil definition to read/write the depth mesh from/to
	ModelStencilDefinitionPtr m_targetModelStencilDefinition;

	// Undistorted Video Frame
	VideoFrameDistortionViewPtr m_distortionView;

	// Finds the calibration pattern in the video frame
	CalibrationPatternFinderPtr m_patternFinder;

	// Used to generate a depth buffer from the video frame and calibration pattern
	SyntheticDepthEstimatorPtr m_depthEstimator;
};