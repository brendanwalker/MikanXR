#pragma once

// -- includes -----
#include "IVideoDevice.h"

struct UsbVideoModeProperties
{
	size_t index; // index of the video mode in the device's list of available modes
	const char* name; // name of the video mode (e.g., "1080p30", "720p60")
	int width; // width of the video frame in pixels
	int height; // height of the video frame in pixels
	int stride; // stride is the number of bytes per row in the video frame buffer
	int frame_rate_numerator; // numerator of the frame rate (e.g., 30 for 30 fps)
	int frame_rate_demonenator; // denominator of the frame rate (e.g., 1 for 30 fps)
};

struct UsbVideoFrameBuffer
{
	const uint8_t* data;
	size_t byte_count;
};

class IUsbVideoDeviceListener
{
public:
	// Called when the video source has been disconnected
	virtual void notifyVideoDeviceDisconnected(const class IUsbVideoDevice* device) = 0;

	// Called when the video source has updated its dimensions or other properties
	virtual void notifyVideoModePropertiesChanged(const class IUsbVideoDevice* device) = 0;

	// Called when new video frame has been received from the video source
	virtual void notifyVideoFrameReceived(const UsbVideoFrameBuffer& bufferInfo) = 0;
};

// UsbVideoDevice interface
class IUsbVideoDevice : public IVideoDevice
{
public:
	IUsbVideoDevice() = default;
	virtual ~IUsbVideoDevice() {}

	// -- Device Listener
	virtual void addListener(IUsbVideoDeviceListener* listener) = 0;
	virtual void removeListener(IUsbVideoDeviceListener* listener) = 0;

    // -- Device Properties
	virtual const char* getDevicePath() const = 0;
	virtual const char* getFriendlyName() const = 0;

	// -- Device Activation
	virtual bool getIsOpen() const = 0;
	virtual bool open() = 0;
	virtual void close() = 0;

	// -- Video Mode
	virtual size_t getAvailableVideoModesCount() const = 0;
	virtual bool getVideoModeProperties(size_t index, UsbVideoModeProperties& outProperties) const = 0;
	virtual int getVideoModeIndex() const = 0;
	virtual const char* getVideoModeName() const = 0;
	virtual bool setVideoModeByName(const char* szVideoModeName) = 0;
	virtual bool setVideoModeByIndex(size_t index) = 0;

	// -- Video Streaming
	virtual eVideoStreamingStatus startVideoStream() = 0;
	virtual eVideoStreamingStatus getVideoStreamingStatus() const = 0;
	virtual void stopVideoStream() = 0;
};

using IUsbVideoDevicePtr = std::shared_ptr<IUsbVideoDevice>;
using IUsbVideoDeviceConstPtr = std::shared_ptr<const IUsbVideoDevice>;
using IUsbVideoDeviceWeakPtr = std::weak_ptr<IUsbVideoDevice>;
