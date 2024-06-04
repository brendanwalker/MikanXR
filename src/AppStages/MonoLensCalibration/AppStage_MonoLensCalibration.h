#pragma once

//-- includes -----
#include "AppStage.h"
#include "Constants_MonoLensCalibration.h"
#include "RmlFwd.h"
#include "VideoDisplayConstants.h"
#include <memory>

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

//-- definitions -----
class AppStage_MonoLensCalibration : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_MonoLensCalibration(class MainWindow* ownerWindow);
	virtual ~AppStage_MonoLensCalibration();

	void setBypassCalibrationFlag(bool flag);

	virtual void enter() override;
	virtual void exit() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;

protected:
	void setMenuState(eMonoLensCalibrationMenuState newState);
	void captureRequested();

	// Calibration Model UI Events
	void onRestartEvent();
	void onReturnEvent();
	void onCancelEvent();

	// Camera Settings Model UI Events
	void onVideoDisplayModeChanged(eVideoDisplayMode newDisplayMode);

private:
	class RmlModel_MonoLensCalibration* m_calibrationModel = nullptr;
	Rml::ElementDocument* m_calibrationView= nullptr;

	class RmlModel_MonoCameraSettings* m_cameraSettingsModel = nullptr;
	Rml::ElementDocument* m_cameraSettingsView= nullptr;

	VideoSourceViewPtr m_videoSourceView;
	class MonoLensDistortionCalibrator* m_monoLensCalibrator;
	class VideoFrameDistortionView* m_monoDistortionView;
};