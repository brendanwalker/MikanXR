#include "MikanWMFVideoDevice.h"
#include "MikanWMFVideoDeviceManager.h"


MikanUsbVideoDevice::MikanUsbVideoDevice(
	MikanWMFVideoDeviceManager* ownerDeviceManager,
	const WMFDeviceInfo& deviceInfo)
	: m_ownerDeviceManager(ownerDeviceManager)
	, m_deviceInfo(deviceInfo)
{
}

MikanUsbVideoDevice::~MikanUsbVideoDevice()
{
}

// -- Device Listener
void MikanUsbVideoDevice::addListener(IUsbVideoDeviceListener* listener)
{
	m_listeners.insert(listener);
}

void MikanUsbVideoDevice::removeListener(IUsbVideoDeviceListener* listener)
{
	m_listeners.erase(listener);
}

// -- Device Properties
const char* MikanUsbVideoDevice::getDevicePath() const
{
	return m_deviceInfo.deviceSymbolicLink.c_str();
}

const char* MikanUsbVideoDevice::getFriendlyName() const
{
	return m_deviceInfo.deviceFriendlyName.c_str();
}

// -- Video Mode
size_t MikanUsbVideoDevice::getAvailableVideoModesCount() const
{
	return m_deviceInfo.deviceAvailableFormats.size();
}

bool MikanUsbVideoDevice::getVideoModeProperties(size_t index, UsbVideoModeProperties& outProperties) const
{
	if(index < m_deviceInfo.deviceAvailableFormats.size())
	{
		const WMFDeviceFormatInfo& formatInfo = m_deviceInfo.deviceAvailableFormats[index];
		outProperties.name = formatInfo.am_format_type_name.c_str();
		outProperties.width = formatInfo.width;
		outProperties.height = formatInfo.height;
		outProperties.stride = abs(formatInfo.default_stride);
		outProperties.frame_rate_numerator = formatInfo.frame_rate_numerator;
		outProperties.frame_rate_demonenator = formatInfo.frame_rate_denominator;

		return true;
	}

	return false;
}

int MikanUsbVideoDevice::getVideoModeIndex() const
{
	return m_currentVideoModeIndex;
}

const char* MikanUsbVideoDevice::getVideoModeName() const
{
	if (m_currentVideoModeIndex >= 0 && 
		m_currentVideoModeIndex < (int)m_deviceInfo.deviceAvailableFormats.size())
	{
		return m_deviceInfo.deviceAvailableFormats[m_currentVideoModeIndex].am_format_type_name.c_str();
	}

	return nullptr;
}

bool MikanUsbVideoDevice::setVideoModeByName(const char* szVideoModeName)
{
	if (szVideoModeName != nullptr && !m_deviceInfo.deviceAvailableFormats.empty())
	{
		const int desiredFormatIndex = m_deviceInfo.findDeviceFormatByName(szVideoModeName);

		return setVideoModeByIndex(desiredFormatIndex);
	}

	return false;
}

bool MikanUsbVideoDevice::setVideoModeByIndex(size_t desiredFormatIndex)
{
	if (m_currentVideoModeIndex != desiredFormatIndex &&
		desiredFormatIndex >= 0 &&
		desiredFormatIndex < (int)m_deviceInfo.deviceAvailableFormats.size())
	{
		m_currentVideoModeIndex = (int)desiredFormatIndex;
		if (!open())
		{
			m_currentVideoModeIndex = -1;
		}
		notifyVideoModePropertiesChanged();
		return true;
	}

	return false;
}

// -- Camera Settings
bool MikanUsbVideoDevice::getCameraSettingConstraint(
	const eUsbCameraSettingType property_type,
	UsbCameraSettingConstraint& outConstraint) const
{
	bool bSuccess = false;

	switch (property_type)
	{
	case eUsbCameraSettingType::Brightness:
		bSuccess = getProcAmpRange(VideoProcAmp_Brightness, outConstraint);
		break;
	case eUsbCameraSettingType::Contrast:
		bSuccess = getProcAmpRange(VideoProcAmp_Contrast, outConstraint);
		break;
	case eUsbCameraSettingType::Hue:
		bSuccess = getProcAmpRange(VideoProcAmp_Hue, outConstraint);
		break;
	case eUsbCameraSettingType::Saturation:
		bSuccess = getProcAmpRange(VideoProcAmp_Saturation, outConstraint);
		break;
	case eUsbCameraSettingType::Sharpness:
		bSuccess = getProcAmpRange(VideoProcAmp_Sharpness, outConstraint);
		break;
	case eUsbCameraSettingType::Gamma:
		bSuccess = getProcAmpRange(VideoProcAmp_Gamma, outConstraint);
		break;
	case eUsbCameraSettingType::WhiteBalance:
		bSuccess = getProcAmpRange(VideoProcAmp_WhiteBalance, outConstraint);
		break;
	case eUsbCameraSettingType::RedBalance:
	case eUsbCameraSettingType::GreenBalance:
	case eUsbCameraSettingType::BlueBalance:
		memset(&outConstraint, 0, sizeof(UsbCameraSettingConstraint));
		bSuccess = true;
		break;
	case eUsbCameraSettingType::Gain:
		bSuccess = getProcAmpRange(VideoProcAmp_Gain, outConstraint);
		break;
	case eUsbCameraSettingType::Pan:
		bSuccess = getCameraControlRange(CameraControl_Pan, outConstraint);
		break;
	case eUsbCameraSettingType::Tilt:
		bSuccess = getCameraControlRange(CameraControl_Tilt, outConstraint);
		break;
	case eUsbCameraSettingType::Roll:
		bSuccess = getCameraControlRange(CameraControl_Roll, outConstraint);
		break;
	case eUsbCameraSettingType::Zoom:
		bSuccess = getCameraControlRange(CameraControl_Zoom, outConstraint);
		break;
	case eUsbCameraSettingType::Exposure:
		bSuccess = getCameraControlRange(CameraControl_Exposure, outConstraint);
		break;
	case eUsbCameraSettingType::Iris:
		bSuccess = getCameraControlRange(CameraControl_Iris, outConstraint);
		break;
	case eUsbCameraSettingType::Focus:
		bSuccess = getCameraControlRange(CameraControl_Focus, outConstraint);
		break;
	}

	return bSuccess;
}

void MikanUsbVideoDevice::setCameraSetting(const eUsbCameraSettingType property_type, int desired_value)
{
	switch (property_type)
	{
	case eUsbCameraSettingType::Brightness:
		setProcAmpProperty(VideoProcAmp_Brightness, desired_value, false);
		break;
	case eUsbCameraSettingType::Contrast:
		setProcAmpProperty(VideoProcAmp_Contrast, desired_value, false);
		break;
	case eUsbCameraSettingType::Hue:
		setProcAmpProperty(VideoProcAmp_Hue, desired_value, false);
		break;
	case eUsbCameraSettingType::Saturation:
		setProcAmpProperty(VideoProcAmp_Saturation, desired_value, false);
		break;
	case eUsbCameraSettingType::Sharpness:
		setProcAmpProperty(VideoProcAmp_Sharpness, desired_value, false);
		break;
	case eUsbCameraSettingType::Gamma:
		setProcAmpProperty(VideoProcAmp_Gamma, desired_value, false);
		break;
	case eUsbCameraSettingType::WhiteBalance:
		setProcAmpProperty(VideoProcAmp_WhiteBalance, desired_value, false);
		break;
	case eUsbCameraSettingType::RedBalance:
	case eUsbCameraSettingType::GreenBalance:
	case eUsbCameraSettingType::BlueBalance:
		// not supported
		break;
	case eUsbCameraSettingType::Gain:
		setProcAmpProperty(VideoProcAmp_Gain, desired_value, false);
		break;
	case eUsbCameraSettingType::Pan:
		setCameraControlProperty(CameraControl_Pan, desired_value, false);
		break;
	case eUsbCameraSettingType::Tilt:
		setCameraControlProperty(CameraControl_Tilt, desired_value, false);
		break;
	case eUsbCameraSettingType::Roll:
		setCameraControlProperty(CameraControl_Roll, desired_value, false);
		break;
	case eUsbCameraSettingType::Zoom:
		setCameraControlProperty(CameraControl_Zoom, desired_value, false);
		break;
	case eUsbCameraSettingType::Exposure:
		setCameraControlProperty(CameraControl_Exposure, desired_value, false);
		break;
	case eUsbCameraSettingType::Iris:
		setCameraControlProperty(CameraControl_Iris, desired_value, false);
		break;
	case eUsbCameraSettingType::Focus:
		setCameraControlProperty(CameraControl_Focus, desired_value, false);
		break;
	}
}

int MikanUsbVideoDevice::getCameraSetting(const eUsbCameraSettingType property_type) const
{
	int value = 0;

	switch (property_type)
	{
	case eUsbCameraSettingType::Brightness:
		value = getProcAmpProperty(VideoProcAmp_Brightness);
		break;
	case eUsbCameraSettingType::Contrast:
		value = getProcAmpProperty(VideoProcAmp_Contrast);
		break;
	case eUsbCameraSettingType::Hue:
		value = getProcAmpProperty(VideoProcAmp_Hue);
		break;
	case eUsbCameraSettingType::Saturation:
		value = getProcAmpProperty(VideoProcAmp_Saturation);
		break;
	case eUsbCameraSettingType::Sharpness:
		value = getProcAmpProperty(VideoProcAmp_Sharpness);
		break;
	case eUsbCameraSettingType::Gamma:
		value = getProcAmpProperty(VideoProcAmp_Gamma);
		break;
	case eUsbCameraSettingType::WhiteBalance:
		value = getProcAmpProperty(VideoProcAmp_WhiteBalance);
		break;
	case eUsbCameraSettingType::RedBalance:
	case eUsbCameraSettingType::GreenBalance:
	case eUsbCameraSettingType::BlueBalance:
		// not supported
		break;
	case eUsbCameraSettingType::Gain:
		value = getProcAmpProperty(VideoProcAmp_Gain);
		break;
	case eUsbCameraSettingType::Pan:
		value = getCameraControlProperty(CameraControl_Pan);
		break;
	case eUsbCameraSettingType::Tilt:
		value = getCameraControlProperty(CameraControl_Tilt);
		break;
	case eUsbCameraSettingType::Roll:
		value = getCameraControlProperty(CameraControl_Roll);
		break;
	case eUsbCameraSettingType::Zoom:
		value = getCameraControlProperty(CameraControl_Zoom);
		break;
	case eUsbCameraSettingType::Exposure:
		value = getCameraControlProperty(CameraControl_Exposure);
		break;
	case eUsbCameraSettingType::Iris:
		value = getCameraControlProperty(CameraControl_Iris);
		break;
	case eUsbCameraSettingType::Focus:
		value = getCameraControlProperty(CameraControl_Focus);
		break;
	}

	return value;
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

bool MikanUsbVideoDevice::open()
{
	return false;
}

void MikanUsbVideoDevice::close()
{

}

/*
	See https://msdn.microsoft.com/en-us/library/windows/desktop/dd407328(v=vs.85).aspx
	VideoProcAmp_Brightness		[-10k, 10k]
	VideoProcAmp_Contrast			[0, 10k]
	VideoProcAmp_Hue				[-180k, 180k]
	VideoProcAmp_Saturation		[0, 10k]
	VideoProcAmp_Sharpness		[0, 100]
	VideoProcAmp_Gamma			[1, 500]
	VideoProcAmp_ColorEnable		0=off, 1=on
	VideoProcAmp_WhiteBalance		device dependent
	VideoProcAmp_BacklightCompensation		0=off, 1=on
	VideoProcAmp_Gain				device dependent
*/
bool MikanUsbVideoDevice::setProcAmpProperty(VideoProcAmpProperty propId, long value, bool bAuto)
{
	bool bSuccess = false;

	IAMVideoProcAmp* pProcAmp = NULL;
	HRESULT hr = m_mediaSource->QueryInterface(IID_PPV_ARGS(&pProcAmp));

	if (SUCCEEDED(hr))
	{
		hr = pProcAmp->Set(propId, value, bAuto ? VideoProcAmp_Flags_Auto : VideoProcAmp_Flags_Manual);
		pProcAmp->Release();
	}

	return SUCCEEDED(hr);
}

long MikanUsbVideoDevice::getProcAmpProperty(VideoProcAmpProperty propId, bool* bIsAuto) const
{
	long intValue = 0;
	IAMVideoProcAmp* pProcAmp = NULL;
	HRESULT hr = m_mediaSource->QueryInterface(IID_PPV_ARGS(&pProcAmp));

	if (SUCCEEDED(hr))
	{
		long flags;
		hr = pProcAmp->Get(propId, &intValue, &flags);

		if (bIsAuto != nullptr)
		{
			*bIsAuto = flags == VideoProcAmp_Flags_Auto;
		}

		pProcAmp->Release();
	}

	return intValue;
}

bool MikanUsbVideoDevice::getProcAmpRange(VideoProcAmpProperty propId, UsbCameraSettingConstraint& constraint) const
{
	IAMVideoProcAmp* pProcAmp = NULL;
	HRESULT hr = m_mediaSource->QueryInterface(IID_PPV_ARGS(&pProcAmp));

	memset(&constraint, 0, sizeof(UsbCameraSettingConstraint));

	if (SUCCEEDED(hr))
	{
		long minValue, maxValue, stepSize, defaultValue, flags;
		hr = pProcAmp->GetRange(propId, &minValue, &maxValue, &stepSize, &defaultValue, &flags);

		if (SUCCEEDED(hr))
		{
			constraint.default_value = defaultValue;
			constraint.min_value = minValue;
			constraint.max_value = maxValue;
			constraint.stepping_delta = stepSize;
			constraint.is_supported = true;
			constraint.is_automatic = flags == VideoProcAmp_Flags_Auto;
		}

		pProcAmp->Release();
	}

	return SUCCEEDED(hr);
}

bool MikanUsbVideoDevice::setCameraControlProperty(CameraControlProperty propId, long value, bool bAuto)
{
	bool bSuccess = false;

	IAMCameraControl* pProcControl = NULL;
	HRESULT hr = m_mediaSource->QueryInterface(IID_PPV_ARGS(&pProcControl));

	if (SUCCEEDED(hr))
	{
		hr = pProcControl->Set(propId, value, bAuto ? CameraControl_Flags_Auto : CameraControl_Flags_Manual);

		pProcControl->Release();
	}

	return SUCCEEDED(hr);
}

long MikanUsbVideoDevice::getCameraControlProperty(CameraControlProperty propId, bool* bIsAuto) const
{
	long intValue = 0;
	IAMCameraControl* pCameraControl = NULL;
	HRESULT hr = m_mediaSource->QueryInterface(IID_PPV_ARGS(&pCameraControl));

	if (SUCCEEDED(hr))
	{
		long flags;
		hr = pCameraControl->Get(propId, &intValue, &flags);

		if (bIsAuto != nullptr)
		{
			*bIsAuto = flags == CameraControl_Flags_Auto;
		}

		pCameraControl->Release();
	}

	return intValue;
}

bool MikanUsbVideoDevice::getCameraControlRange(
	CameraControlProperty propId, UsbCameraSettingConstraint& constraint) const
{
	double unitValue = 0;
	IAMCameraControl* pCameraControl = NULL;
	HRESULT hr = m_mediaSource->QueryInterface(IID_PPV_ARGS(&pCameraControl));

	memset(&constraint, 0, sizeof(UsbCameraSettingConstraint));

	if (SUCCEEDED(hr))
	{
		long minValue, maxValue, stepSize, defaultValue, flags;
		hr = pCameraControl->GetRange(propId, &minValue, &maxValue, &stepSize, &defaultValue, &flags);

		if (SUCCEEDED(hr))
		{
			constraint.default_value = defaultValue;
			constraint.min_value = minValue;
			constraint.max_value = maxValue;
			constraint.stepping_delta = stepSize;
			constraint.is_supported = true;
			constraint.is_automatic = flags == VideoProcAmp_Flags_Auto;
		}

		pCameraControl->Release();
	}

	return SUCCEEDED(hr);
}

void MikanUsbVideoDevice::notifyVideoDeviceDisconnected()
{
	for (auto listener : m_listeners)
	{
		listener->notifyVideoDeviceDisconnected(this);
	}
}

void MikanUsbVideoDevice::notifyVideoModePropertiesChanged()
{
	for (auto listener : m_listeners)
	{
		listener->notifyVideoModePropertiesChanged(this);
	}
}

void MikanUsbVideoDevice::notifyVideoFrameReceived(const UsbVideoFrameBuffer& bufferInfo)
{
	for (auto listener : m_listeners)
	{
		listener->notifyVideoFrameReceived(bufferInfo);
	}
}