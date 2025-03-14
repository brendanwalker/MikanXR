#pragma once

// -- includes -----
#include "OpenCVVideoConfig.h"
#include "DeviceEnumerator.h"
#include "DeviceInterface.h"
#include "VideoCapabilitiesConfig.h"
#include "VideoFwd.h"

#include <string>
#include <vector>
#include <deque>

// -- definitions -----
class OpenCVVideoSource : public IVideoSourceInterface {
public:
    OpenCVVideoSource(IVideoSourceListener* listener);
    virtual ~OpenCVVideoSource();
        
    bool open();
    
    // -- IDeviceInterface
    bool matchesDeviceEnumerator(const DeviceEnumerator *enumerator) const override;
    bool open(const DeviceEnumerator *enumerator) override;
    bool getIsOpen() const override;
    void close() override;
    static eDeviceType getDeviceTypeStatic()
    { return eDeviceType::MonoVideoSource; }
    eDeviceType getDeviceType() const override;
    
    // -- IVideoSourceInterface
	eVideoStreamingStatus startVideoStream() override;
    eVideoStreamingStatus getVideoStreamingStatus() const override;
	void stopVideoStream() override;
    IVideoSourceInterface::eDriverType getDriverType() const override;
    std::string getUSBDevicePath() const override;
    bool getVideoFrameDimensions(int *out_width, int *out_height, int *out_stride) const override;
    bool getIsStereoCamera() const override { return false; }
	bool getIsFrameMirrored() const override { return false; }
	bool getIsBufferMirrored() const override { return false; }
    void loadSettings() override;
    void saveSettings() override;
	bool getAvailableTrackerModes(std::vector<std::string> &out_mode_names) const override;
	const VideoModeConfig *getVideoMode() const override;
	bool setVideoMode(const std::string modeName) override;
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

    // -- Getters
    inline OpenCVVideoConfigConstPtr getConfig() const
    { return m_cfg; }

private:
	IVideoSourceListener* m_listener;

    VideoCapabilitiesConfigConstPtr m_videoCapabilities;
	int m_currentModeIndex;

    OpenCVVideoConfigPtr m_cfg;
    std::string m_deviceIdentifier;

    class OpenCVVideoDevice *m_videoDevice;
    IVideoSourceInterface::eDriverType m_driverType;
};
