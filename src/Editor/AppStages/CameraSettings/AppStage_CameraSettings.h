#pragma once

//-- includes -----
#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "AppStage.h"
#include <memory>
#include <vector>

class VideoFrameDistortionView;
typedef std::shared_ptr<VideoFrameDistortionView> VideoFrameDistortionViewPtr;

class RmlModel_CameraSettings;
using RmlModel_CameraSettingsPtr = std::shared_ptr<RmlModel_CameraSettings>;

//-- definitions -----
class AppStage_CameraSettings : public AppStage
{
public:
	AppStage_CameraSettings(class MainWindow* ownerWindow);
	virtual ~AppStage_CameraSettings();

	virtual void enter() override;
	virtual void exit() override;
	virtual void pause() override;
	virtual void resume() override;
	virtual void update(float deltaSeconds) override;
	virtual void render(IMkViewportPtr targetViewport) override;

	virtual void onRmlClickEvent(const std::string& value);

	static const char* APP_STAGE_NAME;

protected:
	RmlModel_CameraSettingsPtr m_cameraSettingsModel = nullptr;
	Rml::ElementDocument* m_cameraSettingsView = nullptr;

	VideoSourceComponentPtr m_videoSourceComponent;
	VideoFrameDistortionViewPtr m_videoBufferView;
};