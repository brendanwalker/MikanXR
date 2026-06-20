#pragma once

// -- includes -----
#include <memory>

// -- definitions -----
enum class eVideoOpeningStatus : int
{
	failed= -1,
	closed= 0,
	opening= 1,
	open= 2,
};

enum class eVideoStreamingStatus : int
{
	failed= -1,
	stopped= 0,
	pendingStart= 1,
	started= 2,
};

/// The list of possible camera drivers used by Mikan
enum class eVideoSettingType : int
{
	INVALID= -1,

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
	int min_value= 0;
	int max_value= 0;
	int stepping_delta= 0;
	int default_value= 0;
	bool is_automatic= false;
};

// VideoDevice interface
class IVideoDevice
{
public:
	IVideoDevice()= default;
	virtual ~IVideoDevice() {}

	// -- Device Properties
	virtual const char* getDevicePath() const= 0;
	virtual const char* getFriendlyName() const= 0;

	// -- Video Settings
	virtual bool isVideoSettingSupported(const eVideoSettingType property_type) const= 0;
	virtual bool getVideoSettingConstraint(const eVideoSettingType property_type, VideoSettingConstraint& outConstraint) const= 0;
	virtual void setVideoSetting(const eVideoSettingType property_type, int desired_value)= 0;
	virtual int getVideoSetting(const eVideoSettingType property_type) const= 0;
};

using IVideoDevicePtr= std::shared_ptr<IVideoDevice>;
using IVideoDeviceConstPtr= std::shared_ptr<const IVideoDevice>;
using IVideoDeviceWeakPtr= std::weak_ptr<IVideoDevice>;
