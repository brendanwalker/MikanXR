#include "MikanWMFVideoDevice.h"
#include "MikanWMFVideoDeviceManager.h"

#include "Logger.h"
#include "MemoryUtils.h"

// -- MikanUsbVideoDevice -----
MikanWMFVideoDevice::MikanWMFVideoDevice(
	MikanWMFVideoDeviceManager* ownerDeviceManager,
	const WMFDeviceInfo& deviceInfo)
	: m_ownerDeviceManager(ownerDeviceManager)
	, m_deviceInfo(deviceInfo)
{
}

MikanWMFVideoDevice::~MikanWMFVideoDevice()
{
	close();
}

// -- Device Listener
void MikanWMFVideoDevice::addListener(IUsbVideoDeviceListener* listener)
{
	m_listeners.insert(listener);
}

void MikanWMFVideoDevice::removeListener(IUsbVideoDeviceListener* listener)
{
	m_listeners.erase(listener);
}

// -- Device Properties
const char* MikanWMFVideoDevice::getDevicePath() const
{
	return m_deviceInfo.deviceSymbolicLink.c_str();
}

const char* MikanWMFVideoDevice::getFriendlyName() const
{
	return m_deviceInfo.deviceFriendlyName.c_str();
}

// -- Device Activation
bool MikanWMFVideoDevice::getIsOpen() const
{
	return m_mediaSource != nullptr;
}

bool MikanWMFVideoDevice::open()
{
	HRESULT hr;

	// Early ourt if already open
	if (getIsOpen())
	{
		return true;
	}

	if (m_currentVideoModeIndex >= 0 && m_currentVideoModeIndex < m_deviceInfo.deviceAvailableFormats.size())
	{
		IMFAttributes* pAttributes = NULL;
		IMFActivate* vd_pActivate = NULL;

		hr = MFCreateAttributes(&pAttributes, 1);

		if (SUCCEEDED(hr))
		{
			hr = pAttributes->SetGUID(
				MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
				MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
		}

		IMFActivate* deviceActivationInterface = nullptr;
		if (SUCCEEDED(hr))
		{
			IMFActivate** ppDevices = nullptr;
			UINT32 wmfDeviceCount;
			hr = MFEnumDeviceSources(pAttributes, &ppDevices, &wmfDeviceCount);

			if (m_deviceInfo.wmfDeviceIndex >= 0 && m_deviceInfo.wmfDeviceIndex < (int)wmfDeviceCount)
			{
				deviceActivationInterface = ppDevices[m_deviceInfo.wmfDeviceIndex];
				deviceActivationInterface->AddRef();
			}

			for (UINT32 i = 0; i < wmfDeviceCount; i++)
			{
				MemoryUtils::safeRelease(&ppDevices[i]);
			}

			MemoryUtils::safeReleaseAllCount(ppDevices);
		}

		if (SUCCEEDED(hr))
		{
			hr = deviceActivationInterface->ActivateObject(
				__uuidof(IMFMediaSource),
				(void**)&m_mediaSource);
		}

		IMFPresentationDescriptor* pPD = nullptr;
		if (SUCCEEDED(hr))
			hr = m_mediaSource->CreatePresentationDescriptor(&pPD);

		BOOL fSelected;
		IMFStreamDescriptor* pSD = nullptr;
		if (SUCCEEDED(hr))
			hr = pPD->GetStreamDescriptorByIndex(0, &fSelected, &pSD);

		IMFMediaTypeHandler* pHandler = nullptr;
		if (SUCCEEDED(hr))
			hr = pSD->GetMediaTypeHandler(&pHandler);

		DWORD cTypes = 0;
		if (SUCCEEDED(hr))
			hr = pHandler->GetMediaTypeCount(&cTypes);

		IMFMediaType* pType = nullptr;
		if (SUCCEEDED(hr))
			hr = pHandler->GetMediaTypeByIndex((DWORD)m_currentVideoModeIndex, &pType);

		if (SUCCEEDED(hr))
			hr = pHandler->SetCurrentMediaType(pType);

		if (SUCCEEDED(hr))
		{
			const WMFDeviceFormatInfo& deviceFormat =
				m_deviceInfo.deviceAvailableFormats[m_currentVideoModeIndex];

			m_videoFrameProcessor =
				new WMFVideoFrameProcessor(
					m_deviceInfo.wmfDeviceIndex, deviceFormat, this);
			hr = m_videoFrameProcessor->init(m_mediaSource);
		}

		if (SUCCEEDED(hr))
		{
			// Update the property constraints for the current video format
			for (int prop_index = 0; prop_index < (int)eVideoSettingType::COUNT; ++prop_index)
			{
				getVideoSettingConstraint(
					(eVideoSettingType)prop_index,
					m_videoPropertyConstraints[prop_index]);
			}
		}

		MemoryUtils::safeReleaseAllCount(&pPD);
		MemoryUtils::safeRelease(&pSD);
		MemoryUtils::safeRelease(&pHandler);
		MemoryUtils::safeRelease(&pType);
		MemoryUtils::safeReleaseAllCount(&deviceActivationInterface);
		MemoryUtils::safeReleaseAllCount(&pAttributes);

		if (!SUCCEEDED(hr))
		{
			close();
		}
	}
	else
	{
		hr = E_INVALIDARG;
	}

	return SUCCEEDED(hr);

}

void MikanWMFVideoDevice::close()
{
	if (m_videoFrameProcessor != nullptr)
	{
		delete m_videoFrameProcessor;
		m_videoFrameProcessor = nullptr;
	}

	if (m_mediaSource != nullptr)
	{
		m_mediaSource->Stop();
		MemoryUtils::safeRelease(&m_mediaSource);
	}
}

// -- Video Mode
size_t MikanWMFVideoDevice::getAvailableVideoModesCount() const
{
	return m_deviceInfo.deviceAvailableFormats.size();
}

bool MikanWMFVideoDevice::getVideoModeProperties(size_t index, UsbVideoModeProperties& outProperties) const
{
	if(index < m_deviceInfo.deviceAvailableFormats.size())
	{
		const WMFDeviceFormatInfo& formatInfo = m_deviceInfo.deviceAvailableFormats[index];
		outProperties.index = index;
		outProperties.name = formatInfo.format_friendly_name.c_str();
		outProperties.width = formatInfo.width;
		outProperties.height = formatInfo.height;
		outProperties.stride = abs(formatInfo.default_stride);
		outProperties.frame_rate_numerator = formatInfo.frame_rate_numerator;
		outProperties.frame_rate_demonenator = formatInfo.frame_rate_denominator;

		return true;
	}

	return false;
}

int MikanWMFVideoDevice::getVideoModeIndex() const
{
	return m_currentVideoModeIndex;
}

const char* MikanWMFVideoDevice::getVideoModeName() const
{
	if (m_currentVideoModeIndex >= 0 && 
		m_currentVideoModeIndex < (int)m_deviceInfo.deviceAvailableFormats.size())
	{
		return m_deviceInfo.deviceAvailableFormats[m_currentVideoModeIndex].format_friendly_name.c_str();
	}

	return nullptr;
}

bool MikanWMFVideoDevice::setVideoModeByName(const char* szVideoModeName)
{
	if (szVideoModeName != nullptr && !m_deviceInfo.deviceAvailableFormats.empty())
	{
		const int desiredFormatIndex = m_deviceInfo.findDeviceFormatByName(szVideoModeName);

		return setVideoModeByIndex(desiredFormatIndex);
	}

	return false;
}

bool MikanWMFVideoDevice::setVideoModeByIndex(size_t desiredFormatIndex)
{
	if (m_currentVideoModeIndex != desiredFormatIndex &&
		desiredFormatIndex >= 0 &&
		desiredFormatIndex < (int)m_deviceInfo.deviceAvailableFormats.size())
	{
		m_currentVideoModeIndex = (int)desiredFormatIndex;

		// Close the device if open
		if (getIsOpen())
		{
			close();

			// Re-open with the new format
			if (!open())
			{
				// Failed to open with the new format, reset to invalid
				m_currentVideoModeIndex = -1;
			}
		}

		notifyVideoModePropertiesChanged();
		return true;
	}

	return false;
}

// -- Camera Settings
bool MikanWMFVideoDevice::isVideoSettingSupported(const eVideoSettingType property_type) const
{
	VideoSettingConstraint constraint;

	return getVideoSettingConstraint(property_type, constraint);
}

bool MikanWMFVideoDevice::getVideoSettingConstraint(
	const eVideoSettingType property_type,
	VideoSettingConstraint& outConstraint) const
{
	if (!getIsOpen())
	{
		MIKAN_LOG_ERROR("MikanWMFVideoDevice::getVideoSettingConstraint") 
			<< "Unable to get video setting constraint: Device not open: " << m_deviceInfo.deviceFriendlyName;
		return false;
	}

	bool bSuccess = false;
	switch (property_type)
	{
	case eVideoSettingType::Brightness:
		bSuccess = getProcAmpRange(VideoProcAmp_Brightness, outConstraint);
		break;
	case eVideoSettingType::Contrast:
		bSuccess = getProcAmpRange(VideoProcAmp_Contrast, outConstraint);
		break;
	case eVideoSettingType::Hue:
		bSuccess = getProcAmpRange(VideoProcAmp_Hue, outConstraint);
		break;
	case eVideoSettingType::Saturation:
		bSuccess = getProcAmpRange(VideoProcAmp_Saturation, outConstraint);
		break;
	case eVideoSettingType::Sharpness:
		bSuccess = getProcAmpRange(VideoProcAmp_Sharpness, outConstraint);
		break;
	case eVideoSettingType::Gamma:
		bSuccess = getProcAmpRange(VideoProcAmp_Gamma, outConstraint);
		break;
	case eVideoSettingType::WhiteBalance:
		bSuccess = getProcAmpRange(VideoProcAmp_WhiteBalance, outConstraint);
		break;
	case eVideoSettingType::RedBalance:
	case eVideoSettingType::GreenBalance:
	case eVideoSettingType::BlueBalance:
		memset(&outConstraint, 0, sizeof(VideoSettingConstraint));
		bSuccess = false;
		break;
	case eVideoSettingType::Gain:
		bSuccess = getProcAmpRange(VideoProcAmp_Gain, outConstraint);
		break;
	case eVideoSettingType::Pan:
		bSuccess = getCameraControlRange(CameraControl_Pan, outConstraint);
		break;
	case eVideoSettingType::Tilt:
		bSuccess = getCameraControlRange(CameraControl_Tilt, outConstraint);
		break;
	case eVideoSettingType::Roll:
		bSuccess = getCameraControlRange(CameraControl_Roll, outConstraint);
		break;
	case eVideoSettingType::Zoom:
		bSuccess = getCameraControlRange(CameraControl_Zoom, outConstraint);
		break;
	case eVideoSettingType::Exposure:
		bSuccess = getCameraControlRange(CameraControl_Exposure, outConstraint);
		break;
	case eVideoSettingType::Iris:
		bSuccess = getCameraControlRange(CameraControl_Iris, outConstraint);
		break;
	case eVideoSettingType::Focus:
		bSuccess = getCameraControlRange(CameraControl_Focus, outConstraint);
		break;
	}

	if (bSuccess)
	{
		bSuccess = outConstraint.max_value > outConstraint.min_value;
	}

	return bSuccess;
}

void MikanWMFVideoDevice::setVideoSetting(const eVideoSettingType property_type, int desired_value)
{
	if (!getIsOpen())
	{
		MIKAN_LOG_ERROR("MikanWMFVideoDevice::setVideoSetting")
			<< "Unable to set video setting: Device not open: " << m_deviceInfo.deviceFriendlyName;

		return;
	}

	switch (property_type)
	{
	case eVideoSettingType::Brightness:
		setProcAmpProperty(VideoProcAmp_Brightness, desired_value, false);
		break;
	case eVideoSettingType::Contrast:
		setProcAmpProperty(VideoProcAmp_Contrast, desired_value, false);
		break;
	case eVideoSettingType::Hue:
		setProcAmpProperty(VideoProcAmp_Hue, desired_value, false);
		break;
	case eVideoSettingType::Saturation:
		setProcAmpProperty(VideoProcAmp_Saturation, desired_value, false);
		break;
	case eVideoSettingType::Sharpness:
		setProcAmpProperty(VideoProcAmp_Sharpness, desired_value, false);
		break;
	case eVideoSettingType::Gamma:
		setProcAmpProperty(VideoProcAmp_Gamma, desired_value, false);
		break;
	case eVideoSettingType::WhiteBalance:
		setProcAmpProperty(VideoProcAmp_WhiteBalance, desired_value, false);
		break;
	case eVideoSettingType::RedBalance:
	case eVideoSettingType::GreenBalance:
	case eVideoSettingType::BlueBalance:
		// not supported
		break;
	case eVideoSettingType::Gain:
		setProcAmpProperty(VideoProcAmp_Gain, desired_value, false);
		break;
	case eVideoSettingType::Pan:
		setCameraControlProperty(CameraControl_Pan, desired_value, false);
		break;
	case eVideoSettingType::Tilt:
		setCameraControlProperty(CameraControl_Tilt, desired_value, false);
		break;
	case eVideoSettingType::Roll:
		setCameraControlProperty(CameraControl_Roll, desired_value, false);
		break;
	case eVideoSettingType::Zoom:
		setCameraControlProperty(CameraControl_Zoom, desired_value, false);
		break;
	case eVideoSettingType::Exposure:
		setCameraControlProperty(CameraControl_Exposure, desired_value, false);
		break;
	case eVideoSettingType::Iris:
		setCameraControlProperty(CameraControl_Iris, desired_value, false);
		break;
	case eVideoSettingType::Focus:
		setCameraControlProperty(CameraControl_Focus, desired_value, false);
		break;
	}
}

int MikanWMFVideoDevice::getVideoSetting(const eVideoSettingType property_type) const
{
	if (!getIsOpen())
	{
		MIKAN_LOG_ERROR("MikanWMFVideoDevice::getVideoSetting")
			<< "Unable to get video setting: Device not open: " << m_deviceInfo.deviceFriendlyName;
		return 0;
	}

	int value = 0;
	switch (property_type)
	{
	case eVideoSettingType::Brightness:
		value = getProcAmpProperty(VideoProcAmp_Brightness);
		break;
	case eVideoSettingType::Contrast:
		value = getProcAmpProperty(VideoProcAmp_Contrast);
		break;
	case eVideoSettingType::Hue:
		value = getProcAmpProperty(VideoProcAmp_Hue);
		break;
	case eVideoSettingType::Saturation:
		value = getProcAmpProperty(VideoProcAmp_Saturation);
		break;
	case eVideoSettingType::Sharpness:
		value = getProcAmpProperty(VideoProcAmp_Sharpness);
		break;
	case eVideoSettingType::Gamma:
		value = getProcAmpProperty(VideoProcAmp_Gamma);
		break;
	case eVideoSettingType::WhiteBalance:
		value = getProcAmpProperty(VideoProcAmp_WhiteBalance);
		break;
	case eVideoSettingType::RedBalance:
	case eVideoSettingType::GreenBalance:
	case eVideoSettingType::BlueBalance:
		// not supported
		break;
	case eVideoSettingType::Gain:
		value = getProcAmpProperty(VideoProcAmp_Gain);
		break;
	case eVideoSettingType::Pan:
		value = getCameraControlProperty(CameraControl_Pan);
		break;
	case eVideoSettingType::Tilt:
		value = getCameraControlProperty(CameraControl_Tilt);
		break;
	case eVideoSettingType::Roll:
		value = getCameraControlProperty(CameraControl_Roll);
		break;
	case eVideoSettingType::Zoom:
		value = getCameraControlProperty(CameraControl_Zoom);
		break;
	case eVideoSettingType::Exposure:
		value = getCameraControlProperty(CameraControl_Exposure);
		break;
	case eVideoSettingType::Iris:
		value = getCameraControlProperty(CameraControl_Iris);
		break;
	case eVideoSettingType::Focus:
		value = getCameraControlProperty(CameraControl_Focus);
		break;
	}

	return value;
}

// -- Video Streaming
eVideoStreamingStatus MikanWMFVideoDevice::startVideoStream()
{
	if (getIsOpen())
	{
		eVideoStreamingStatus status= getVideoStreamingStatus();

		if (status == eVideoStreamingStatus::stopped ||
			status == eVideoStreamingStatus::failed)
		{
			status= 
				SUCCEEDED(m_videoFrameProcessor->startVideoFrameThread())
				? eVideoStreamingStatus::started
				: eVideoStreamingStatus::failed;
		}

		return status;
	}

	return eVideoStreamingStatus::failed;
}

eVideoStreamingStatus MikanWMFVideoDevice::getVideoStreamingStatus() const
{
	if (getIsOpen())
	{
		return m_videoFrameProcessor->getIsRunning() 
			? eVideoStreamingStatus::started
			: eVideoStreamingStatus::stopped;
	}

	return eVideoStreamingStatus::failed;
}

void MikanWMFVideoDevice::stopVideoStream()
{
	if (getIsOpen())
	{
		m_videoFrameProcessor->stopVideoFrameThread();
	}
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
bool MikanWMFVideoDevice::setProcAmpProperty(VideoProcAmpProperty propId, long value, bool bAuto)
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

long MikanWMFVideoDevice::getProcAmpProperty(VideoProcAmpProperty propId, bool* bIsAuto) const
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

bool MikanWMFVideoDevice::getProcAmpRange(VideoProcAmpProperty propId, VideoSettingConstraint& constraint) const
{
	IAMVideoProcAmp* pProcAmp = NULL;
	HRESULT hr = m_mediaSource->QueryInterface(IID_PPV_ARGS(&pProcAmp));

	memset(&constraint, 0, sizeof(VideoSettingConstraint));

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
			constraint.is_automatic = flags == VideoProcAmp_Flags_Auto;
		}

		pProcAmp->Release();
	}

	return SUCCEEDED(hr);
}

bool MikanWMFVideoDevice::setCameraControlProperty(CameraControlProperty propId, long value, bool bAuto)
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

long MikanWMFVideoDevice::getCameraControlProperty(CameraControlProperty propId, bool* bIsAuto) const
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

bool MikanWMFVideoDevice::getCameraControlRange(
	CameraControlProperty propId, VideoSettingConstraint& constraint) const
{
	double unitValue = 0;
	IAMCameraControl* pCameraControl = NULL;
	HRESULT hr = m_mediaSource->QueryInterface(IID_PPV_ARGS(&pCameraControl));

	memset(&constraint, 0, sizeof(VideoSettingConstraint));

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
			constraint.is_automatic = flags == VideoProcAmp_Flags_Auto;
		}

		pCameraControl->Release();
	}

	return SUCCEEDED(hr);
}

void MikanWMFVideoDevice::notifyVideoDeviceDisconnected()
{
	close();

	for (auto listener : m_listeners)
	{
		listener->notifyVideoDeviceDisconnected(this);
	}
}

void MikanWMFVideoDevice::notifyVideoModePropertiesChanged()
{
	for (auto listener : m_listeners)
	{
		listener->notifyVideoModePropertiesChanged(this);
	}
}

void MikanWMFVideoDevice::notifyVideoFrameReceived(const UsbVideoFrameBuffer& bufferInfo)
{
	for (auto listener : m_listeners)
	{
		listener->notifyVideoFrameReceived(bufferInfo);
	}
}


// -- WMF Video Frame Processor -----
WMFVideoFrameProcessor::WMFVideoFrameProcessor(
	int deviceIndex,
	const WMFDeviceFormatInfo& deviceFormat,
	MikanWMFVideoDevice* listener)
	: WorkerThread("WMFVideoFrameProcessor")
	, m_deviceIndex(deviceIndex)
	, m_deviceFormat(deviceFormat)
	, m_videoSourceListener(listener)
	, m_referenceCount(1)
	, m_pSession(nullptr)
	, m_pTopology(nullptr)
	, m_bIsRunning(false)
	, m_sampleIndex(0)
{
}

WMFVideoFrameProcessor::~WMFVideoFrameProcessor(void)
{
	dispose();
}

HRESULT WMFVideoFrameProcessor::init(IMFMediaSource* pSource)
{
	// Clean up previous session, if any.
	if (m_pSession)
	{
		m_pSession->Shutdown();
	}
	MemoryUtils::safeReleaseAllCount(&m_pSession);
	MemoryUtils::safeReleaseAllCount(&m_pTopology);

	// Configure the media type that the video frame processor will receive.
	// Setting the major and subtype is usually enough for the topology loader
	// to resolve the topology.
	IMFMediaType* pType = nullptr;
	HRESULT hr = MFCreateMediaType(&pType);

	if (SUCCEEDED(hr))
		hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);

	if (SUCCEEDED(hr))
		hr = pType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB24);

	// Create the sample grabber sink.
	IMFActivate* pSinkActivate = nullptr;
	if (SUCCEEDED(hr))
		hr = MFCreateSampleGrabberSinkActivate(pType, this, &pSinkActivate);

	// To run as fast as possible, set this attribute (requires Windows 7):
	if (SUCCEEDED(hr))
		hr = pSinkActivate->SetUINT32(MF_SAMPLEGRABBERSINK_IGNORE_CLOCK, TRUE);

	// Create the Media Session.
	if (SUCCEEDED(hr))
		hr = MFCreateMediaSession(NULL, &m_pSession);

	// Create the topology.
	if (SUCCEEDED(hr))
		hr = CreateTopology(pSource, pSinkActivate, &m_pTopology);


	// Clean up.
	if (FAILED(hr))
	{
		if (m_pSession)
		{
			m_pSession->Shutdown();
		}

		MemoryUtils::safeRelease(&m_pSession);
		MemoryUtils::safeRelease(&m_pTopology);
	}

	MemoryUtils::safeRelease(&pSinkActivate);
	MemoryUtils::safeRelease(&pType);

	return hr;
}

void WMFVideoFrameProcessor::dispose()
{
	stopVideoFrameThread();

	if (m_pSession)
	{
		m_pSession->Shutdown();
	}

	MemoryUtils::safeReleaseAllCount(&m_pSession);
	MemoryUtils::safeReleaseAllCount(&m_pTopology);

	MIKAN_LOG_INFO("WMFVideoFrameProcessor::dispose") << "Disposing video frame grabber for device: " << m_deviceIndex;
}

HRESULT WMFVideoFrameProcessor::startVideoFrameThread(void)
{
	HRESULT hr = m_pSession->SetTopology(0, m_pTopology);

	m_sampleIndex = 0;

	if (SUCCEEDED(hr))
	{
		PROPVARIANT var;
		PropVariantInit(&var);

		hr = m_pSession->Start(&GUID_NULL, &var);
	}

	if (SUCCEEDED(hr))
	{
		m_bIsRunning = true;
		WorkerThread::startThread();
	}

	return SUCCEEDED(hr);
}

bool WMFVideoFrameProcessor::doWork()
{
	bool bKeepRunning = true;

	IMFMediaEvent* pEvent = nullptr;
	HRESULT hr = m_pSession->GetEvent(0, &pEvent);

	if (SUCCEEDED(hr))
	{
		HRESULT hrStatus;
		hr = pEvent->GetStatus(&hrStatus);

		if (!SUCCEEDED(hr) || !SUCCEEDED(hrStatus))
		{
			bKeepRunning = false;
			hr = E_FAIL;
		}

		MediaEventType met = MEUnknown;
		if (SUCCEEDED(hr))
		{
			hr = pEvent->GetType(&met);
		}

		if (SUCCEEDED(hr))
		{
			switch (met)
			{
			case MESessionEnded:
			{
				MIKAN_MT_LOG_INFO("WMFVideoFrameProcessor::doWork") << "MESessionEnded: " << m_deviceIndex;
				bKeepRunning = false;
			} break;

			case MESessionStopped:
			{
				MIKAN_MT_LOG_INFO("WMFVideoFrameProcessor::doWork") << "MESessionStopped: " << m_deviceIndex;
				bKeepRunning = false;
			} break;

			case MEVideoCaptureDeviceRemoved:
			{
				MIKAN_MT_LOG_INFO("WMFVideoFrameProcessor::doWork") << "MEVideoCaptureDeviceRemoved: " << m_deviceIndex;
				bKeepRunning = false;
			} break;
			}
		}
		else
		{
			bKeepRunning = false;
		}
	}
	else
	{
		bKeepRunning = false;
	}

	MemoryUtils::safeRelease(&pEvent);

	return bKeepRunning;
}

void WMFVideoFrameProcessor::stopVideoFrameThread()
{
	MIKAN_LOG_INFO("WMFVideoFrameProcessor::stop") << "Stopping video frame grabbing on device: " << m_deviceIndex;

	if (m_bIsRunning)
	{
		if (m_pSession != nullptr)
		{
			// This will send a MESessionStopped event to the worker thread
			m_pSession->Stop();
		}

		WorkerThread::stopThread();

		m_bIsRunning = false;
	}
}

HRESULT WMFVideoFrameProcessor::CreateTopology(
	IMFMediaSource* pSource, IMFActivate* pSinkActivate, IMFTopology** ppTopo)
{
	IMFTopology* pTopology = nullptr;
	HRESULT hr = MFCreateTopology(&pTopology);

	IMFPresentationDescriptor* pPD = nullptr;
	if (SUCCEEDED(hr))
		hr = pSource->CreatePresentationDescriptor(&pPD);

	DWORD cStreams = 0;
	if (SUCCEEDED(hr))
		hr = pPD->GetStreamDescriptorCount(&cStreams);

	if (SUCCEEDED(hr))
	{
		for (DWORD i = 0; i < cStreams; i++)
		{
			// Look for video streams and connect them to the sink
			BOOL fSelected = FALSE;
			GUID majorType;

			IMFStreamDescriptor* pSD = nullptr;
			hr = pPD->GetStreamDescriptorByIndex(i, &fSelected, &pSD);

			IMFMediaTypeHandler* pHandler = nullptr;
			if (SUCCEEDED(hr))
				hr = pSD->GetMediaTypeHandler(&pHandler);

			if (SUCCEEDED(hr))
				hr = pHandler->GetMajorType(&majorType);

			if (SUCCEEDED(hr))
			{
				if (majorType == MFMediaType_Video && fSelected)
				{
					IMFTopologyNode* pNode1 = nullptr;
					IMFTopologyNode* pNode2 = nullptr;

					hr = AddSourceNode(pTopology, pSource, pPD, pSD, &pNode1);

					if (SUCCEEDED(hr))
						hr = AddOutputNode(pTopology, pSinkActivate, 0, &pNode2);

					if (SUCCEEDED(hr))
						hr = pNode1->ConnectOutput(0, pNode2, 0);

					MemoryUtils::safeRelease(&pNode1);
					MemoryUtils::safeRelease(&pNode2);
					break;
				}
				else
				{
					hr = pPD->DeselectStream(i);
				}
			}

			MemoryUtils::safeRelease(&pSD);
			MemoryUtils::safeRelease(&pHandler);

			if (FAILED(hr))
				break;
		}
	}

	if (SUCCEEDED(hr))
	{
		*ppTopo = pTopology;
		(*ppTopo)->AddRef();
	}

	MemoryUtils::safeRelease(&pTopology);
	MemoryUtils::safeRelease(&pPD);

	return hr;
}

HRESULT WMFVideoFrameProcessor::AddSourceNode(
	IMFTopology* pTopology,
	IMFMediaSource* pSource,
	IMFPresentationDescriptor* pPD,
	IMFStreamDescriptor* pSD,
	IMFTopologyNode** ppNode)
{
	IMFTopologyNode* pNode = nullptr;
	HRESULT hr = MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &pNode);

	if (SUCCEEDED(hr))
		hr = pNode->SetUnknown(MF_TOPONODE_SOURCE, pSource);

	if (SUCCEEDED(hr))
		hr = pNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, pPD);

	if (SUCCEEDED(hr))
		hr = pNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD);

	if (SUCCEEDED(hr))
		hr = pTopology->AddNode(pNode);

	// Return the pointer to the caller.
	if (SUCCEEDED(hr))
	{
		*ppNode = pNode;
		(*ppNode)->AddRef();
	}

	MemoryUtils::safeRelease(&pNode);

	return hr;
}

HRESULT WMFVideoFrameProcessor::AddOutputNode(
	IMFTopology* pTopology,
	IMFActivate* pActivate,
	DWORD dwId,
	IMFTopologyNode** ppNode)
{
	IMFTopologyNode* pNode = NULL;

	HRESULT hr = MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &pNode);

	if (SUCCEEDED(hr))
		hr = pNode->SetObject(pActivate);

	if (SUCCEEDED(hr))
		hr = pNode->SetUINT32(MF_TOPONODE_STREAMID, dwId);

	if (SUCCEEDED(hr))
		hr = pNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, FALSE);

	if (SUCCEEDED(hr))
		hr = pTopology->AddNode(pNode);

	// Return the pointer to the caller.
	if (SUCCEEDED(hr))
	{
		*ppNode = pNode;
		(*ppNode)->AddRef();
	}

	MemoryUtils::safeRelease(&pNode);

	return hr;
}

STDMETHODIMP WMFVideoFrameProcessor::QueryInterface(REFIID riid, void** ppv)
{
	// Creation tab of shifting interfaces from start of this class
	static const QITAB qit[] =
	{
		QITABENT(WMFVideoFrameProcessor, IMFSampleGrabberSinkCallback),
		QITABENT(WMFVideoFrameProcessor, IMFClockStateSink),
		{ 0 }
	};
	return QISearch(this, qit, riid, ppv);
}

STDMETHODIMP_(ULONG) WMFVideoFrameProcessor::AddRef()
{
	return InterlockedIncrement(&m_referenceCount);
}

STDMETHODIMP_(ULONG) WMFVideoFrameProcessor::Release()
{
	ULONG cRef = InterlockedDecrement(&m_referenceCount);
	if (cRef == 0)
	{
		delete this;
	}
	return cRef;
}

STDMETHODIMP WMFVideoFrameProcessor::OnProcessSample(REFGUID guidMajorMediaType, DWORD dwSampleFlags,
	LONGLONG llSampleTime, LONGLONG llSampleDuration, const BYTE* pSampleBuffer,
	DWORD dwSampleSize)
{
	if (m_videoSourceListener)
	{
		UsbVideoFrameBuffer frameBuffer;
		frameBuffer.data = static_cast<const uint8_t*>(pSampleBuffer);
		frameBuffer.byte_count = static_cast<size_t>(dwSampleSize);

		m_videoSourceListener->notifyVideoFrameReceived(frameBuffer);
	}

	m_sampleIndex++;

	return S_OK;
}
