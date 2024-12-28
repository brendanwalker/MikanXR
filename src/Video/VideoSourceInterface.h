#pragma once

// -- includes -----
#include "DeviceInterface.h"
#include "MikanMathTypes.h"

#include "glm/ext/quaternion_double.hpp"
#include "glm/ext/vector_double3.hpp"

#include <string>
#include <vector>

// -- definitions -----
/// The list of possible sub sections to extract from a video frame
enum class VideoFrameSection : int
{
	Left = 0, ///< The left frame from a stereo camera
	Right = 1, ///< The right frame from a stereo camera
	Primary = 0  ///< The only frame from a stereo camera
};

/// The list of possible camera drivers used by PSVRService
enum class VideoPropertyType : int
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
struct VideoPropertyConstraint
{
	int min_value;
	int max_value;
	int stepping_delta;
	int default_value;
	bool is_automatic;
	bool is_supported;
};

/// Interface class for video events. Implemented by VideoSourceView
class IVideoSourceListener
{
public:
	// Called when new video frame has been received from the video source
	virtual void notifyVideoFrameReceived(const unsigned char*) = 0;
};

/// Interface class for VideoSources
class IVideoSourceInterface : public IDeviceInterface
{
public:
    enum eDriverType
    {
        INVALID,

        OpenCV,
        WindowsMediaFramework,

        SUPPORTED_DRIVER_TYPE_COUNT,
    };

    // Returns the driver type being used by this camera
    virtual eDriverType getDriverType() const = 0;

	// Returns the friendly name of the device
	virtual std::string getFriendlyName() const { return ""; } 

    // Returns the full usb device path for the tracker
    virtual std::string getUSBDevicePath() const = 0;

    // Returns the video frame size (used to compute frame buffer size)
    virtual bool getVideoFrameDimensions(int *out_width, int *out_height, int *out_stride) const = 0;

    // Returns true if this device is a stereo camera
    virtual bool getIsStereoCamera() const = 0;

    // Returns true if the frame coming from the camera is mirrored backwards
    virtual bool getIsFrameMirrored() const = 0;

    // Returns true if the left and right frames coming from the camera are swapped
    virtual bool getIsBufferMirrored() const = 0;

    static const char *getDriverTypeString(eDriverType device_type)
    {
        const char *result = nullptr;

        switch (device_type)
        {
        case OpenCV:
            result = "OpenCV";
            break;          
        case WindowsMediaFramework:
            result = "Windows Media Framework";
            break;
        default:
            result = "UNKNOWN";
        }

        return result;
    }
    
    virtual void loadSettings() = 0;
    virtual void saveSettings() = 0;

	virtual bool getAvailableTrackerModes(std::vector<std::string> &out_mode_names) const = 0;
	virtual const struct VideoModeConfig *getVideoMode() const = 0;
	virtual bool setVideoMode(const std::string modeName) = 0;

    virtual bool startVideoStream() = 0;
    virtual bool getIsVideoStreaming() const = 0;
    virtual void stopVideoStream() = 0;

	virtual double getFrameWidth() const = 0;
	virtual double getFrameHeight() const = 0;
	virtual double getFrameRate() const = 0;

	virtual bool getVideoPropertyConstraint(const VideoPropertyType property_type, VideoPropertyConstraint &outConstraint) const = 0;

    virtual void setVideoProperty(const VideoPropertyType property_type, int desired_value, bool save_setting) = 0;
    virtual int getVideoProperty(const VideoPropertyType property_type) const = 0;

    virtual void getCameraIntrinsics(struct MikanVideoSourceIntrinsics& out_tracker_intrinsics) const = 0;
    virtual void setCameraIntrinsics(const struct MikanVideoSourceIntrinsics& tracker_intrinsics) = 0;

    virtual MikanQuatd getCameraOffsetOrientation() const = 0;
    virtual MikanVector3d getCameraOffsetPosition() const = 0;
    virtual void setCameraPoseOffset(const MikanQuatd& q, const MikanVector3d& v) = 0;

    virtual void getFOV(float &outHFOV, float &outVFOV) const = 0;
    virtual void getZRange(float &outZNear, float &outZFar) const = 0;

	// Assign a Tracker listener to send Tracker events to
	virtual void setVideoSourceListener(IVideoSourceListener* listener) = 0;
};
