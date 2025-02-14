#pragma once

#include "ComponentFwd.h"
#include "DeviceViewFwd.h"
#include "MikanRendererFwd.h"
#include "Transform.h"

#include <memory>
#include <string>

#include "glm/ext/quaternion_double.hpp"
#include "glm/ext/vector_double3.hpp"
#include "glm/ext/matrix_double4x4.hpp"

typedef int32_t MikanStencilID;

class StencilAligner
{
public:
	StencilAligner(
		VRDevicePoseViewPtr cameraTrackingPuckPoseView,
		class VideoFrameDistortionView* distortionView,
		ModelStencilComponentPtr modelStencil);
	virtual ~StencilAligner();

	bool hasFinishedPointSampling() const;
	void resetCalibrationState();

	void samplePixel(const glm::vec2& pixel);
	void sampleVertex(const glm::vec3& localVertex);
	bool computeStencilTransform(glm::mat4& outStencilTransform);

	void renderPixelSamples();
	void renderVertexSamples();

protected:
	static const int DESIRED_SAMPLE_COUNT = 4;

	float m_frameWidth;
	float m_frameHeight;

	// Internal Calibration State
	struct StencilAlignmentState* m_calibrationState;

	// Tracking puck used for calibration
	VRDevicePoseViewPtr m_cameraTrackingPuckPoseView;

	// Video buffer state
	class VideoFrameDistortionView* m_distortionView;

	// Model stencil to align
	ModelStencilComponentPtr m_modelStencil;
};