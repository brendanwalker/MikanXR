#pragma once

//-- includes -----
#include "AppStage.h"
#include "ComponentFwd.h"
#include "Shared/RmlDataBinding_Fwd.h"

#include <memory>
#include <vector>

class VideoFrameDistortionView;
typedef std::shared_ptr<VideoFrameDistortionView> VideoFrameDistortionViewPtr;


//-- definitions -----
class AppStage_VideoSourceSettings : public AppStage
{
public:
	AppStage_VideoSourceSettings(class IEditorWindow* ownerWindow);
	virtual ~AppStage_VideoSourceSettings();

	void setVideoSourceComponent(VideoSourceComponentPtr videoSourceComponent)
	{
		m_videoSourceComponent = videoSourceComponent;
	}

	virtual void enter() override;
	virtual void exit() override;
	virtual void pause() override;
	virtual void resume() override;
	virtual void update(float deltaSeconds) override;
	virtual void render(IMkViewportPtr targetViewport) override;

	static const char* APP_STAGE_NAME;

protected:
	// UI Events
	void onReturnEvent();

	// Video Source Events
	void onVideoSourceStarted(VideoSourceComponentPtr videoSource);
	void onVideoSourceStopped(VideoSourceComponentPtr videoSource);
	void onVideoSourceFrameSizeChanged(VideoSourceComponentPtr videoSource);

	Rml::ElementDocument* m_videoSourceSettingsView = nullptr;

	VideoSourceComponentWeakPtr m_videoSourceComponent;
	VideoFrameDistortionViewPtr m_videoBufferView;
};