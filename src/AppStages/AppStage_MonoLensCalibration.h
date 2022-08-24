#pragma once

//-- includes -----
#include "AppStage.h"
#include <memory>

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

//-- definitions -----
class AppStage_MonoLensCalibration : public AppStage
{
public:
	static const char* APP_STAGE_NAME;

	AppStage_MonoLensCalibration(class App* app);

	inline void setBypassCalibrationFlag(bool flag) { m_bypassCalibrationFlag = flag; }

	virtual void enter() override;
	virtual void exit() override;
	virtual void update() override;
	virtual void render() override;
	virtual void renderUI() override;

protected:
	void captureRequested();
	void renderCameraSettingsUI();

private:
	enum eMenuState
	{
		inactive,
		capture,
		processingCalibration,
		testCalibration,
		failedCalibration,
		failedVideoStartStreamRequest,
	};

	// Menu state
	eMenuState m_menuState= inactive;

	// Tracker Settings state
	int m_brightness = 0;
	int m_brightnessMin = 0;
	int m_brightnessMax = 0;
	int m_brightnessStep = 0;
	bool m_bypassCalibrationFlag= false;

	VideoSourceViewPtr m_videoSourceView;
	class MonoLensDistortionCalibrator* m_monoLensCalibrator;
	class VideoFrameDistortionView* m_monoDistortionView;
};