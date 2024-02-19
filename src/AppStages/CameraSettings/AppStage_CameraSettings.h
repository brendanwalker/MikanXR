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
	AppStage_CameraSettings(class MainWindow* ownerWindow);
	virtual ~AppStage_CameraSettings();

	virtual void enter() override;
	virtual void exit() override;
	virtual void pause() override;
	virtual void resume() override;
	virtual void update(float deltaSeconds) override;
	virtual void render() override;

	virtual void onRmlClickEvent(const std::string& value);

	static const char* APP_STAGE_NAME;

protected:
	void startVideoSource(VideoSourceViewPtr videoSource);
	void stopVideoSource(VideoSourceViewPtr videoSource);

	void onVideoDisplayModeChanged(const std::string& newModeName);

	struct CameraSettingsDataModel* m_dataModel = nullptr;

	class VideoSourceListIterator* m_videoSourceIterator= nullptr;
	class VideoFrameDistortionView* m_videoBufferView = nullptr;
};