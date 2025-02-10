#pragma once

#include "MikanMathTypes.h"
#include "ObjectSystemConfigFwd.h"
#include "MikanRendererFwd.h"

#include "glm/ext/matrix_float4x4.hpp"

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
		IMkWindow* ownerWindow,
		ProfileConfigConstPtr profileConfig,
		VideoFrameDistortionViewPtr distortionView,
		SyntheticDepthEstimatorPtr depthEstimator);
	virtual ~DepthMeshGenerator();

	inline CalibrationPatternFinderPtr getPatternFinder() const { return m_patternFinder; }

	bool loadMeshFromStencilDefinition(ModelStencilDefinitionPtr stencilDefinition);
	bool saveMeshToStencilDefinition(ModelStencilDefinitionPtr stencilDefinition, const glm::mat4& cameraXform);

	bool hasFinishedSampling() const;
	void resetCalibrationState();

	bool captureMesh();
	MikanRenderModelResourcePtr getCapturedDepthMeshResource() const;

	void renderCameraSpaceCalibrationState();

protected:

	float frameWidth;
	float frameHeight;

	// Internal Capture State
	struct DepthMeshCaptureState* m_calibrationState;

	// Undistorted Video Frame
	VideoFrameDistortionViewPtr m_distortionView;

	// Finds the calibration pattern in the video frame
	CalibrationPatternFinderPtr m_patternFinder;

	// Used to generate a depth buffer from the video frame and calibration pattern
	SyntheticDepthEstimatorPtr m_depthEstimator;
};