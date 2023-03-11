#pragma once

//-- includes -----
#include "AppStage.h"
#include "Constants_FastenerCalibration.h"
#include "MikanClientTypes.h"
#include "VideoDisplayConstants.h"
#include <memory>

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class VRDeviceView;
typedef std::shared_ptr<VRDeviceView> VRDeviceViewPtr;

//-- definitions -----
class AppStage_FastenerCalibration : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_FastenerCalibration(class App* app);
	virtual ~AppStage_FastenerCalibration();

	void setBypassCalibrationFlag(bool flag);
	void setTargetFastenerId(MikanSpatialFastenerID fastenerId) { m_targetFastenerId= fastenerId; }

	virtual void enter() override;
	virtual void exit() override;
	virtual void update() override;
	virtual void render() override;

protected:
	void updateCamera();
	void renderVRScene();
	void setMenuState(eFastenerCalibrationMenuState newState);

	// Input Events
	void onMouseButtonUp(int button);

	// Calibration Model UI Events
	void onContinueEvent();
	void onRestartEvent();
	void onCancelEvent();
	void onReturnEvent();

	// Camera Settings Model UI Events
	void onViewportModeChanged(eFastenerCalibrationViewpointMode newViewMode);
	void onBrightnessChanged(int newBrightness);
	
private:
	class RmlModel_FastenerCalibration* m_calibrationModel = nullptr;
	Rml::ElementDocument* m_calibrationView = nullptr;

	class RmlModel_FastenerCameraSettings* m_cameraSettingsModel = nullptr;
	Rml::ElementDocument* m_cameraSettingsView = nullptr;

	VideoSourceViewPtr m_videoSourceView;

	// Tracking puck used for calibration
	VRDeviceViewPtr m_cameraTrackingPuckView;

	class FastenerCalibrator* m_fastenerCalibrator;
	class VideoFrameDistortionView* m_monoDistortionView;

	MikanSpatialFastenerID m_targetFastenerId;

	class GlScene* m_scene;
	class GlCamera* m_camera;
};