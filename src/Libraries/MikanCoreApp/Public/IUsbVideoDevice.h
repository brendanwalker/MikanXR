#pragma once

// -- includes -----
#include <memory>

// -- definitions -----
enum class eUsbVideoStreamingStatus : int
{
	failed = -1,
	stopped = 0,
	pendingStart = 1,
	started = 2,
};

/// The list of possible camera drivers used by Mikan
enum class eUsbCameraSettingType : int
{
	Brightness,
	Contrast,
	Hue,
	Saturation,
	Sharpness,
	Gamma,
	WhiteBalance,
	RedBalance,
	GreenBalance,
	BlueBalance,
	Gain,
	Pan,
	Tilt,
	Roll,
	Zoom,
	Exposure,
	Iris,
	Focus,

	COUNT
};

/// Constraints on the values for a single tracker property
struct UsbCameraSettingConstraint
{
	int min_value;
	int max_value;
	int stepping_delta;
	int default_value;
	bool is_automatic;
	bool is_supported;
};

struct UsbVideoModeProperties
{
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
class IUsbVideoDevice
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

	// -- Video Mode
	virtual size_t getAvailableVideoModesCount() const = 0;
	virtual bool getVideoModeProperties(size_t index, UsbVideoModeProperties& outProperties) const = 0;
	virtual int getVideoModeIndex() const = 0;
	virtual const char* getVideoModeName() const = 0;
	virtual bool setVideoModeByName(const char* szVideoModeName) = 0;
	virtual bool setVideoModeByIndex(size_t index) = 0;

	// -- Camera Settings
	virtual bool getCameraSettingConstraint(const eUsbCameraSettingType property_type, UsbCameraSettingConstraint& outConstraint) const = 0;
	virtual void setCameraSetting(const eUsbCameraSettingType property_type, int desired_value) = 0;
	virtual int getCameraSetting(const eUsbCameraSettingType property_type) const = 0;

	// -- Video Streaming
	virtual eUsbVideoStreamingStatus startVideoStream() = 0;
	virtual eUsbVideoStreamingStatus getVideoStreamingStatus() const = 0;
	virtual void stopVideoStream() = 0;
};

using IUsbVideoDevicePtr = std::shared_ptr<IUsbVideoDevice>;
using IUsbVideoDeviceConstPtr = std::shared_ptr<const IUsbVideoDevice>;
using IUsbVideoDeviceWeakPtr = std::weak_ptr<IUsbVideoDevice>;
