#pragma once

// -- includes -----
#include "GStreamerCameraEnumerator.h"
#include "GStreamerVideoConfig.h"
#include "VideoCapabilitiesConfig.h"
#include "WorkerThread.h"
#include "VideoFwd.h"

// -- definitions -----
class GStreamerVideoDevice
{
public:
	GStreamerVideoDevice(const int deviceIndex, const std::string& cameraURI);
	~GStreamerVideoDevice();

	bool open(GStreamerVideoConfigPtr cfg, class IVideoSourceListener* trackerListener);
	bool getIsOpen() const;
	void tryPullSample();
	void close();

	bool startVideoStream();
	bool getIsVideoStreaming();
	void stopVideoStream();

public:
	int m_deviceIndex;
	std::string m_cameraURI;
	class GStreamerVideoDeviceImpl* m_impl;
};
