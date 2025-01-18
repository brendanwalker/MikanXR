#pragma once

#include "DeviceViewFwd.h"
#include "Transform.h"

#include <memory>
#include <string>

#include "glm/ext/quaternion_double.hpp"
#include "glm/ext/vector_double3.hpp"
#include "glm/ext/matrix_double4x4.hpp"

typedef int32_t MikanSpatialAnchorID;

struct AnchorTriangulatorInfo
{
	MikanSpatialAnchorID anchorId;
	GlmTransform worldTransform;
	std::string anchorName;
};

class AnchorTriangulator
{
public:
	AnchorTriangulator(
		VRDevicePoseViewPtr cameraTrackingPuckPoseView,
		class VideoFrameDistortionView* distortionView);
	virtual ~AnchorTriangulator();

	bool hasFinishedInitialPointSampling() const;
	bool hasFinishedTriangulatedPointSampling() const;
	void resetCalibrationState();

	void sampleCameraPose();
	void sampleMouseScreenPosition();
	void computeCurrentTriangulation();
	bool computeAnchorTransform(AnchorTriangulatorInfo& anchorInfo);

	void renderInitialPoint2dSegements();
	void renderCurrentPointTriangulation();
	void renderInitialPoint3dRays();
	void renderAllTriangulatedPoints(bool bShowCameraFrustum);
	void renderAnchorTransform();

protected:
	glm::vec2 computeMouseScreenPosition() const;
	void computeCameraRayAtPixel(
		const glm::mat4 cameraXform,
		const glm::vec2& imagePoint,
		glm::vec3& outRayStart,
		glm::vec3& outRayDirection) const;

	float m_frameWidth;
	float m_frameHeight;

	// Internal Calibration State
	struct AnchorTriangulationState* m_calibrationState;

	// Tracking puck used for calibration
	VRDevicePoseViewPtr m_cameraTrackingPuckPoseView;

	// Video buffer state
	class VideoFrameDistortionView* m_distortionView;
};