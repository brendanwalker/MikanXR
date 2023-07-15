#pragma once

// -- includes -----
#include "CommonConfig.h"
#include "DeviceEnumerator.h"
#include "DeviceInterface.h"
#include "MikanClientTypes.h"
#include "WMFConfig.h"
#include "VideoFwd.h"

#include <string>
#include <vector>
#include <array>
#include <deque>

// -- definitions -----
class WMFStereoVideoConfig : public WMFVideoConfig
{
public:
    WMFStereoVideoConfig(const std::string &fnamebase = "WMFStereoTrackerConfig");
    
    virtual configuru::Config writeToJSON() override;
    virtual void readFromJSON(const configuru::Config &pt) override;

    MikanStereoIntrinsics tracker_intrinsics;

    static const int CONFIG_VERSION;
};

class WMFStereoVideoSource : public IVideoSourceInterface {
public:
    WMFStereoVideoSource();
    virtual ~WMFStereoVideoSource();
        
    // Stereo Tracker
    bool open(); // Opens the first virtual stereo tracker
    
    // -- IDeviceInterface
    bool matchesDeviceEnumerator(const DeviceEnumerator *enumerator) const override;
    bool open(const DeviceEnumerator *enumerator) override;
    bool getIsOpen() const override;
    void close() override;
    static eDeviceType getDeviceTypeStatic()
    { return eDeviceType::StereoVideoSource; }
    eDeviceType getDeviceType() const override;
    
    // -- IVideoSourceInterface
	bool startVideoStream() override;
    bool getIsVideoStreaming() const override;
	void stopVideoStream() override;
    eDriverType getDriverType() const override;
    std::string getFriendlyName() const;
    std::string getUSBDevicePath() const override;
    bool getVideoFrameDimensions(int *out_width, int *out_height, int *out_stride) const override;
    bool getIsStereoCamera() const override { return true; }
	bool getIsFrameMirrored() const override;
	bool getIsBufferMirrored() const override;
    void loadSettings() override;
    void saveSettings() override;
	bool getAvailableTrackerModes(std::vector<std::string> &out_mode_names) const override;
	const struct VideoModeConfig * getVideoMode() const override;
	bool setVideoMode(const std::string mode_name);
	double getFrameWidth() const override;
	double getFrameHeight() const override;
	double getFrameRate() const override;
	bool getVideoPropertyConstraint(const VideoPropertyType property_type, VideoPropertyConstraint &outConstraint) const override;
    void setVideoProperty(const VideoPropertyType property_type, int desired_value, bool save_setting) override;
    int getVideoProperty(const VideoPropertyType property_type) const override;
    void getCameraIntrinsics(MikanVideoSourceIntrinsics& out_tracker_intrinsics) const override;
    void setCameraIntrinsics(const MikanVideoSourceIntrinsics& tracker_intrinsics) override;
    MikanQuatd getCameraOffsetOrientation() const override;
    MikanVector3d getCameraOffsetPosition() const override;
	void setCameraPoseOffset(const MikanQuatd& q, const MikanVector3d& p) override;
    void getFOV(float &outHFOV, float &outVFOV) const override;
    void getZRange(float &outZNear, float &outZFar) const override;
	void setVideoSourceListener(IVideoSourceListener *listener) override;

    // -- Getters
    inline WMFStereoVideoConfigConstPtr getConfig() const
    { return m_cfg; }

protected:
	VideoCapabilitiesConfigConstPtr m_capabilities;
	const struct VideoModeConfig *m_currentMode;

    WMFStereoVideoConfigPtr m_cfg;
    std::string m_device_identifier;

	class WMFVideoDevice *m_videoDevice;
    IVideoSourceInterface::eDriverType m_DriverType;
    IVideoSourceListener*m_listener;
};
