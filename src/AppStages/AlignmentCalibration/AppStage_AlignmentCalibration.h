#pragma once

//-- includes -----
#include "AppStage.h"
#include <memory>

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class VRDeviceView;
typedef std::shared_ptr<VRDeviceView> VRDeviceViewPtr;

//-- definitions -----
class AppStage_AlignmentCalibration : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_AlignmentCalibration(class App* app);
	virtual ~AppStage_AlignmentCalibration();

	inline void setBypassCalibrationFlag(bool flag) { m_bypassCalibrationFlag = flag; }

	virtual void enter() override;
	virtual void exit() override;
	virtual void update() override;
	virtual void render() override;
	virtual void renderUI() override;

protected:
	enum class eMenuState : int
	{
		inactive,
		verifySetup,
		capture,
		testCalibration,
		failedVideoStartStreamRequest,
	};

	enum class eViewpointMode : int
	{
		cameraViewpoint,
		vrViewpoint,
		mixedRealityViewpoint,

		COUNT
	};

	void renderVRScene();
	void renderCameraSettingsUI();
	void setViewpointMode(eViewpointMode viewMode);
	void updateCamera();

private:
	// Menu state
	eMenuState m_menuState = eMenuState::inactive;
	eViewpointMode m_viewMode= eViewpointMode::cameraViewpoint;

	// Tracker Settings state
	int m_uiBrightness = 0;
	bool m_bypassCalibrationFlag = false;

	VideoSourceViewPtr m_videoSourceView;

	// Tracking pucks used for calibration
	VRDeviceViewPtr m_cameraTrackingPuckView;
	VRDeviceViewPtr m_matTrackingPuckView;

	class MonoLensTrackerPoseCalibrator* m_trackerPoseCalibrator;
	class VideoFrameDistortionView* m_monoDistortionView;

	class GlScene* m_scene;
	class GlCamera* m_camera;
};