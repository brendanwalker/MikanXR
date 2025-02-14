#pragma once

// -- includes -----
#include "MikanVideoSourceTypes.h"
#include "DeviceEnumerator.h"
#include "DeviceInterface.h"
#include "WMFConfig.h"
#include "VideoFwd.h"

#include <string>
#include <vector>
#include <array>
#include <deque>

// -- definitions -----
class WMFMonoVideoConfig : public WMFVideoConfig
{
public:
    WMFMonoVideoConfig(const std::string &fnamebase = "WMFMonoCameraConfig");
    
    virtual configuru::Config writeToJSON() override;
    virtual void readFromJSON(const configuru::Config &pt) override;

    bool areIntrinsicsUserCalibrated;
    MikanMonoIntrinsics tracker_intrinsics;

    static const int CONFIG_VERSION;
};

class WMFMonoVideoSource : public IVideoSourceInterface {
public:
    WMFMonoVideoSource(IVideoSourceListener* listener);
    virtual ~WMFMonoVideoSource();
        
    // Stereo Tracker
    bool open(); // Opens the first virtual stereo tracker
    
    // -- IDeviceInterface
    bool matchesDeviceEnumerator(const DeviceEnumerator *enumerator) const override;
    bool open(const DeviceEnumerator *enumerator) override;
    bool getIsOpen() const override;
    void close() override;
    static eDeviceType getDeviceTypeStatic()
    { return eDeviceType::MonoVideoSource; }
    eDeviceType getDeviceType() const override;
    
    // -- IVideoSourceInterface
	bool startVideoStream() override;
    bool getIsVideoStreaming() const override;
	void stopVideoStream() override;
    IVideoSourceInterface::eDriverType getDriverType() const override;
    std::string getFriendlyName() const override;
    std::string getUSBDevicePath() const override;
    bool getVideoFrameDimensions(int *out_width, int *out_height, int *out_stride) const override;
    bool getIsStereoCamera() const override { return false; }
	bool getIsFrameMirrored() const override;
	bool getIsBufferMirrored() const override;
    void loadSettings() override;
    void saveSettings() override;
	bool getAvailableTrackerModes(std::vector<std::string> &out_mode_names) const override;
	const struct VideoModeConfig * getVideoMode() const override;
	bool setVideoMode(const std::string modeName) override;
	double getFrameWidth() const override;
	double getFrameHeight() const override;
	double getFrameRate() const override;
	bool getVideoPropertyConstraint(const VideoPropertyType property_type, VideoPropertyConstraint &outConstraint) const override;
    void setVideoProperty(const VideoPropertyType property_type, int desired_value, bool save_setting) override;
    int getVideoProperty(const VideoPropertyType property_type) const override;
    void getCameraIntrinsics(MikanVideoSourceIntrinsics&out_tracker_intrinsics) const override;
    void setCameraIntrinsics(const MikanVideoSourceIntrinsics&tracker_intrinsics) override;
	MikanQuatd getCameraOffsetOrientation() const override;
	MikanVector3d getCameraOffsetPosition() const override;
	void setCameraPoseOffset(const MikanQuatd& q, const MikanVector3d& p) override;
    void getFOV(float &outHFOV, float &outVFOV) const override;
    void getZRange(float &outZNear, float &outZFar) const override;

    // -- Getters
    inline WMFMonoVideoConfigConstPtr getConfig() const
    { return m_cfg; }

private:
	IVideoSourceListener *m_listener;

	VideoCapabilitiesConfigConstPtr m_capabilities;
	const struct VideoModeConfig *m_currentMode;

    WMFMonoVideoConfigPtr m_cfg;
    std::string m_device_identifier;

	class WMFVideoDevice *m_videoDevice;
    IVideoSourceInterface::eDriverType m_DriverType;
};
