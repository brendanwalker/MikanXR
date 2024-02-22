#pragma once

#include "MikanMathTypes.h"
#include "ObjectSystemConfigFwd.h"

#include <memory>

#include "glm/ext/quaternion_double.hpp"
#include "glm/ext/vector_double3.hpp"
#include "glm/ext/matrix_double4x4.hpp"

class VRDeviceView;
typedef std::shared_ptr<VRDeviceView> VRDeviceViewPtr;

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class MonoLensDepthMeshCapture
{
public:
	MonoLensDepthMeshCapture(
		ProfileConfigConstPtr profileConfig,
		VRDeviceViewPtr cameraTrackingPuckView,
		class VideoFrameDistortionView* distortionView,
		int desiredSampleCount);
	virtual ~MonoLensDepthMeshCapture();

	inline class CalibrationPatternFinder* getPatternFinder() const { return m_patternFinder; }

	bool hasFinishedSampling() const;
	float getCalibrationProgress() const;
	void resetCalibrationState();

	bool computeCameraToPuckXform();
	glm::mat4 getLastCameraPose(VRDeviceViewPtr attachedVRDevicePtr) const;
	void sampleLastCameraToPuckXform();
	bool computeCalibratedCameraTrackerOffset(MikanQuatd& outRotationOffset, MikanVector3d& outTranslationOffset);

	void renderCameraSpaceCalibrationState();
	void renderVRSpaceCalibrationState();

protected:

	float frameWidth;
	float frameHeight;

	// Internal Calibration State
	struct MonoLensDepthMeshCaptureState* m_calibrationState;

	// Tracking pucks used for calibration
	VRDeviceViewPtr m_cameraTrackingPuckView;

	// Video buffer state
	class VideoFrameDistortionView* m_distortionView;

	// Calibration pattern being used
	class CalibrationPatternFinder* m_patternFinder;
};