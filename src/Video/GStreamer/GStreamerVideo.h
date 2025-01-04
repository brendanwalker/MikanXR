#pragma once

// -- includes -----
#include "GStreamerCameraEnumerator.h"
#include "GStreamerVideoConfig.h"
#include "VideoCapabilitiesConfig.h"
#include "WorkerThread.h"
#include "VideoFwd.h"

#include <functional>

// -- definitions -----
class GStreamerVideoDevice
{
public:
	GStreamerVideoDevice(
		const int cameraIndex,
		GStreamerVideoConfigPtr cfg,
		class IVideoSourceListener* listener);
	~GStreamerVideoDevice();

	bool open();
	bool getIsOpen() const;
	void close();

	using VideoModeChangedCallback = std::function<void(VideoModeConfigPtr newVideoMode)>;
	void tryPullSample(VideoModeConfigPtr inVideoMode, VideoModeChangedCallback onVideoModeChanged);

	bool startVideoStream();
	bool getIsVideoStreaming();
	void stopVideoStream();

private:
	int m_cameraIndex;
	GStreamerVideoConfigPtr m_cfg;
	class IVideoSourceListener* m_videoSourceListener;
	class GStreamerVideoDeviceImpl* m_impl;
};
