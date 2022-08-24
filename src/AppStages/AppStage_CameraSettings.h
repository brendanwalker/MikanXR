#pragma once

//-- includes -----
#include "AppStage.h"
#include <memory>
#include <vector>

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;
typedef std::vector<VideoSourceViewPtr> VideoSourceList;

//-- definitions -----
class AppStage_CameraSettings : public AppStage
{
public:
	AppStage_CameraSettings(class App* app);
	virtual ~AppStage_CameraSettings();

	virtual void enter() override;
	virtual void exit() override;
	virtual void pause() override;
	virtual void resume() override;
	virtual void update() override;
	virtual void render() override;

	virtual void renderUI() override;

	static const char* APP_STAGE_NAME;

protected:
	void startVideoSource(VideoSourceViewPtr videoSource);
	void stopVideoSource(VideoSourceViewPtr videoSource);

	class VideoSourceListIterator* m_videoSourceIterator= nullptr;
	class VideoFrameDistortionView* m_videoBufferView = nullptr;
	int m_brightness= 0;
	int m_brightnessMin = 0;
	int m_brightnessMax = 0;
	int m_brightnessStep = 0;
};