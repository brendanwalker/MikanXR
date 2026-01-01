#pragma once

// -- includes -----
#include <memory>

// -- definitions -----
enum class eVideoStreamingStatus : int
{
	failed = -1,
	stopped = 0,
	pendingStart = 1,
	started = 2,
};

/// The list of possible camera drivers used by Mikan
enum class eVideoSettingType : int
{
	INVALID = -1,

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
struct VideoSettingConstraint
{
	int min_value;
	int max_value;
	int stepping_delta;
	int default_value;
	bool is_automatic;
};

// VideoDevice interface
class IVideoDevice
{
public:
	IVideoDevice() = default;
	virtual ~IVideoDevice() {}

    // -- Device Properties
	virtual const char* getDevicePath() const = 0;
	virtual const char* getFriendlyName() const = 0;

	// -- Video Settings
	virtual bool isVideoSettingSupported(const eVideoSettingType property_type) const = 0;
	virtual bool getVideoSettingConstraint(const eVideoSettingType property_type, VideoSettingConstraint& outConstraint) const = 0;
	virtual void setVideoSetting(const eVideoSettingType property_type, int desired_value) = 0;
	virtual int getVideoSetting(const eVideoSettingType property_type) const = 0;
};

using IVideoDevicePtr = std::shared_ptr<IVideoDevice>;
using IVideoDeviceConstPtr = std::shared_ptr<const IVideoDevice>;
using IVideoDeviceWeakPtr = std::weak_ptr<IVideoDevice>;
