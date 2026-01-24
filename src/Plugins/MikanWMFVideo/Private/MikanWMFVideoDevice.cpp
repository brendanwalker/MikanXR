#include "MikanWMFVideoDevice.h"
#include "MikanWMFVideoDeviceManager.h"

#include "Logger.h"
#include "MemoryUtils.h"

#include <mfreadwrite.h>
#include <sstream>
#include <iomanip>

static std::string GetHresultMessage(HRESULT hr);

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

	// Early out if already open
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
		// Stop streaming and close device
		close();

		m_currentVideoModeIndex = (int)desiredFormatIndex;

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
		m_videoFrameProcessor->startVideoFrameStream();

		return getVideoStreamingStatus();
	}

	return eVideoStreamingStatus::failed;
}

eVideoStreamingStatus MikanWMFVideoDevice::getVideoStreamingStatus() const
{
	if (getIsOpen())
	{
		switch (m_videoFrameProcessor->getState())
		{
		case WMFVideoFrameProcessor::State::Stopped:
			return eVideoStreamingStatus::stopped;
		case WMFVideoFrameProcessor::State::Starting:
			return eVideoStreamingStatus::pendingStart;
		case WMFVideoFrameProcessor::State::Running:
			return eVideoStreamingStatus::started;
		case WMFVideoFrameProcessor::State::Stopping:
			return eVideoStreamingStatus::stopped;
		case WMFVideoFrameProcessor::State::Failed:
			return eVideoStreamingStatus::failed;
		}
	}

	return eVideoStreamingStatus::failed;
}

void MikanWMFVideoDevice::stopVideoStream()
{
	if (getIsOpen())
	{
		m_videoFrameProcessor->stopVideoFrameStream();
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
	: m_deviceIndex(deviceIndex)
	, m_deviceFormat(deviceFormat)
	, m_videoSourceListener(listener)
	, m_referenceCount(1)
	, m_pSourceReader(nullptr)
	, m_pDecoderTransform(nullptr)
	, m_pNativeInputType(nullptr)
	, m_bNeedsDecoder(false)
	, m_decoderOutputInfo({})
	, m_state(State::Stopped)
	, m_sampleIndex(0)
	, m_outputFormat(eUSBVideoFrameBufferFormat::USBVideo_UNKNOWN)
	, m_wmfOutputFormat(GUID_NULL)
{
}

WMFVideoFrameProcessor::~WMFVideoFrameProcessor(void)
{
	dispose();
}

HRESULT WMFVideoFrameProcessor::init(IMFMediaSource* pSource)
{
	// Clean up previous source reader, if any
	// This is critical when changing video modes to avoid stuck transforms
	if (m_pSourceReader)
	{
		// Stop any ongoing streaming first
		if (m_state == State::Running)
		{
			stopVideoFrameStream();
		}

		// Flush the source reader to clear any pending samples and reset transforms
		m_pSourceReader->Flush((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM);

		// Release the source reader completely
		MemoryUtils::safeReleaseAllCount(&m_pSourceReader);

		// Give MFTs time to clean up (workaround for AMD/NVIDIA MFT cleanup issues)
		Sleep(100);
	}

	// Create attributes for the source reader
	IMFAttributes* pAttributes = nullptr;
	HRESULT hr = MFCreateAttributes(&pAttributes, 8);

	// Enable async callbacks
	if (SUCCEEDED(hr))
		hr = pAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, this);

	// Check for H.264 decoders (we'll decide whether to enable hardware later)
	CLSID selectedDecoderCLSID = GUID_NULL;
	bool microsoftDecoderAvailable = false;

	HRESULT hrDecoder = findBestH264Decoder(&selectedDecoderCLSID);
	if (SUCCEEDED(hrDecoder) && selectedDecoderCLSID != GUID_NULL)
	{
		microsoftDecoderAvailable = true;
		MIKAN_LOG_INFO("WMFVideoFrameProcessor::init")
			<< "Microsoft H.264 decoder available for compressed formats";
	}

	// Always enable converters (we may need format conversion)
	if (SUCCEEDED(hr))
		hr = pAttributes->SetUINT32(MF_READWRITE_DISABLE_CONVERTERS, FALSE);

	// Defer hardware transforms decision until we know if format is compressed
	// (Will be set after we detect the native format)

	// Make D3D support optional - allows software fallback if hardware fails
	if (SUCCEEDED(hr))
		hr = pAttributes->SetUINT32(MF_READWRITE_D3D_OPTIONAL, TRUE);

	// Enable low latency mode to reduce buffering
	if (SUCCEEDED(hr))
		hr = pAttributes->SetUINT32(MF_LOW_LATENCY, TRUE);

	// NOTE: We intentionally do NOT set MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING
	// That flag loads vendor-specific video processors which can hang when reconfiguring.

	// Create the source reader
	// If hardware decoding is safe, the source reader will automatically select Microsoft's decoder
	if (SUCCEEDED(hr))
		hr = MFCreateSourceReaderFromMediaSource(pSource, pAttributes, &m_pSourceReader);

	// Log what native format the source reader detected
	if (SUCCEEDED(hr))
	{
		IMFMediaType* pDetectedNativeType = nullptr;
		HRESULT hrNative = m_pSourceReader->GetNativeMediaType(
			(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
			MF_SOURCE_READER_CURRENT_TYPE_INDEX,
			&pDetectedNativeType);

		if (SUCCEEDED(hrNative) && pDetectedNativeType)
		{
			GUID detectedSubtype;
			UINT32 detectedWidth = 0, detectedHeight = 0;
			pDetectedNativeType->GetGUID(MF_MT_SUBTYPE, &detectedSubtype);
			MFGetAttributeSize(pDetectedNativeType, MF_MT_FRAME_SIZE, &detectedWidth, &detectedHeight);

			const char* formatName = "Unknown";
			if (detectedSubtype == MFVideoFormat_H264) formatName = "H264";
			else if (detectedSubtype == MFVideoFormat_YUY2) formatName = "YUY2";
			else if (detectedSubtype == MFVideoFormat_NV12) formatName = "NV12";
			else if (detectedSubtype == MFVideoFormat_MJPG) formatName = "MJPG";

			MIKAN_LOG_INFO("WMFVideoFrameProcessor::init")
				<< "Source reader detected native format: " << formatName
				<< " " << detectedWidth << "x" << detectedHeight;

			MemoryUtils::safeRelease(&pDetectedNativeType);
		}
	}

	// Determine output format based on whether source is compressed
	bool isCompressedFormat =
		(m_deviceFormat.sub_type_name == "H264" ||
		 m_deviceFormat.sub_type_name == "MJPG" ||
		 m_deviceFormat.sub_type_name == "H265" ||
		 m_deviceFormat.sub_type_name == "VP80" ||
		 m_deviceFormat.sub_type_name == "VP90");

	// Save the native input type BEFORE setting output format
	// This preserves the H.264 media type with codec private data for our manual decoder
	if (SUCCEEDED(hr) && isCompressedFormat)
	{
		// Use the selected format index, not 0!
		int formatIndex = (m_deviceFormat.device_format_index >= 0) ? m_deviceFormat.device_format_index : 0;
		HRESULT hrNative = m_pSourceReader->GetNativeMediaType(
			(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
			formatIndex,
			&m_pNativeInputType);

		if (SUCCEEDED(hrNative) && m_pNativeInputType)
		{
			GUID nativeSubtype;
			m_pNativeInputType->GetGUID(MF_MT_SUBTYPE, &nativeSubtype);

			UINT32 nativeWidth = 0, nativeHeight = 0;
			MFGetAttributeSize(m_pNativeInputType, MF_MT_FRAME_SIZE, &nativeWidth, &nativeHeight);

			UINT32 codecDataSize = 0;
			m_pNativeInputType->GetBlobSize(MF_MT_USER_DATA, &codecDataSize);

			// Log the actual GUID to debug
			std::stringstream guidStr;
			guidStr << std::hex << std::uppercase
				<< std::setfill('0') << std::setw(8) << nativeSubtype.Data1 << "-"
				<< std::setw(4) << nativeSubtype.Data2 << "-"
				<< std::setw(4) << nativeSubtype.Data3 << "-"
				<< std::setw(2) << (int)nativeSubtype.Data4[0]
				<< std::setw(2) << (int)nativeSubtype.Data4[1] << "-"
				<< std::setw(2) << (int)nativeSubtype.Data4[2]
				<< std::setw(2) << (int)nativeSubtype.Data4[3]
				<< std::setw(2) << (int)nativeSubtype.Data4[4]
				<< std::setw(2) << (int)nativeSubtype.Data4[5]
				<< std::setw(2) << (int)nativeSubtype.Data4[6]
				<< std::setw(2) << (int)nativeSubtype.Data4[7];

			const char* formatName = "Unknown";
			if (nativeSubtype == MFVideoFormat_H264) formatName = "H264";
			else if (nativeSubtype == MFVideoFormat_YUY2) formatName = "YUY2";
			else if (nativeSubtype == MFVideoFormat_NV12) formatName = "NV12";
			else if (nativeSubtype == MFVideoFormat_MJPG) formatName = "MJPG";

			MIKAN_LOG_INFO("WMFVideoFrameProcessor::init")
				<< "Saved native input type - Subtype GUID: " << guidStr.str()
				<< " (" << formatName << ")"
				<< ", Resolution: " << nativeWidth << "x" << nativeHeight
				<< ", Codec data size: " << codecDataSize << " bytes";

			// If the native format is NOT actually compressed, disable manual decoder
			if (nativeSubtype != MFVideoFormat_H264 &&
				nativeSubtype != MFVideoFormat_H265 &&
				nativeSubtype != MFVideoFormat_MJPG)
			{
				MIKAN_LOG_WARNING("WMFVideoFrameProcessor::init")
					<< "Expected compressed format (" << m_deviceFormat.sub_type_name
					<< ") but camera native format is uncompressed (" << formatName
					<< "). Disabling manual decoder - will use source reader output directly.";
				isCompressedFormat = false;
			}
		}
	}

	// Determine preferred output format
	GUID preferredFormat = GUID_NULL;
	GUID fallbackFormat = GUID_NULL;
	eUSBVideoFrameBufferFormat preferredFrameBufferFormat;
	eUSBVideoFrameBufferFormat fallbackFrameBufferFormat;

	if (isCompressedFormat)
	{
		// For compressed formats, prefer NV12 (native decoder output, better performance)
		// with YUY2 as fallback (universally supported but slower due to conversion)
		preferredFormat = MFVideoFormat_YUY2;
		preferredFrameBufferFormat = eUSBVideoFrameBufferFormat::USBVideo_YUY2;
		fallbackFormat = MFVideoFormat_YUY2;
		fallbackFrameBufferFormat = eUSBVideoFrameBufferFormat::USBVideo_YUY2;

		MIKAN_LOG_INFO("WMFVideoFrameProcessor::init")
			<< "Compressed format detected (" << m_deviceFormat.sub_type_name
			<< "), will try NV12 output (fallback: YUY2)";
	}
	else
	{
		// For uncompressed formats, pass through the native format without conversion
		// Color conversion (YUY2→RGB24) can load AMD video processors which cause hangs
		preferredFormat = GUID_NULL; // Will use native format
		preferredFrameBufferFormat = eUSBVideoFrameBufferFormat::USBVideo_UNKNOWN;
		fallbackFormat = GUID_NULL;
		fallbackFrameBufferFormat = eUSBVideoFrameBufferFormat::USBVideo_UNKNOWN;

		MIKAN_LOG_INFO("WMFVideoFrameProcessor::init")
			<< "Uncompressed format - will use native format without conversion";
	}

	// Now configure hardware transforms based on whether format is compressed
	if (isCompressedFormat && microsoftDecoderAvailable)
	{
		// Safe to use hardware decoding for compressed formats
		if (SUCCEEDED(hr))
			hr = pAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, TRUE);

		MIKAN_LOG_INFO("WMFVideoFrameProcessor::init")
			<< "Enabling hardware transforms for H.264 decoding";
	}
	else
	{
		// Disable hardware transforms for uncompressed formats or if no safe decoder
		// This prevents AMD/NVIDIA video processors from being loaded
		if (SUCCEEDED(hr))
			hr = pAttributes->SetUINT32(MF_READWRITE_ENABLE_HARDWARE_TRANSFORMS, FALSE);

		if (!isCompressedFormat)
		{
			MIKAN_LOG_INFO("WMFVideoFrameProcessor::init")
				<< "Disabling hardware transforms for uncompressed format (avoids vendor video processors)";
		}
	}

	m_wmfOutputFormat = preferredFormat;
	m_outputFormat = preferredFrameBufferFormat;

	// Configure the output media type by cloning the native type and changing only the subtype
	// This preserves all attributes (frame rate, interlace mode, color space, etc.)
	// Try preferred format first, fallback to alternative if not supported
	// SKIP if m_wmfOutputFormat is GUID_NULL (passthrough mode for uncompressed)
	IMFMediaType* pType = nullptr;
	bool tryFallback = false;
	bool useNativePassthrough = (m_wmfOutputFormat == GUID_NULL);

	if (SUCCEEDED(hr) && !useNativePassthrough && m_deviceFormat.device_format_index >= 0)
	{
		// Get the complete native media type by index
		IMFMediaType* pNativeType = nullptr;
		hr = m_pSourceReader->GetNativeMediaType(
			(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
			m_deviceFormat.device_format_index,
			&pNativeType);

		if (SUCCEEDED(hr))
		{
			// Clone the native type to preserve all attributes
			hr = MFCreateMediaType(&pType);
			if (SUCCEEDED(hr))
			{
				hr = pNativeType->CopyAllItems(pType);
			}

			// Change only the subtype to trigger decoding (e.g., H264 -> YUY2)
			if (SUCCEEDED(hr))
			{
				hr = pType->SetGUID(MF_MT_SUBTYPE, m_wmfOutputFormat);
			}

			MemoryUtils::safeRelease(&pNativeType);
		}
	}
	else if (useNativePassthrough)
	{
		MIKAN_LOG_INFO("WMFVideoFrameProcessor::init")
			<< "Using native passthrough - no output format conversion";
	}

	// Fallback: create minimal media type if we couldn't get native type by index
	if (!pType && SUCCEEDED(hr))
	{
		MIKAN_LOG_WARNING("WMFVideoFrameProcessor::init")
			<< "Could not get native type by index, creating minimal output type";

		hr = MFCreateMediaType(&pType);
		if (SUCCEEDED(hr))
			hr = pType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
		if (SUCCEEDED(hr))
			hr = pType->SetGUID(MF_MT_SUBTYPE, m_wmfOutputFormat);
		if (SUCCEEDED(hr))
		{
			hr = MFSetAttributeSize(pType, MF_MT_FRAME_SIZE,
				m_deviceFormat.width, m_deviceFormat.height);
		}
		if (SUCCEEDED(hr) && m_deviceFormat.frame_rate_numerator > 0 && m_deviceFormat.frame_rate_denominator > 0)
		{
			hr = MFSetAttributeRatio(pType, MF_MT_FRAME_RATE,
				m_deviceFormat.frame_rate_numerator,
				m_deviceFormat.frame_rate_denominator);
		}
	}

	// Try to set the output type on the first video stream (skip if passthrough)
	if (SUCCEEDED(hr) && pType && !useNativePassthrough)
	{
		const char* formatName =
			(m_wmfOutputFormat == MFVideoFormat_NV12) ? "NV12" :
			(m_wmfOutputFormat == MFVideoFormat_YUY2) ? "YUY2" : "RGB24";

		MIKAN_LOG_INFO("WMFVideoFrameProcessor::init")
			<< "Attempting to set output format to " << formatName << "...";

		hr = m_pSourceReader->SetCurrentMediaType(
			(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
			nullptr,
			pType);

		if (SUCCEEDED(hr))
		{
			MIKAN_LOG_INFO("WMFVideoFrameProcessor::init")
				<< "Successfully set output format to " << formatName;
		}
		else if (fallbackFormat != GUID_NULL)
		{
			// Preferred format failed, try fallback
			MIKAN_LOG_WARNING("WMFVideoFrameProcessor::init")
				<< "Failed to set preferred output format: " << GetHresultMessage(hr)
				<< ", trying fallback format";

			tryFallback = true;
		}
	}
	else if (useNativePassthrough)
	{
		// For passthrough, detect the native format and use it
		IMFMediaType* pNativeType = nullptr;
		HRESULT hrNative = m_pSourceReader->GetNativeMediaType(
			(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
			MF_SOURCE_READER_CURRENT_TYPE_INDEX,
			&pNativeType);

		if (SUCCEEDED(hrNative))
		{
			GUID nativeSubtype;
			pNativeType->GetGUID(MF_MT_SUBTYPE, &nativeSubtype);

			// Update our tracked format to match native
			m_wmfOutputFormat = nativeSubtype;

			if (nativeSubtype == MFVideoFormat_YUY2)
			{
				m_outputFormat = eUSBVideoFrameBufferFormat::USBVideo_YUY2;
				MIKAN_LOG_INFO("WMFVideoFrameProcessor::init")
					<< "Using native YUY2 format (passthrough)";
			}
			else if (nativeSubtype == MFVideoFormat_NV12)
			{
				m_outputFormat = eUSBVideoFrameBufferFormat::USBVideo_NV12;
				MIKAN_LOG_INFO("WMFVideoFrameProcessor::init")
					<< "Using native NV12 format (passthrough)";
			}
			else
			{
				MIKAN_LOG_WARNING("WMFVideoFrameProcessor::init")
					<< "Unknown native format for passthrough";
			}

			MemoryUtils::safeRelease(&pNativeType);
		}
	}

	// Try fallback format if preferred failed (skip if passthrough)
	if (tryFallback && !useNativePassthrough)
	{
		MemoryUtils::safeRelease(&pType);

		// Recreate media type with fallback format
		if (m_deviceFormat.device_format_index >= 0)
		{
			IMFMediaType* pNativeType = nullptr;
			hr = m_pSourceReader->GetNativeMediaType(
				(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
				m_deviceFormat.device_format_index,
				&pNativeType);

			if (SUCCEEDED(hr))
			{
				hr = MFCreateMediaType(&pType);
				if (SUCCEEDED(hr))
				{
					hr = pNativeType->CopyAllItems(pType);
				}
				if (SUCCEEDED(hr))
				{
					hr = pType->SetGUID(MF_MT_SUBTYPE, fallbackFormat);
				}
				MemoryUtils::safeRelease(&pNativeType);
			}
		}

		if (SUCCEEDED(hr) && pType)
		{
			const char* fallbackFormatName =
				(fallbackFormat == MFVideoFormat_NV12) ? "NV12" :
				(fallbackFormat == MFVideoFormat_YUY2) ? "YUY2" : "RGB24";

			MIKAN_LOG_INFO("WMFVideoFrameProcessor::init")
				<< "Attempting to set fallback format to " << fallbackFormatName << "...";

			hr = m_pSourceReader->SetCurrentMediaType(
				(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
				nullptr,
				pType);

			if (SUCCEEDED(hr))
			{
				// Update our tracked format to the fallback
				m_wmfOutputFormat = fallbackFormat;
				m_outputFormat = fallbackFrameBufferFormat;

				MIKAN_LOG_INFO("WMFVideoFrameProcessor::init")
					<< "Fallback successful - using " << fallbackFormatName << " output format";
			}
			else
			{
				MIKAN_LOG_ERROR("WMFVideoFrameProcessor::init")
					<< "Fallback format also failed: " << GetHresultMessage(hr);
			}
		}
	}

	// Verify we actually got the format we requested
	IMFMediaType* pActualType = nullptr;
	if (SUCCEEDED(hr))
	{
		hr = m_pSourceReader->GetCurrentMediaType(
			(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
			&pActualType);

		if (SUCCEEDED(hr))
		{
			GUID actualSubtype;
			hr = pActualType->GetGUID(MF_MT_SUBTYPE, &actualSubtype);

			if (SUCCEEDED(hr))
			{
				// Get the native media type to see what the camera actually provides
				IMFMediaType* pNativeType = nullptr;
				HRESULT hrNative = m_pSourceReader->GetNativeMediaType(
					(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
					0,
					&pNativeType);

				if (SUCCEEDED(hrNative))
				{
					GUID nativeSubtype;
					pNativeType->GetGUID(MF_MT_SUBTYPE, &nativeSubtype);

					const char* requestedFormatName =
						(m_wmfOutputFormat == MFVideoFormat_NV12) ? "NV12" :
						(m_wmfOutputFormat == MFVideoFormat_YUY2) ? "YUY2" : "RGB24";

					MIKAN_LOG_INFO("WMFVideoFrameProcessor::init")
						<< "Native camera format: " << (nativeSubtype == MFVideoFormat_H264 ? "H264" : "Other")
						<< ", Requested output: " << requestedFormatName
						<< ", Actual output: " << (actualSubtype == m_wmfOutputFormat ? "MATCH" : "MISMATCH");

					MemoryUtils::safeRelease(&pNativeType);
				}

				const char* requestedFormatName2 =
					(m_wmfOutputFormat == MFVideoFormat_NV12) ? "NV12" :
					(m_wmfOutputFormat == MFVideoFormat_YUY2) ? "YUY2" : "RGB24";

				if (actualSubtype != m_wmfOutputFormat)
				{
					MIKAN_LOG_ERROR("WMFVideoFrameProcessor::init")
						<< "Failed to set desired output format. Requested: "
						<< requestedFormatName2
						<< ", but got a different format (possibly still compressed)";
					hr = E_FAIL;
				}
				else
				{
					MIKAN_LOG_INFO("WMFVideoFrameProcessor::init")
						<< "Successfully configured output format: "
						<< requestedFormatName2;
				}
			}
		}
	}

	// We're using explicit decoder in the pipeline (no manual decoder object needed)
	m_bNeedsDecoder = false;

	// Clean up
	if (FAILED(hr))
	{
		MemoryUtils::safeRelease(&m_pSourceReader);
	}

	MemoryUtils::safeRelease(&pActualType);
	MemoryUtils::safeRelease(&pType);
	MemoryUtils::safeRelease(&pAttributes);

	return hr;
}

void WMFVideoFrameProcessor::dispose()
{
	stopVideoFrameStream();

	MemoryUtils::safeReleaseAllCount(&m_pNativeInputType);
	MemoryUtils::safeReleaseAllCount(&m_pDecoderTransform);
	MemoryUtils::safeReleaseAllCount(&m_pSourceReader);

	MIKAN_LOG_INFO("WMFVideoFrameProcessor::dispose")
		<< "Disposing video frame reader for device: " << m_deviceIndex;
}

void WMFVideoFrameProcessor::startVideoFrameStream()
{
	if (m_state == State::Stopped || m_state == State::Failed)
	{
		m_sampleIndex = 0;
		m_state = State::Starting;

		// Request the first sample - this starts the async callback loop
		HRESULT hr = m_pSourceReader->ReadSample(
			(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
			0,  // No flags
			nullptr,  // Don't need actual stream index back
			nullptr,  // Don't need flags back
			nullptr,  // Don't need timestamp back
			nullptr); // Sample will be delivered to OnReadSample()

		if (SUCCEEDED(hr))
		{
			m_state = State::Running;
			MIKAN_LOG_INFO("WMFVideoFrameProcessor::startVideoFrameStream")
				<< "Started video stream for device: " << m_deviceIndex;
		}
		else
		{
			MIKAN_LOG_ERROR("WMFVideoFrameProcessor::startVideoFrameStream")
				<< "Failed to start video stream: " << GetHresultMessage(hr);
			m_state = State::Failed;
		}
	}
	else
	{
		MIKAN_LOG_WARNING("WMFVideoFrameProcessor::startVideoFrameStream")
			<< "Cannot start stream due to pending stream operation";
	}
}

void WMFVideoFrameProcessor::stopVideoFrameStream()
{
	if (m_state != State::Stopped)
	{
		MIKAN_LOG_INFO("WMFVideoFrameProcessor::stopVideoFrameStream")
			<< "Stopping video frame reading on device: " << m_deviceIndex;

		m_state = State::Stopped;

		// Flush the source reader to stop receiving samples
		if (m_pSourceReader != nullptr)
		{
			m_pSourceReader->Flush((DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM);
		}
	}
}

STDMETHODIMP WMFVideoFrameProcessor::OnReadSample(
	HRESULT hrStatus,
	DWORD dwStreamIndex,
	DWORD dwStreamFlags,
	LONGLONG llTimestamp,
	IMFSample* pSample)
{
	// Check if we're still running
	if (m_state != State::Running)
	{
		if (m_sampleIndex >= 9 && m_sampleIndex <= 11)
		{
			MIKAN_LOG_WARNING("WMFVideoFrameProcessor::OnReadSample")
				<< "Frame " << m_sampleIndex << ": State is not Running (state=" << (int)m_state << ")";
		}
		return S_OK;
	}

	// Handle errors
	if (FAILED(hrStatus))
	{
		MIKAN_LOG_ERROR("WMFVideoFrameProcessor::OnReadSample")
			<< "Read sample failed: " << GetHresultMessage(hrStatus);
		m_state = State::Failed;
		return hrStatus;
	}

	// Check for end of stream
	if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM)
	{
		MIKAN_LOG_INFO("WMFVideoFrameProcessor::OnReadSample")
			<< "End of stream";
		return S_OK;
	}

	// Process the sample if we have one
	if (pSample && m_videoSourceListener)
	{
		const size_t width = m_deviceFormat.width;
		const size_t height = m_deviceFormat.height;

		// If we have a manual decoder and this is a compressed sample, decode it first
		IMFSample* pProcessedSample = pSample;
		IMFSample* pDecodedSample = nullptr;
		if (m_bNeedsDecoder && m_pDecoderTransform)
		{
			HRESULT hrDecode = processCompressedSample(pSample, &pDecodedSample);
			if (hrDecode == S_OK && pDecodedSample)
			{
				pProcessedSample = pDecodedSample;
			}
			else if (hrDecode == S_FALSE)
			{
				// Decoder needs more input - skip this frame and continue
				m_sampleIndex++;
				if (m_state == State::Running)
				{
					m_pSourceReader->ReadSample(
						(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
						0, nullptr, nullptr, nullptr, nullptr);
				}
				else
				{
					MIKAN_LOG_WARNING("WMFVideoFrameProcessor::OnReadSample")
						<< "State is no longer Running after decode S_FALSE, stopping stream";
				}
				return S_OK;
			}
			else
			{
				MIKAN_LOG_ERROR("WMFVideoFrameProcessor::OnReadSample")
					<< "Manual decoder failed: " << GetHresultMessage(hrDecode);
				// Skip this frame and continue
				m_sampleIndex++;
				if (m_state == State::Running)
				{
					m_pSourceReader->ReadSample(
						(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
						0, nullptr, nullptr, nullptr, nullptr);
				}
				return S_OK;
			}
		}

		// Convert to contiguous buffer - this removes all padding!
		IMFMediaBuffer* pBuffer = nullptr;
		HRESULT hr = pProcessedSample->ConvertToContiguousBuffer(&pBuffer);

		if (SUCCEEDED(hr))
		{
			BYTE* pData = nullptr;
			DWORD dwBufferLength = 0;

			hr = pBuffer->Lock(&pData, nullptr, &dwBufferLength);

			if (SUCCEEDED(hr))
			{
				// Create frame buffer info
				UsbVideoFrameBuffer frameBuffer;
				frameBuffer.data = pData;
				frameBuffer.byte_count = dwBufferLength;
				frameBuffer.width = (int)width;
				frameBuffer.height = (int)height;
				frameBuffer.data_format = m_outputFormat;
				frameBuffer.stride = (int)width;  // Contiguous buffer = stride equals width!

				// Log first sample
				if (m_sampleIndex == 0)
				{
					const size_t expectedNV12Size = width * (height + height / 2);
					const size_t expectedRGB24Size = width * height * 3;
					const size_t expectedYUY2Size = width * height * 2;

					const char* formatName =
						(m_outputFormat == eUSBVideoFrameBufferFormat::USBVideo_NV12) ? "NV12" :
						(m_outputFormat == eUSBVideoFrameBufferFormat::USBVideo_YUY2) ? "YUY2" : "RGB24";

					const size_t expectedSize =
						(m_outputFormat == eUSBVideoFrameBufferFormat::USBVideo_NV12) ? expectedNV12Size :
						(m_outputFormat == eUSBVideoFrameBufferFormat::USBVideo_YUY2) ? expectedYUY2Size : expectedRGB24Size;

					MIKAN_LOG_INFO("WMFVideoFrameProcessor::OnReadSample")
						<< "First sample - Format: " << (int)m_outputFormat
						<< " (" << formatName << ")"
						<< ", Actual buffer size: " << dwBufferLength << " bytes"
						<< ", Expected size: " << expectedSize << " bytes"
						<< ", Width: " << width << ", Height: " << height
						<< ", Contiguous buffer (no padding)";

					if (dwBufferLength != expectedSize)
					{
						if (m_outputFormat == eUSBVideoFrameBufferFormat::USBVideo_NV12 && dwBufferLength > expectedSize)
						{
							MIKAN_LOG_WARNING("WMFVideoFrameProcessor::OnReadSample")
								<< "NV12 buffer has extra padding (inter-plane alignment). "
								<< "Extra bytes: " << (dwBufferLength - expectedSize)
								<< ". This may cause visual artifacts. "
								<< "ConvertToContiguousBuffer doesn't remove inter-plane padding in planar formats. "
								<< "Consider using YUY2 instead for simpler buffer layout.";
						}
						else
						{
							MIKAN_LOG_ERROR("WMFVideoFrameProcessor::OnReadSample")
								<< "CRITICAL: Buffer size mismatch! Receiving compressed H.264 data instead of decoded "
								<< formatName << " frames. WMF decoder is NOT working!";
						}
					}
					else
					{
						MIKAN_LOG_INFO("WMFVideoFrameProcessor::OnReadSample")
							<< "SUCCESS: Buffer size matches expected " << formatName << " format. Decoder is working!";
					}
				}

				// Notify listener
				m_videoSourceListener->notifyVideoFrameReceived(frameBuffer);

				pBuffer->Unlock();
			}

			MemoryUtils::safeRelease(&pBuffer);
		}

		// Clean up decoded sample if we created one
		if (pDecodedSample)
		{
			MemoryUtils::safeRelease(&pDecodedSample);
		}

		m_sampleIndex++;
	}

	// Request next sample to continue the async loop
	if (m_state == State::Running)
	{
		m_pSourceReader->ReadSample(
			(DWORD)MF_SOURCE_READER_FIRST_VIDEO_STREAM,
			0,
			nullptr,
			nullptr,
			nullptr,
			nullptr);
	}

	return S_OK;
}

STDMETHODIMP WMFVideoFrameProcessor::QueryInterface(REFIID riid, void** ppv)
{
	static const QITAB qit[] =
	{
		QITABENT(WMFVideoFrameProcessor, IMFSourceReaderCallback),
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

HRESULT WMFVideoFrameProcessor::findBestH264Decoder(CLSID* pDecoderCLSID)
{
	if (!pDecoderCLSID)
	{
		return E_POINTER;
	}

	// Known problematic vendor MFT CLSIDs to avoid
	// AMD Advanced Media Framework (AMF) Decoders
	static const CLSID CLSID_AMD_H264_DECODER =
		{ 0x82CE8B14, 0xF24E, 0x4F2E, { 0x80, 0x93, 0xDB, 0x8C, 0x63, 0x09, 0x87, 0x72 } };

	// NVIDIA Video Decoder
	static const CLSID CLSID_NVIDIA_H264_DECODER =
		{ 0x56AF0A3E, 0x47A9, 0x4F49, { 0x9F, 0x7A, 0x6C, 0x1A, 0x6E, 0x72, 0x0B, 0x5A } };

	// Configure input/output types for MFT enumeration
	MFT_REGISTER_TYPE_INFO inputType = { MFMediaType_Video, MFVideoFormat_H264 };
	MFT_REGISTER_TYPE_INFO outputType = { MFMediaType_Video, MFVideoFormat_NV12 };

	UINT32 flags = MFT_ENUM_FLAG_SYNCMFT | MFT_ENUM_FLAG_ASYNCMFT |
	               MFT_ENUM_FLAG_HARDWARE | MFT_ENUM_FLAG_SORTANDFILTER;

	IMFActivate** ppActivate = nullptr;
	UINT32 count = 0;

	// Enumerate H.264 decoders
	HRESULT hr = MFTEnumEx(
		MFT_CATEGORY_VIDEO_DECODER,
		flags,
		&inputType,
		&outputType,
		&ppActivate,
		&count);

	if (FAILED(hr) || count == 0)
	{
		MIKAN_LOG_WARNING("WMFVideoFrameProcessor::findBestH264Decoder")
			<< "No H.264 decoders found: " << GetHresultMessage(hr);
		return E_FAIL;
	}

	MIKAN_LOG_INFO("WMFVideoFrameProcessor::findBestH264Decoder")
		<< "Found " << count << " H.264 decoder(s)";

	// Find the best decoder (prefer Microsoft, avoid AMD/NVIDIA)
	CLSID selectedCLSID = GUID_NULL;
	CLSID microsoftCLSID = GUID_NULL;
	bool foundMicrosoft = false;

	for (UINT32 i = 0; i < count; i++)
	{
		CLSID clsid;
		hr = ppActivate[i]->GetGUID(MFT_TRANSFORM_CLSID_Attribute, &clsid);

		if (SUCCEEDED(hr))
		{
			// Get friendly name for logging
			WCHAR* pName = nullptr;
			UINT32 nameLen = 0;
			ppActivate[i]->GetAllocatedString(MFT_FRIENDLY_NAME_Attribute, &pName, &nameLen);

			std::wstring wname = pName ? pName : L"Unknown";
			std::string name(wname.begin(), wname.end());

			// Check if this is a problematic vendor MFT
			bool isProblematic = (clsid == CLSID_AMD_H264_DECODER ||
			                      clsid == CLSID_NVIDIA_H264_DECODER);

			// Check if this is Microsoft's decoder (best guess by name)
			bool isMicrosoft = (name.find("Microsoft") != std::string::npos);

			MIKAN_LOG_INFO("WMFVideoFrameProcessor::findBestH264Decoder")
				<< "  [" << i << "] " << name
				<< (isMicrosoft ? " (Microsoft)" : "")
				<< (isProblematic ? " (PROBLEMATIC vendor - may cause hangs)" : "");

			if (pName)
			{
				CoTaskMemFree(pName);
			}

			// Prefer Microsoft's decoder (don't skip problematic ones, just prioritize Microsoft)
			if (isMicrosoft && !foundMicrosoft)
			{
				microsoftCLSID = clsid;
				foundMicrosoft = true;
			}

			// Track any non-Microsoft decoder as fallback
			if (!isMicrosoft && selectedCLSID == GUID_NULL)
			{
				selectedCLSID = clsid;
			}
		}
	}

	// Cleanup
	for (UINT32 i = 0; i < count; i++)
	{
		ppActivate[i]->Release();
	}
	CoTaskMemFree(ppActivate);

	// Prefer Microsoft, fallback to first non-problematic
	if (foundMicrosoft)
	{
		*pDecoderCLSID = microsoftCLSID;
		MIKAN_LOG_INFO("WMFVideoFrameProcessor::findBestH264Decoder")
			<< "Selected Microsoft H.264 decoder";
		return S_OK;
	}
	else if (selectedCLSID != GUID_NULL)
	{
		*pDecoderCLSID = selectedCLSID;
		MIKAN_LOG_INFO("WMFVideoFrameProcessor::findBestH264Decoder")
			<< "Selected first available non-problematic decoder";
		return S_OK;
	}

	MIKAN_LOG_ERROR("WMFVideoFrameProcessor::findBestH264Decoder")
		<< "No suitable H.264 decoder found";
	return E_FAIL;
}

HRESULT WMFVideoFrameProcessor::getActualInputMediaType(IMFMediaType** ppMediaType)
{
	if (!ppMediaType)
	{
		return E_POINTER;
	}

	*ppMediaType = nullptr;

	// Return the saved native media type that we captured before setting output format
	if (!m_pNativeInputType)
	{
		MIKAN_LOG_ERROR("WMFVideoFrameProcessor::getActualInputMediaType")
			<< "Native input type was not saved";
		return E_FAIL;
	}

	// AddRef and return the saved type
	m_pNativeInputType->AddRef();
	*ppMediaType = m_pNativeInputType;
	return S_OK;
}

HRESULT WMFVideoFrameProcessor::createDecoder()
{
	// Create H.264 decoder MFT
	HRESULT hr = CoCreateInstance(CLSID_CMSH264DecoderMFT, nullptr,
		CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_pDecoderTransform));

	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("WMFVideoFrameProcessor::createDecoder")
			<< "Failed to create H.264 decoder: " << GetHresultMessage(hr);
		return hr;
	}

	// Try to enable low-latency mode (reduces buffering)
	IMFAttributes* pAttributes = nullptr;
	if (SUCCEEDED(m_pDecoderTransform->GetAttributes(&pAttributes)))
	{
		// Enable low-latency mode to minimize decoder buffering
		pAttributes->SetUINT32(MF_LOW_LATENCY, TRUE);

		MemoryUtils::safeRelease(&pAttributes);
	}

	// Get the actual input media type from the source reader
	// This is critical because it includes codec private data (SPS/PPS for H.264)
	IMFMediaType* pInputType = nullptr;
	hr = getActualInputMediaType(&pInputType);

	if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("WMFVideoFrameProcessor::createDecoder")
			<< "Failed to get input media type: " << GetHresultMessage(hr);
		return hr;
	}

	// Log some info about the input type
	GUID inputSubtype = GUID_NULL;
	pInputType->GetGUID(MF_MT_SUBTYPE, &inputSubtype);

	UINT32 codecDataSize = 0;
	pInputType->GetBlobSize(MF_MT_USER_DATA, &codecDataSize);

	MIKAN_LOG_INFO("WMFVideoFrameProcessor::createDecoder")
		<< "Input media type - Subtype: " << (inputSubtype == MFVideoFormat_H264 ? "H264" : "Other")
		<< ", Codec private data size: " << codecDataSize << " bytes";

	// Set the input type on the decoder
	if (SUCCEEDED(hr))
	{
		hr = m_pDecoderTransform->SetInputType(0, pInputType, 0);
		if (FAILED(hr))
		{
			MIKAN_LOG_ERROR("WMFVideoFrameProcessor::createDecoder")
				<< "Failed to set input type: " << GetHresultMessage(hr);
		}
	}

	// Set output type (YUY2)
	IMFMediaType* pOutputType = nullptr;
	if (SUCCEEDED(hr))
	{
		hr = MFCreateMediaType(&pOutputType);
	}

	if (SUCCEEDED(hr))
	{
		hr = pOutputType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
	}

	if (SUCCEEDED(hr))
	{
		hr = pOutputType->SetGUID(MF_MT_SUBTYPE, m_wmfOutputFormat);
	}

	if (SUCCEEDED(hr))
	{
		hr = MFSetAttributeSize(pOutputType, MF_MT_FRAME_SIZE,
			m_deviceFormat.width, m_deviceFormat.height);
	}

	// Set frame rate on output type too
	if (SUCCEEDED(hr) && m_deviceFormat.frame_rate_numerator > 0 && m_deviceFormat.frame_rate_denominator > 0)
	{
		MFSetAttributeRatio(pOutputType, MF_MT_FRAME_RATE,
			m_deviceFormat.frame_rate_numerator, m_deviceFormat.frame_rate_denominator);
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pDecoderTransform->SetOutputType(0, pOutputType, 0);
	}

	// Get output stream info to determine if we need to allocate output samples
	if (SUCCEEDED(hr))
	{
		hr = m_pDecoderTransform->GetOutputStreamInfo(0, &m_decoderOutputInfo);
	}

	// Start streaming on the decoder
	if (SUCCEEDED(hr))
	{
		hr = m_pDecoderTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_BEGIN_STREAMING, 0);
	}

	if (SUCCEEDED(hr))
	{
		hr = m_pDecoderTransform->ProcessMessage(MFT_MESSAGE_NOTIFY_START_OF_STREAM, 0);
	}

	MemoryUtils::safeRelease(&pInputType);
	MemoryUtils::safeRelease(&pOutputType);

	return hr;
}

HRESULT WMFVideoFrameProcessor::processCompressedSample(IMFSample* pCompressedSample, IMFSample** ppDecodedSample)
{
	if (!m_pDecoderTransform || !pCompressedSample || !ppDecodedSample)
	{
		return E_POINTER;
	}

	*ppDecodedSample = nullptr;

	// Send compressed sample to decoder
	HRESULT hr = m_pDecoderTransform->ProcessInput(0, pCompressedSample, 0);

	bool bInputAccepted = SUCCEEDED(hr);

	if (hr == MF_E_NOTACCEPTING)
	{
		// Decoder's input buffer is full - we need to drain output first before adding more input
		// This is normal for H.264 decoders. Try to get output without adding more input.
		MIKAN_LOG_INFO("WMFVideoFrameProcessor::processCompressedSample")
			<< "Decoder input buffer full (frame " << m_sampleIndex << "), draining output";
		// Don't return error, just try to get output below
		hr = S_OK;
		bInputAccepted = false;
	}
	else if (FAILED(hr))
	{
		MIKAN_LOG_ERROR("WMFVideoFrameProcessor::processCompressedSample")
			<< "ProcessInput failed: " << GetHresultMessage(hr);
		return hr;
	}
	else if (m_sampleIndex < 10)
	{
		MIKAN_LOG_INFO("WMFVideoFrameProcessor::processCompressedSample")
			<< "ProcessInput succeeded for frame " << m_sampleIndex;
	}

	// Check if we need to allocate output sample (most MFTs don't provide their own)
	bool bNeedOutputSample = (m_decoderOutputInfo.dwFlags & MFT_OUTPUT_STREAM_PROVIDES_SAMPLES) == 0;

	// Try to get output - may need to call multiple times as decoder can buffer frames
	// H.264 decoders often need several input frames before producing first output
	for (int attempt = 0; attempt < 10; attempt++)
	{
		// Prepare output buffer
		MFT_OUTPUT_DATA_BUFFER outputBuffer = {0};
		outputBuffer.dwStreamID = 0;
		outputBuffer.pSample = nullptr;
		outputBuffer.dwStatus = 0;
		outputBuffer.pEvents = nullptr;

		// Allocate output sample if needed
		if (bNeedOutputSample)
		{
			hr = MFCreateSample(&outputBuffer.pSample);
			if (FAILED(hr))
			{
				MIKAN_LOG_ERROR("WMFVideoFrameProcessor::processCompressedSample")
					<< "MFCreateSample failed: " << GetHresultMessage(hr);
				return hr;
			}

			IMFMediaBuffer* pBuffer = nullptr;
			hr = MFCreateMemoryBuffer(m_decoderOutputInfo.cbSize, &pBuffer);
			if (SUCCEEDED(hr))
			{
				hr = outputBuffer.pSample->AddBuffer(pBuffer);
				MemoryUtils::safeRelease(&pBuffer);
			}

			if (FAILED(hr))
			{
				MIKAN_LOG_ERROR("WMFVideoFrameProcessor::processCompressedSample")
					<< "Buffer allocation failed: " << GetHresultMessage(hr);
				MemoryUtils::safeRelease(&outputBuffer.pSample);
				return hr;
			}
		}

		// Get decoded output
		DWORD dwStatus = 0;
		hr = m_pDecoderTransform->ProcessOutput(0, 1, &outputBuffer, &dwStatus);

		if (hr == MF_E_TRANSFORM_NEED_MORE_INPUT)
		{
			// Decoder needs more input before producing output (this is normal for first few frames)
			if (outputBuffer.pSample)
			{
				MemoryUtils::safeRelease(&outputBuffer.pSample);
			}
			if (attempt == 0 && m_sampleIndex < 10)
			{
				MIKAN_LOG_INFO("WMFVideoFrameProcessor::processCompressedSample")
					<< "Decoder buffering (frame " << m_sampleIndex << ", attempt " << attempt
					<< ", input " << (bInputAccepted ? "accepted" : "not accepted") << ")";
			}

			// If input wasn't accepted and we have no output, we're stuck
			// This shouldn't happen but log it if it does
			if (!bInputAccepted && attempt == 0)
			{
				MIKAN_LOG_WARNING("WMFVideoFrameProcessor::processCompressedSample")
					<< "Frame " << m_sampleIndex << ": Input rejected but no output available - decoder may be stuck";
			}

			return S_FALSE;
		}

		if (hr == MF_E_TRANSFORM_STREAM_CHANGE)
		{
			// Output format changed, need to query new type and set it
			if (outputBuffer.pSample)
			{
				MemoryUtils::safeRelease(&outputBuffer.pSample);
			}

			MIKAN_LOG_INFO("WMFVideoFrameProcessor::processCompressedSample")
				<< "Stream format change detected, updating output type";

			// Query available output types
			IMFMediaType* pNewOutputType = nullptr;
			hr = m_pDecoderTransform->GetOutputAvailableType(0, 0, &pNewOutputType);
			if (SUCCEEDED(hr))
			{
				// Set the new output type
				hr = m_pDecoderTransform->SetOutputType(0, pNewOutputType, 0);
				MemoryUtils::safeRelease(&pNewOutputType);

				if (SUCCEEDED(hr))
				{
					// Update stream info with new format
					hr = m_pDecoderTransform->GetOutputStreamInfo(0, &m_decoderOutputInfo);

					if (SUCCEEDED(hr))
					{
						// Continue loop to try getting output again with new format
						continue;
					}
				}
			}

			MIKAN_LOG_ERROR("WMFVideoFrameProcessor::processCompressedSample")
				<< "Failed to handle stream change: " << GetHresultMessage(hr);
			return hr;
		}

		if (SUCCEEDED(hr) && outputBuffer.pSample)
		{
			*ppDecodedSample = outputBuffer.pSample;
			if (outputBuffer.pEvents)
			{
				outputBuffer.pEvents->Release();
			}

			if (m_sampleIndex < 5)
			{
				MIKAN_LOG_INFO("WMFVideoFrameProcessor::processCompressedSample")
					<< "Successfully decoded frame " << m_sampleIndex;
			}
			return S_OK;
		}

		// Some other error occurred
		if (outputBuffer.pSample)
		{
			MemoryUtils::safeRelease(&outputBuffer.pSample);
		}
		if (outputBuffer.pEvents)
		{
			outputBuffer.pEvents->Release();
		}

		MIKAN_LOG_ERROR("WMFVideoFrameProcessor::processCompressedSample")
			<< "ProcessOutput failed (attempt " << attempt << "): " << GetHresultMessage(hr);
		return hr;
	}

	// Should not reach here
	return E_FAIL;
}

static std::string GetHresultMessage(HRESULT hr)
{
	// Check if it's a facility_win32 HRESULT
	if (SUCCEEDED(hr)) 
	{
		return "Operation successful";
	}

	DWORD flags = FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;

	// Handle specific facilities if needed (e.g., FACILITY_WIN32)
	if ((hr & 0xFFFF0000) == MAKE_HRESULT(1, FACILITY_WIN32, 0)) {
		flags |= FORMAT_MESSAGE_FROM_SYSTEM; // Already included above, but good for clarity
		hr = HRESULT_CODE(hr); // Extract the Win32 error code
	}

	LPSTR messageBuffer = nullptr;
	size_t size = FormatMessageA(
		flags,
		NULL,
		hr, // Use the HRESULT or the extracted code
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPSTR)&messageBuffer,
		0,
		NULL
	);

	std::stringstream stream;
	stream << "Unknown error 0x" << std::hex << std::uppercase << hr;
	std::string message = stream.str();

	if (size > 0) {
		message = messageBuffer;
		LocalFree(messageBuffer); // Free the buffer allocated by FormatMessage
	}

	return message;
}