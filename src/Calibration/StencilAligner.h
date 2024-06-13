#pragma once

#include "RendererFwd.h"
#include "Transform.h"

#include <memory>
#include <string>

#include "glm/ext/quaternion_double.hpp"
#include "glm/ext/vector_double3.hpp"
#include "glm/ext/matrix_double4x4.hpp"

typedef int32_t MikanStencilID;

class VRDeviceView;
typedef std::shared_ptr<VRDeviceView> VRDeviceViewPtr;

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

struct StencilAlignerInfo
{
	MikanStencilID stencilId;
	GlmTransform relativeTransform; // Relative to origin anchor
	std::string stencilName;
};

class StencilAligner
{
public:
	StencilAligner(
		VRDeviceViewPtr cameraTrackingPuckView,
		class VideoFrameDistortionView* distortionView);
	virtual ~StencilAligner();

	bool hasFinishedPointSampling() const;
	void resetCalibrationState();

	//void sampleCameraPose();
	void sampleMouseScreenPosition();
	void sampleVertex(const glm::vec3& localVertex);
	bool computeStencilTransform(StencilAlignerInfo& anchorInfo);

	//void renderInitialPoint2dSegements();
	//void renderAllTriangulatedPoints(bool bShowCameraFrustum);
	//void renderAnchorTransform();

protected:
	static const int DESIRED_SAMPLE_COUNT = 4;

	glm::vec2 computeMouseScreenPosition() const;

	float m_frameWidth;
	float m_frameHeight;

	// Internal Calibration State
	struct StencilAlignmentState* m_calibrationState;

	// Tracking puck used for calibration
	VRDeviceViewPtr m_cameraTrackingPuckView;

	// Video buffer state
	class VideoFrameDistortionView* m_distortionView;
};