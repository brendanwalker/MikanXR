#pragma once

#include "IUsbVideoDevice.h"

#include <string>
#include <map>

class MikanUsbVideoDevice : public IUsbVideoDevice
{
public:
	MikanUsbVideoDevice(class MikanWMFVideoDeviceManager* ownerDeviceManager);
	virtual ~MikanUsbVideoDevice();

	// -- Device Listener
	virtual void addListener(IUsbVideoDeviceListener* listener) override;
	virtual void removeListener(IUsbVideoDeviceListener* listener) override;

	// -- Device Properties
	virtual size_t getDeviceIndex() const override;
	virtual const char* getDevicePath() const override;
	virtual const char* getFriendlyName() const override;

	// -- Video Mode
	virtual size_t getAvailableVideoModesCount() const override;
	virtual bool getVideoModeProperties(size_t index, UsbVideoModeProperties& outProperties) const override;
	virtual int getVideoModeIndex() const override;
	virtual const char* getVideoModeName() const override;
	virtual bool setVideoModeByName(const char* szVideoModeName) override;
	virtual bool setVideoModeByIndex(size_t index) override;

	// -- Camera Settings
	virtual bool getCameraSettingConstraint(const eUsbCameraSettingType property_type, UsbCameraSettingConstraint& outConstraint) const override;
	virtual void setCameraSetting(const eUsbCameraSettingType property_type, int desired_value, bool save_setting) override;
	virtual int getCameraSetting(const eUsbCameraSettingType property_type) const override;

	// -- Video Streaming
	virtual eUsbVideoStreamingStatus startVideoStream() override;
	virtual eUsbVideoStreamingStatus getVideoStreamingStatus() const override;
	virtual void stopVideoStream() override;

private:
	class MikanWMFVideoDeviceManager* m_ownerDeviceManager;
};

