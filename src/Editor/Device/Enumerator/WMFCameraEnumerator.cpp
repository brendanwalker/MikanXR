// Adapted from "Capturing Video from Web-camera on Windows 7 and 8 by using Media Foundation"
// by Evgeny Pereguda
// https://www.codeproject.com/Tips/559437/Capturing-Video-from-Web-camera-on-Windows-and-by

// -- includes -----
#include "CameraMath.h"
#include "WMFCameraEnumerator.h"
#include "VideoCapabilitiesConfig.h"
#include "Logger.h"
#include "MemoryUtils.h"
#include "StringUtils.h"

#include "assert.h"
#include "string.h"
#include "strsafe.h"

#include <string>

#include <Mfidl.h>
#include <Mfapi.h>
#include <Shlwapi.h>

//#include <opencv2/videoio/videoio.hpp>

// -- Statics --
VideoCapabilitiesSet *WMFCameraEnumerator::s_videoCapabilitiesSet = nullptr;

// -- Structures --
struct WMFDeviceList
{
	std::vector<WMFDeviceInfo> deviceList;

	bool fetchDeviceList();
};

// -- Prototypes -----
static bool FetchDeviceInfo(IMFActivate **wmfDeviceList, int device_index, WMFDeviceInfo &outDeviceInfo);
static HRESULT BuildCaptureFormatList(IMFMediaSource *pSource, WMFDeviceInfo &deviceInfo);
static WMFDeviceFormatInfo ParseWMFFormatType(int mediaTypeIndex, IMFMediaType *pWMFMediaType);
static HRESULT ParseWMFFormatTypeAttributeByIndex(IMFAttributes *pAttr, DWORD index, WMFDeviceFormatInfo &outFormatType);
static HRESULT GetGUIDNameCopy(const GUID& guid, std::wstring &out_guidName);
static LPCWSTR GetGUIDNameConst(const GUID& guid);

// -- WMFCameraEnumerator -----
WMFCameraEnumerator::WMFCameraEnumerator()
    : DeviceEnumerator()
	, m_wmfDeviceList(new WMFDeviceList)
{
    m_deviceEnumeratorIndex= -1;
	m_currentDeviceIdentifier= "";

	if (m_wmfDeviceList->fetchDeviceList())
	{
		next();
	}
	else
	{
		m_deviceEnumeratorIndex= 0;
	}
}

WMFCameraEnumerator::~WMFCameraEnumerator()
{
	delete m_wmfDeviceList;
}

const char *WMFCameraEnumerator::getDevicePath() const
{
	return m_currentDeviceIdentifier.c_str();
}

eDeviceType WMFCameraEnumerator::getDeviceType() const
{
	VideoCapabilitiesConfigConstPtr config= getVideoCapabilities();

	return config != nullptr ? config->deviceType : eDeviceType::INVALID;
}

int WMFCameraEnumerator::getUsbVendorId() const
{
	return isValid() ? m_wmfDeviceList->deviceList[m_deviceEnumeratorIndex].usbVendorId : -1;
}

int WMFCameraEnumerator::getUsbProductId() const
{
	return isValid() ? m_wmfDeviceList->deviceList[m_deviceEnumeratorIndex].usbProductId : -1;
}

const char *WMFCameraEnumerator::getUniqueIdentifier() const
{
	return isValid() ? m_wmfDeviceList->deviceList[m_deviceEnumeratorIndex].uniqueIdentifier.c_str() : nullptr;
}

const WMFDeviceInfo *WMFCameraEnumerator::getDeviceInfo() const
{
	return isValid() ? &m_wmfDeviceList->deviceList[m_deviceEnumeratorIndex] : nullptr;
}

VideoCapabilitiesConfigConstPtr WMFCameraEnumerator::getVideoCapabilities() const
{
	return isValid() ? m_videoCapabilities : nullptr;
}

bool WMFCameraEnumerator::isValid() const
{
	return m_deviceEnumeratorIndex < (int)m_wmfDeviceList->deviceList.size();
}

bool WMFCameraEnumerator::next()
{
	bool bFoundValid= false;
	++m_deviceEnumeratorIndex;

	while (!bFoundValid && m_deviceEnumeratorIndex < (int)m_wmfDeviceList->deviceList.size())
	{
		const WMFDeviceInfo &deviceInfo= m_wmfDeviceList->deviceList[m_deviceEnumeratorIndex];
		const int vendorId= deviceInfo.usbVendorId;
		const int productId= deviceInfo.usbProductId;

		// First try to get video capability defaults from the supported_trackers config files
		m_videoCapabilities= s_videoCapabilitiesSet->getVideoSourceCapabilities(vendorId, productId);

		// If not found, convert the WMF device info to a VideoCapabilitiesConfig
		if (!m_videoCapabilities)
		{
			m_videoCapabilities= deviceInfo.convertToVideoCapabilites();
		}

		m_currentDeviceIdentifier= deviceInfo.deviceSymbolicLink;
		bFoundValid= true;
	}

	return bFoundValid;
}

// -- WMF Device Info -----
int WMFDeviceInfo::findBestDeviceFormatIndex(
	unsigned int w,
	unsigned int h,
	unsigned int frameRate,
	const char *buffer_format) const
{
	wchar_t wcs_buffer_format[256];
	if (!StringUtils::convertMbsToWcs(buffer_format, wcs_buffer_format, sizeof(wcs_buffer_format)))
	{
		return INVALID_DEVICE_FORMAT_INDEX;
	}

	int result_id= INVALID_DEVICE_FORMAT_INDEX;
	for (int attempt = 0; attempt < 2; ++attempt)
	{
		for (const WMFDeviceFormatInfo &info : deviceAvailableFormats)
		{
			unsigned int rounded_frame_rate= info.frame_rate_numerator / info.frame_rate_denominator;

			if ((w == UNSPECIFIED_CAMERA_WIDTH || info.width == w) && 
				(h == UNSPECIFIED_CAMERA_HEIGHT || info.height == h) && 
				info.sub_type_name == wcs_buffer_format && 
				(frameRate == UNSPECIFIED_CAMERA_FPS || rounded_frame_rate == frameRate))
			{
				result_id= info.device_format_index;
				break;
			}
		}

		if (result_id != INVALID_DEVICE_FORMAT_INDEX)
		{
			break;
		}
		else if (attempt == 0)
		{
			// Fallback to no FPS restriction on second pass
			frameRate= UNSPECIFIED_CAMERA_FPS;
		}
	}

	return result_id;
}

VideoCapabilitiesConfigConstPtr WMFDeviceInfo::convertToVideoCapabilites() const
{
	VideoCapabilitiesConfigPtr videoCapabilities= std::make_shared<VideoCapabilitiesConfig>();
	
	videoCapabilities->friendlyName= deviceFriendlyName;
	videoCapabilities->usbProductId= usbProductId;
	videoCapabilities->usbVendorId= usbVendorId;
	videoCapabilities->deviceType = eDeviceType::MonoVideoSource; // By default, assume mono video source

	// Fill in the supported video modes
	for (const WMFDeviceFormatInfo &info : deviceAvailableFormats)
	{
		const std::string subTypeName= StringUtils::convertWStringToUTF8String(info.sub_type_name);
		const float frameRate= (float)info.frame_rate_numerator / (float)info.frame_rate_denominator;
		char formatName[32];

		// USB device symbolic link
		if (sscanf_s(
			subTypeName.c_str(), "MFVideoFormat_%s",
			formatName, (unsigned)_countof(formatName)) == 1)
		{
			VideoModeConfig mode;
			mode.modeName = StringUtils::stringify(info.width, "x", info.height, "(", frameRate, "FPS)");
			mode.frameRate = frameRate;
			mode.isFrameMirrored = false;
			mode.isBufferMirrored = false;
			mode.bufferPixelWidth = info.width;
			mode.bufferPixelHeight = info.height;
			mode.bufferFormat = formatName;
			mode.frameSections.push_back({0, 0}); // Only one frame section for mono video sources

			// Set default camera intrinsic properties
			createDefautMonoIntrinsics(
				info.width,
				info.height,
				mode.intrinsics.makeMonoIntrinsics());

			videoCapabilities->supportedModes.push_back(mode);
		}
	}

	return videoCapabilities;
}

// -- WMF Device List -----
bool WMFDeviceList::fetchDeviceList()
{
	UINT32 wmfDeviceCount= 0;
	IMFAttributes *wmfAttributeTable= NULL;
	IMFActivate **wmfDeviceList= NULL;

    HRESULT hr = MFCreateAttributes(&wmfAttributeTable, 1);
   
	if (SUCCEEDED(hr))
    {
        hr = wmfAttributeTable->SetGUID(
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    }

	if (SUCCEEDED(hr))
    {
		hr = MFEnumDeviceSources(wmfAttributeTable, &wmfDeviceList, &wmfDeviceCount);
	}

	if (SUCCEEDED(hr))
	{
		deviceList.clear();
		for (int wmfDeviceIndex= 0; wmfDeviceIndex < (int)wmfDeviceCount; ++wmfDeviceIndex)
		{
			WMFDeviceInfo device;

			if (FetchDeviceInfo(wmfDeviceList, wmfDeviceIndex, device))
			{
				deviceList.push_back(device);
			}
		}
	}

	if (wmfDeviceList != nullptr)
	{
		for(UINT32 i = 0; i < wmfDeviceCount; i++)
		{
			MemoryUtils::safeRelease(&wmfDeviceList[i]);
		}	

		MemoryUtils::safeReleaseAllCount(wmfDeviceList);
	}

	MemoryUtils::safeReleaseAllCount(&wmfAttributeTable);

	return SUCCEEDED(hr);
}

// -- WMF Device -----
static bool FetchDeviceInfo(IMFActivate **wmfDeviceList, int wmfDeviceIndex, WMFDeviceInfo &deviceInfo)
{
	IMFActivate *pActivate= wmfDeviceList[wmfDeviceIndex];

	HRESULT hr;

	deviceInfo.wmfDeviceIndex= wmfDeviceIndex;

	{
		wchar_t* wszDeviceFriendlyName= nullptr;

		hr = pActivate->GetAllocatedString(
				MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
				&wszDeviceFriendlyName,
				NULL);

		if (SUCCEEDED(hr))
		{
			const std::wstring wsDeviceFriendlyName= wszDeviceFriendlyName;
			deviceInfo.deviceFriendlyName= StringUtils::convertWStringToUTF8String(wsDeviceFriendlyName);

			CoTaskMemFree(wszDeviceFriendlyName);
		}
	}

	if (SUCCEEDED(hr))
	{
		wchar_t *wszDeviceSymbolicLink= nullptr;

		hr = pActivate->GetAllocatedString(
				MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
				&wszDeviceSymbolicLink,
				NULL);

		if (SUCCEEDED(hr))
		{
			char szDeviceSymbolicLink[512];
			wcstombs(szDeviceSymbolicLink, wszDeviceSymbolicLink, (unsigned)_countof(szDeviceSymbolicLink));  
			deviceInfo.deviceSymbolicLink= szDeviceSymbolicLink;

			int symLinkIndex = -1;
			char symLinkGuid[36 + 1];

			// USB device symbolic link
			if (sscanf_s(
				szDeviceSymbolicLink, "\\\\?\\usb#vid_%x&pid_%x#",
				&deviceInfo.usbVendorId, &deviceInfo.usbProductId) == 2)
			{
				BYTE hash[8];

				hr = HashData(
					(BYTE*)deviceInfo.deviceSymbolicLink.c_str(),
					(DWORD)deviceInfo.deviceSymbolicLink.length(),
					hash, 8);

				if (SUCCEEDED(hr))
				{
					char hash_string[32];

					sprintf_s(hash_string, sizeof(hash_string), "%x%x%x%x%x%x%x%x",
							  hash[0], hash[1], hash[2], hash[3],
							  hash[4], hash[5], hash[6], hash[7]);

					deviceInfo.uniqueIdentifier = hash_string;
				}
			}
			// Streamed camera symbolic link
			else if (sscanf_s(
				szDeviceSymbolicLink, "\\\\?\\root#media#%x#{%36s}\\global",
				&symLinkIndex, symLinkGuid, (unsigned)_countof(symLinkGuid)) == 2)
			{
				deviceInfo.usbProductId= -1;
				deviceInfo.usbVendorId= -1;
				deviceInfo.uniqueIdentifier= symLinkGuid;

				hr= S_OK;
			}
			// Streamed camera symbolic link
			else if (sscanf_s(
				szDeviceSymbolicLink, "\\\\?\\root#image#%x#{%36s}\\global",
				&symLinkIndex, symLinkGuid, (unsigned)_countof(symLinkGuid)) == 2)
			{
				deviceInfo.usbProductId = -1;
				deviceInfo.usbVendorId = -1;
				deviceInfo.uniqueIdentifier = symLinkGuid;

				hr = S_OK;
			}
			else
			{
				hr= E_FAIL;
			}

			CoTaskMemFree(wszDeviceSymbolicLink);
		}
	}

	// Compute a unique identifier from using an 8-byte hash of the device symbolic path
	// This hash encapsulates VID, PID, REV and driver GUID
	if (SUCCEEDED(hr))
	{

	}

	IMFMediaSource *pSource = NULL;
	if (SUCCEEDED(hr))
	{
		hr = pActivate->ActivateObject(
			__uuidof(IMFMediaSource),
			(void**)&pSource);
	}

	if (SUCCEEDED(hr))
	{
		hr= BuildCaptureFormatList(pSource, deviceInfo);
	}

	MemoryUtils::safeRelease(&pSource);

	return SUCCEEDED(hr);
}

static HRESULT BuildCaptureFormatList(IMFMediaSource *pSource, WMFDeviceInfo &deviceInfo)
{
	IMFPresentationDescriptor *pPD = NULL;
    HRESULT hr = pSource->CreatePresentationDescriptor(&pPD);

    BOOL fSelected;
    IMFStreamDescriptor *pSD = NULL;
    if (SUCCEEDED(hr))
    {
		hr = pPD->GetStreamDescriptorByIndex(0, &fSelected, &pSD);
    }

    IMFMediaTypeHandler *pHandler = NULL;
    if (SUCCEEDED(hr))
    {
	    hr = pSD->GetMediaTypeHandler(&pHandler);
    }

    DWORD cTypes = 0;
    if (SUCCEEDED(hr))
    {
	    hr = pHandler->GetMediaTypeCount(&cTypes);
    }

    for (DWORD i = 0; i < cTypes; i++)
    {
	    IMFMediaType *pWMFMediaType = nullptr;

        hr = pHandler->GetMediaTypeByIndex(i, &pWMFMediaType);

		if (SUCCEEDED(hr))
		{
			WMFDeviceFormatInfo mediaTypeInfo = ParseWMFFormatType(i, pWMFMediaType);

			deviceInfo.deviceAvailableFormats.push_back(mediaTypeInfo);
		
			MemoryUtils::safeReleaseAllCount(&pWMFMediaType);
		}
		else
        {
			MemoryUtils::safeReleaseAllCount(&pWMFMediaType);
            break;
        }		
    }

    MemoryUtils::safeReleaseAllCount(&pPD);
    MemoryUtils::safeRelease(&pSD);
    MemoryUtils::safeRelease(&pHandler);

    return hr;
}

// -- WMF Format Table Helper Functions -----
static WMFDeviceFormatInfo ParseWMFFormatType(int mediaTypeIndex, IMFMediaType *pWMFMediaType)
{
	UINT32 count = 0;
	HRESULT hr = S_OK;
	WMFDeviceFormatInfo result;

	memset(&result, 0, sizeof(WMFDeviceFormatInfo));
	result.device_format_index= mediaTypeIndex;

	hr = pWMFMediaType->LockStore();
	if (FAILED(hr))
    {
        return result;
    }

    hr = pWMFMediaType->GetCount(&count);
    if (FAILED(hr))
    {
        return result;
    }
	
    for (UINT32 i = 0; i < count; i++)
    {
        hr = ParseWMFFormatTypeAttributeByIndex(pWMFMediaType, i, result);

        if (FAILED(hr))
        {
            break;
        }
    }

	hr = pWMFMediaType->UnlockStore();
	if (FAILED(hr))
    {
        return result;
    }

    return result;
}

static HRESULT ParseWMFFormatTypeAttributeByIndex(IMFAttributes *pAttr, DWORD index, WMFDeviceFormatInfo &outMediaType)
{
    PROPVARIANT var;
    PropVariantInit(&var);

    GUID guid = { 0 };
    HRESULT hr = pAttr->GetItemByIndex(index, &guid, &var);

    std::wstring guidName;
    if (SUCCEEDED(hr))
    {
        hr = GetGUIDNameCopy(guid, guidName);
    }

    if (SUCCEEDED(hr))
    {
		if (guid == MF_MT_FRAME_SIZE)
		{
			UINT32 uHigh = 0, uLow = 0;

			Unpack2UINT32AsUINT64(var.uhVal.QuadPart, &uHigh, &uLow);
			outMediaType.width = uHigh;
			outMediaType.height = uLow;
			outMediaType.frame_size = outMediaType.width * outMediaType.height;
		}
		else if (guid == MF_MT_FRAME_RATE)
		{
			UINT32 uHigh = 0, uLow = 0;

			Unpack2UINT32AsUINT64(var.uhVal.QuadPart, &uHigh, &uLow);
			outMediaType.frame_rate_numerator = uHigh;
			outMediaType.frame_rate_denominator = uLow;
		}
		else if (guid == MF_MT_FRAME_RATE_RANGE_MAX)
		{
			UINT32 uHigh = 0, uLow = 0;

			Unpack2UINT32AsUINT64(var.uhVal.QuadPart, &uHigh, &uLow);
			outMediaType.frame_rate_range_max_numerator = uHigh;
			outMediaType.frame_rate_range_max_denominator = uLow;
		}
		else if (guid == MF_MT_FRAME_RATE_RANGE_MIN)
		{
			UINT32 uHigh = 0, uLow = 0;

			Unpack2UINT32AsUINT64(var.uhVal.QuadPart, &uHigh, &uLow);
			outMediaType.frame_rate_range_min_numerator = uHigh;
			outMediaType.frame_rate_range_min_denominator = uLow;
		}
		else if (guid == MF_MT_PIXEL_ASPECT_RATIO)
		{
			UINT32 uHigh = 0, uLow = 0;

			Unpack2UINT32AsUINT64(var.uhVal.QuadPart, &uHigh, &uLow);
			outMediaType.pixel_aspect_ratio_horizontal = uHigh;
			outMediaType.pixel_aspect_ratio_vertical = uLow;
		}	     
		else
		{
			switch (var.vt)
			{
			case VT_UI4:
				{
					if(guid == MF_MT_YUV_MATRIX) 
						outMediaType.yuv_matrix = var.ulVal;
					else if(guid == MF_MT_VIDEO_LIGHTING) 
						outMediaType.video_lighting = var.ulVal;
					else if(guid == MF_MT_DEFAULT_STRIDE) 
						outMediaType.default_stride = (int)var.ulVal;
					else if(guid == MF_MT_VIDEO_CHROMA_SITING) 
						outMediaType.video_chroma_siting = var.ulVal;
					else if(guid == MF_MT_VIDEO_NOMINAL_RANGE) 
						outMediaType.video_nominal_range = var.ulVal;
					else if(guid == MF_MT_ALL_SAMPLES_INDEPENDENT) 
						outMediaType.all_samples_independant = var.ulVal;
					else if(guid == MF_MT_FIXED_SIZE_SAMPLES) 
						outMediaType.fixed_size_samples = var.ulVal;
					else if(guid == MF_MT_SAMPLE_SIZE) 
						outMediaType.sample_size = var.ulVal;
					else if(guid == MF_MT_VIDEO_PRIMARIES) 
						outMediaType.video_primaries = var.ulVal;
					else if(guid == MF_MT_INTERLACE_MODE) 
						outMediaType.interlace_mode = var.ulVal;
				} break;

			case VT_CLSID:
				{
					if(guid == MF_MT_AM_FORMAT_TYPE)
					{
						hr = GetGUIDNameCopy(*var.puuid, outMediaType.am_format_type_name);
					}

					if(guid == MF_MT_MAJOR_TYPE)
					{
						hr = GetGUIDNameCopy(*var.puuid, outMediaType.major_type_name);
					}

					if(guid == MF_MT_SUBTYPE)
					{
						hr = GetGUIDNameCopy(*var.puuid, outMediaType.sub_type_name);

					}
				} break;

			case VT_UI8:
			case VT_R8:
			case VT_LPWSTR:
			case VT_VECTOR | VT_UI1:
			case VT_UNKNOWN:
				break;

			default:
				break;
			}
		}
    }

    PropVariantClear(&var);
    return hr;
}

static HRESULT GetGUIDNameCopy(const GUID& guid, std::wstring &out_guidName)
{
	HRESULT hr= S_OK;
    LPCWSTR pcwsz = GetGUIDNameConst(guid);

    if (pcwsz)
    {
		out_guidName= pcwsz;
    }
    else
    {
	    WCHAR *pName = NULL;
        hr = StringFromCLSID(guid, &pName);

		if (SUCCEEDED(hr))
		{
			out_guidName= pName;
			CoTaskMemFree(pName);
		}
    }

	return hr;
}

#ifndef IF_EQUAL_RETURN
#define IF_EQUAL_RETURN(param, val) if(val == param) return L#val
#endif

static LPCWSTR GetGUIDNameConst(const GUID& guid)
{
    IF_EQUAL_RETURN(guid, MF_MT_MAJOR_TYPE);
    IF_EQUAL_RETURN(guid, MF_MT_MAJOR_TYPE);
    IF_EQUAL_RETURN(guid, MF_MT_SUBTYPE);
    IF_EQUAL_RETURN(guid, MF_MT_ALL_SAMPLES_INDEPENDENT);
    IF_EQUAL_RETURN(guid, MF_MT_FIXED_SIZE_SAMPLES);
    IF_EQUAL_RETURN(guid, MF_MT_COMPRESSED);
    IF_EQUAL_RETURN(guid, MF_MT_SAMPLE_SIZE);
    IF_EQUAL_RETURN(guid, MF_MT_WRAPPED_TYPE);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_NUM_CHANNELS);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_SAMPLES_PER_SECOND);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_FLOAT_SAMPLES_PER_SECOND);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_AVG_BYTES_PER_SECOND);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_BLOCK_ALIGNMENT);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_BITS_PER_SAMPLE);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_VALID_BITS_PER_SAMPLE);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_SAMPLES_PER_BLOCK);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_CHANNEL_MASK);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_FOLDDOWN_MATRIX);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_PEAKREF);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_PEAKTARGET);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_AVGREF);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_WMADRC_AVGTARGET);
    IF_EQUAL_RETURN(guid, MF_MT_AUDIO_PREFER_WAVEFORMATEX);
    IF_EQUAL_RETURN(guid, MF_MT_AAC_PAYLOAD_TYPE);
    IF_EQUAL_RETURN(guid, MF_MT_AAC_AUDIO_PROFILE_LEVEL_INDICATION);
    IF_EQUAL_RETURN(guid, MF_MT_FRAME_SIZE);
    IF_EQUAL_RETURN(guid, MF_MT_FRAME_RATE);
    IF_EQUAL_RETURN(guid, MF_MT_FRAME_RATE_RANGE_MAX);
    IF_EQUAL_RETURN(guid, MF_MT_FRAME_RATE_RANGE_MIN);
    IF_EQUAL_RETURN(guid, MF_MT_PIXEL_ASPECT_RATIO);
    IF_EQUAL_RETURN(guid, MF_MT_DRM_FLAGS);
    IF_EQUAL_RETURN(guid, MF_MT_PAD_CONTROL_FLAGS);
    IF_EQUAL_RETURN(guid, MF_MT_SOURCE_CONTENT_HINT);
    IF_EQUAL_RETURN(guid, MF_MT_VIDEO_CHROMA_SITING);
    IF_EQUAL_RETURN(guid, MF_MT_INTERLACE_MODE);
    IF_EQUAL_RETURN(guid, MF_MT_TRANSFER_FUNCTION);
    IF_EQUAL_RETURN(guid, MF_MT_VIDEO_PRIMARIES);
    IF_EQUAL_RETURN(guid, MF_MT_CUSTOM_VIDEO_PRIMARIES);
    IF_EQUAL_RETURN(guid, MF_MT_YUV_MATRIX);
    IF_EQUAL_RETURN(guid, MF_MT_VIDEO_LIGHTING);
    IF_EQUAL_RETURN(guid, MF_MT_VIDEO_NOMINAL_RANGE);
    IF_EQUAL_RETURN(guid, MF_MT_GEOMETRIC_APERTURE);
    IF_EQUAL_RETURN(guid, MF_MT_MINIMUM_DISPLAY_APERTURE);
    IF_EQUAL_RETURN(guid, MF_MT_PAN_SCAN_APERTURE);
    IF_EQUAL_RETURN(guid, MF_MT_PAN_SCAN_ENABLED);
    IF_EQUAL_RETURN(guid, MF_MT_AVG_BITRATE);
    IF_EQUAL_RETURN(guid, MF_MT_AVG_BIT_ERROR_RATE);
    IF_EQUAL_RETURN(guid, MF_MT_MAX_KEYFRAME_SPACING);
    IF_EQUAL_RETURN(guid, MF_MT_DEFAULT_STRIDE);
    IF_EQUAL_RETURN(guid, MF_MT_PALETTE);
    IF_EQUAL_RETURN(guid, MF_MT_USER_DATA);
    IF_EQUAL_RETURN(guid, MF_MT_AM_FORMAT_TYPE);
    IF_EQUAL_RETURN(guid, MF_MT_MPEG_START_TIME_CODE);
    IF_EQUAL_RETURN(guid, MF_MT_MPEG2_PROFILE);
    IF_EQUAL_RETURN(guid, MF_MT_MPEG2_LEVEL);
    IF_EQUAL_RETURN(guid, MF_MT_MPEG2_FLAGS);
    IF_EQUAL_RETURN(guid, MF_MT_MPEG_SEQUENCE_HEADER);
    IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_SRC_PACK_0);
    IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_CTRL_PACK_0);
    IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_SRC_PACK_1);
    IF_EQUAL_RETURN(guid, MF_MT_DV_AAUX_CTRL_PACK_1);
    IF_EQUAL_RETURN(guid, MF_MT_DV_VAUX_SRC_PACK);
    IF_EQUAL_RETURN(guid, MF_MT_DV_VAUX_CTRL_PACK);
    IF_EQUAL_RETURN(guid, MF_MT_ARBITRARY_HEADER);
    IF_EQUAL_RETURN(guid, MF_MT_ARBITRARY_FORMAT);
    IF_EQUAL_RETURN(guid, MF_MT_IMAGE_LOSS_TOLERANT); 
    IF_EQUAL_RETURN(guid, MF_MT_MPEG4_SAMPLE_DESCRIPTION);
    IF_EQUAL_RETURN(guid, MF_MT_MPEG4_CURRENT_SAMPLE_ENTRY);
    IF_EQUAL_RETURN(guid, MF_MT_ORIGINAL_4CC); 
    IF_EQUAL_RETURN(guid, MF_MT_ORIGINAL_WAVE_FORMAT_TAG);
    
    // Media types
    IF_EQUAL_RETURN(guid, MFMediaType_Audio);
    IF_EQUAL_RETURN(guid, MFMediaType_Video);
    IF_EQUAL_RETURN(guid, MFMediaType_Protected);
    IF_EQUAL_RETURN(guid, MFMediaType_SAMI);
    IF_EQUAL_RETURN(guid, MFMediaType_Script);
    IF_EQUAL_RETURN(guid, MFMediaType_Image);
    IF_EQUAL_RETURN(guid, MFMediaType_HTML);
    IF_EQUAL_RETURN(guid, MFMediaType_Binary);
    IF_EQUAL_RETURN(guid, MFMediaType_FileTransfer);

    IF_EQUAL_RETURN(guid, MFVideoFormat_AI44); //     FCC('AI44')
    IF_EQUAL_RETURN(guid, MFVideoFormat_ARGB32); //   D3DFMT_A8R8G8B8 
    IF_EQUAL_RETURN(guid, MFVideoFormat_AYUV); //     FCC('AYUV')
    IF_EQUAL_RETURN(guid, MFVideoFormat_DV25); //     FCC('dv25')
    IF_EQUAL_RETURN(guid, MFVideoFormat_DV50); //     FCC('dv50')
    IF_EQUAL_RETURN(guid, MFVideoFormat_DVH1); //     FCC('dvh1')
    IF_EQUAL_RETURN(guid, MFVideoFormat_DVSD); //     FCC('dvsd')
    IF_EQUAL_RETURN(guid, MFVideoFormat_DVSL); //     FCC('dvsl')
    IF_EQUAL_RETURN(guid, MFVideoFormat_H264); //     FCC('H264')
    IF_EQUAL_RETURN(guid, MFVideoFormat_I420); //     FCC('I420')
    IF_EQUAL_RETURN(guid, MFVideoFormat_IYUV); //     FCC('IYUV')
    IF_EQUAL_RETURN(guid, MFVideoFormat_M4S2); //     FCC('M4S2')
    IF_EQUAL_RETURN(guid, MFVideoFormat_MJPG);
    IF_EQUAL_RETURN(guid, MFVideoFormat_MP43); //     FCC('MP43')
    IF_EQUAL_RETURN(guid, MFVideoFormat_MP4S); //     FCC('MP4S')
    IF_EQUAL_RETURN(guid, MFVideoFormat_MP4V); //     FCC('MP4V')
    IF_EQUAL_RETURN(guid, MFVideoFormat_MPG1); //     FCC('MPG1')
    IF_EQUAL_RETURN(guid, MFVideoFormat_MSS1); //     FCC('MSS1')
    IF_EQUAL_RETURN(guid, MFVideoFormat_MSS2); //     FCC('MSS2')
    IF_EQUAL_RETURN(guid, MFVideoFormat_NV11); //     FCC('NV11')
    IF_EQUAL_RETURN(guid, MFVideoFormat_NV12); //     FCC('NV12')
    IF_EQUAL_RETURN(guid, MFVideoFormat_P010); //     FCC('P010')
    IF_EQUAL_RETURN(guid, MFVideoFormat_P016); //     FCC('P016')
    IF_EQUAL_RETURN(guid, MFVideoFormat_P210); //     FCC('P210')
    IF_EQUAL_RETURN(guid, MFVideoFormat_P216); //     FCC('P216')
    IF_EQUAL_RETURN(guid, MFVideoFormat_RGB24); //    D3DFMT_R8G8B8 
    IF_EQUAL_RETURN(guid, MFVideoFormat_RGB32); //    D3DFMT_X8R8G8B8 
    IF_EQUAL_RETURN(guid, MFVideoFormat_RGB555); //   D3DFMT_X1R5G5B5 
    IF_EQUAL_RETURN(guid, MFVideoFormat_RGB565); //   D3DFMT_R5G6B5 
    IF_EQUAL_RETURN(guid, MFVideoFormat_RGB8);
    IF_EQUAL_RETURN(guid, MFVideoFormat_UYVY); //     FCC('UYVY')
    IF_EQUAL_RETURN(guid, MFVideoFormat_v210); //     FCC('v210')
    IF_EQUAL_RETURN(guid, MFVideoFormat_v410); //     FCC('v410')
    IF_EQUAL_RETURN(guid, MFVideoFormat_WMV1); //     FCC('WMV1')
    IF_EQUAL_RETURN(guid, MFVideoFormat_WMV2); //     FCC('WMV2')
    IF_EQUAL_RETURN(guid, MFVideoFormat_WMV3); //     FCC('WMV3')
    IF_EQUAL_RETURN(guid, MFVideoFormat_WVC1); //     FCC('WVC1')
    IF_EQUAL_RETURN(guid, MFVideoFormat_Y210); //     FCC('Y210')
    IF_EQUAL_RETURN(guid, MFVideoFormat_Y216); //     FCC('Y216')
    IF_EQUAL_RETURN(guid, MFVideoFormat_Y410); //     FCC('Y410')
    IF_EQUAL_RETURN(guid, MFVideoFormat_Y416); //     FCC('Y416')
    IF_EQUAL_RETURN(guid, MFVideoFormat_Y41P);
    IF_EQUAL_RETURN(guid, MFVideoFormat_Y41T);
    IF_EQUAL_RETURN(guid, MFVideoFormat_YUY2); //     FCC('YUY2')
    IF_EQUAL_RETURN(guid, MFVideoFormat_YV12); //     FCC('YV12')
    IF_EQUAL_RETURN(guid, MFVideoFormat_YVYU);

    IF_EQUAL_RETURN(guid, MFAudioFormat_PCM); //              WAVE_FORMAT_PCM 
    IF_EQUAL_RETURN(guid, MFAudioFormat_Float); //            WAVE_FORMAT_IEEE_FLOAT 
    IF_EQUAL_RETURN(guid, MFAudioFormat_DTS); //              WAVE_FORMAT_DTS 
    IF_EQUAL_RETURN(guid, MFAudioFormat_Dolby_AC3_SPDIF); //  WAVE_FORMAT_DOLBY_AC3_SPDIF 
    IF_EQUAL_RETURN(guid, MFAudioFormat_DRM); //              WAVE_FORMAT_DRM 
    IF_EQUAL_RETURN(guid, MFAudioFormat_WMAudioV8); //        WAVE_FORMAT_WMAUDIO2 
    IF_EQUAL_RETURN(guid, MFAudioFormat_WMAudioV9); //        WAVE_FORMAT_WMAUDIO3 
    IF_EQUAL_RETURN(guid, MFAudioFormat_WMAudio_Lossless); // WAVE_FORMAT_WMAUDIO_LOSSLESS 
    IF_EQUAL_RETURN(guid, MFAudioFormat_WMASPDIF); //         WAVE_FORMAT_WMASPDIF 
    IF_EQUAL_RETURN(guid, MFAudioFormat_MSP1); //             WAVE_FORMAT_WMAVOICE9 
    IF_EQUAL_RETURN(guid, MFAudioFormat_MP3); //              WAVE_FORMAT_MPEGLAYER3 
    IF_EQUAL_RETURN(guid, MFAudioFormat_MPEG); //             WAVE_FORMAT_MPEG 
    IF_EQUAL_RETURN(guid, MFAudioFormat_AAC); //              WAVE_FORMAT_MPEG_HEAAC 
    IF_EQUAL_RETURN(guid, MFAudioFormat_ADTS); //             WAVE_FORMAT_MPEG_ADTS_AAC 

    return NULL;
}