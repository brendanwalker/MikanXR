#pragma once

//-- includes -----
#include "AppStage.h"
#include "Constants_CTOffsetCalibration.h"
#include "DeviceViewFwd.h"
#include "MikanRendererFwd.h"
#include "VideoDisplayConstants.h"
#include <memory>

//-- definitions -----
class AppStage_CTOffsetCalibration : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_CTOffsetCalibration(class MainWindow* ownerWindow);
	virtual ~AppStage_CTOffsetCalibration();

	void setBypassCalibrationFlag(bool flag);

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;

protected:
	void updateCamera();
	void renderVRScene();
	void setMenuState(eCTOffsetCalibrationMenuState newState);

	// Calibration Model UI Events
	void onContinueEvent();
	void onCancelEvent();
	void onCaptureEvent();
	void onRestartEvent();

	// Camera Settings Model UI Events
	void onViewportModeChanged(eCTOffsetCalibrationViewpointMode newViewMode);
	void onBrightnessChanged(int newBrightness);
	void onVRFrameDelayChanged(int newVRFrameDelay);
	
private:
	class RmlModel_CTOffsetCalibration* m_calibrationModel = nullptr;
	Rml::ElementDocument* m_calibrationView = nullptr;

	class RmlModel_CTOffsetCameraSettings* m_cameraSettingsModel = nullptr;
	Rml::ElementDocument* m_cameraSettingsView = nullptr;

	bool m_bHasModifiedCameraSettings= false;

	VideoSourceViewPtr m_videoSourceView;

	// Tracking pucks used for calibration
	VRDevicePoseViewPtr m_cameraTrackingPuckPoseView;

	class CameraTrackerOffsetCalibrator* m_trackerPoseCalibrator;
	class VideoFrameDistortionView* m_monoDistortionView;

	GlScenePtr m_scene;
	GlCameraPtr m_camera;
};