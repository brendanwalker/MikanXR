#include "MikanWMFVideoDevice.h"
#include "MikanWMFVideoDeviceManager.h"


MikanUsbVideoDevice::MikanUsbVideoDevice(
	MikanWMFVideoDeviceManager* ownerDeviceManager)
	: m_ownerDeviceManager(ownerDeviceManager)
{
}

MikanUsbVideoDevice::~MikanUsbVideoDevice()
{
}

// -- Device Listener
void MikanUsbVideoDevice::addListener(IUsbVideoDeviceListener* listener)
{

}

void MikanUsbVideoDevice::removeListener(IUsbVideoDeviceListener* listener)
{

}

// -- Device Properties
size_t MikanUsbVideoDevice::getDeviceIndex() const
{
	return 0;
}

const char* MikanUsbVideoDevice::getDevicePath() const
{
	return nullptr;
}

const char* MikanUsbVideoDevice::getFriendlyName() const
{
	return nullptr;
}

// -- Video Mode
size_t MikanUsbVideoDevice::getAvailableVideoModesCount() const
{
	return 0;
}

bool MikanUsbVideoDevice::getVideoModeProperties(size_t index, UsbVideoModeProperties& outProperties) const
{
	return false;
}

int MikanUsbVideoDevice::getVideoModeIndex() const
{
	return -1;
}

const char* MikanUsbVideoDevice::getVideoModeName() const
{
	return nullptr;
}

bool MikanUsbVideoDevice::setVideoModeByName(const char* szVideoModeName)
{
	return false;
}

bool MikanUsbVideoDevice::setVideoModeByIndex(size_t index)
{
	return false;
}

// -- Camera Settings
bool MikanUsbVideoDevice::getCameraSettingConstraint(const eUsbCameraSettingType property_type, UsbCameraSettingConstraint& outConstraint) const
{
	return false;
}

void MikanUsbVideoDevice::setCameraSetting(const eUsbCameraSettingType property_type, int desired_value, bool save_setting)
{

}

int MikanUsbVideoDevice::getCameraSetting(const eUsbCameraSettingType property_type) const
{
	return 0;
}

// -- Video Streaming
eUsbVideoStreamingStatus MikanUsbVideoDevice::startVideoStream()
{
	return eUsbVideoStreamingStatus::failed;
}

eUsbVideoStreamingStatus MikanUsbVideoDevice::getVideoStreamingStatus() const
{
	return eUsbVideoStreamingStatus::failed;
}

void MikanUsbVideoDevice::stopVideoStream()
{

}
