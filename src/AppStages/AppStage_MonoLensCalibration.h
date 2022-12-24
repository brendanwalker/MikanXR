#pragma once

//-- includes -----
#include "AppStage.h"
#include <memory>

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

namespace Rml
{
	class ElementDocument;
};

//-- definitions -----
enum class eMonoLensCalibrationMenuState : int
{
	inactive = 0,
	capture = 1,
	processingCalibration = 2,
	testCalibration = 3,
	failedCalibration = 4,
	failedVideoStartStreamRequest = 5,
};

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

private:
	struct MonoLensCalibrationDataModel* m_dataModel = nullptr;
	Rml::ElementDocument* m_cameraSettingsDoc= nullptr;

	VideoSourceViewPtr m_videoSourceView;
	class MonoLensDistortionCalibrator* m_monoLensCalibrator;
	class VideoFrameDistortionView* m_monoDistortionView;
};