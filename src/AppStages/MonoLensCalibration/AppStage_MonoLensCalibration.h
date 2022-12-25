#pragma once

//-- includes -----
#include "AppStage.h"
#include "Constants_MonoLensCalibration.h"
#include <memory>

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

namespace Rml
{
	class ElementDocument;
};

//-- definitions -----
class AppStage_MonoLensCalibration : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_MonoLensCalibration(class App* app);
	virtual ~AppStage_MonoLensCalibration();

	void setBypassCalibrationFlag(bool flag);

	virtual void enter() override;
	virtual void exit() override;
	virtual void update() override;
	virtual void render() override;

protected:
	void setMenuState(eMonoLensCalibrationMenuState newState);
	void captureRequested();
	void updateVideoDisplayProperties();

	// Calibration Model Events
	void onRestartEvent();
	void onGotoMainMenuEvent();

private:
	class RmlModel_MonoLensCalibration* m_calibrationModel = nullptr;
	Rml::ElementDocument* m_calibrationView= nullptr;

	class RmlModel_MonoCameraSettings* m_cameraSettingsModel = nullptr;
	Rml::ElementDocument* m_cameraSettingsView= nullptr;

	VideoSourceViewPtr m_videoSourceView;
	class MonoLensDistortionCalibrator* m_monoLensCalibrator;
	class VideoFrameDistortionView* m_monoDistortionView;
};