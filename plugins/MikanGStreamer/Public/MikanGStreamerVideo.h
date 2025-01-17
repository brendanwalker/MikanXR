#pragma once

#include "MikanGStreamerExport.h"
#include "MikanGStreamerConstants.h"

struct MikanGStreamerSettings
{
	eGStreamerProtocol protocol;
	const char* address;
	const char* path;
	int port;
};

struct MikanGStreamerVideoMode
{
	int bufferPixelWidth;
	int bufferPixelHeight;
	double frameRate;
	char modeName[256];
	char bufferFormat[32];
};

struct MikanGStreamerBuffer
{
	unsigned char* data;
	size_t byte_count;
};

class IMikanGStreamerVideoDevice
{
public:
	IMikanGStreamerVideoDevice()= default;
	virtual ~IMikanGStreamerVideoDevice()= default;

	// Create / Destroy a GStreamer Video Pipeline based on the settings
	virtual bool open()= 0;
	virtual bool getIsOpen() const= 0;
	virtual void close()= 0;
	
	// Start / Stop an opened pipeline
	virtual bool startVideoStream()= 0;
	virtual bool getIsVideoStreaming()= 0;
	virtual void stopVideoStream()= 0;

	// Try and fetch the next video frame from a started pipeline
	virtual void tryPullSample(
		const MikanGStreamerVideoMode& inVideoMode, 
		void (*onVideoModeChanged)(const MikanGStreamerVideoMode& newVideoMode, void* userdata),
		void (*onVideoFrameReceived)(const MikanGStreamerBuffer& newBuffer, void* userdata),
		void* userdata)= 0;
};

// C-API
MIKAN_GSTREAMER_CAPI(IMikanGStreamerVideoDevice*) MikanGStreamerVideoDeviceAllocate(const MikanGStreamerSettings& settings);
MIKAN_GSTREAMER_CAPI(void) MikanGStreamerVideoDeviceDispose(IMikanGStreamerVideoDevice* device);