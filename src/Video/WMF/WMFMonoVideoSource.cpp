// -- includes -----
#include "WMFMonoVideoSource.h"
#include "Logger.h"
#include "StringUtils.h"
#include "VideoDeviceEnumerator.h"
#include "VideoCapabilitiesConfig.h"
#include "WMFCameraEnumerator.h"
#include "WorkerThread.h"
#include "WMFVideo.h"

#include "glm/gtc/type_ptr.hpp"

#ifdef _MSC_VER
    #pragma warning (disable: 4996) // 'This function or variable may be unsafe': strncpy
#endif

// -- constants -----
#define VIRTUAL_STEREO_STATE_BUFFER_MAX		16

// -- private definitions -----
// -- public methods
// -- WMF Stereo Tracker Config
const int WMFMonoVideoConfig::CONFIG_VERSION = 1;

WMFMonoVideoConfig::WMFMonoVideoConfig(const std::string &fnamebase)
    : WMFVideoConfig(fnamebase)
{
	areIntrinsicsUserCalibrated = false;
	tracker_intrinsics.pixel_width= 640;
	tracker_intrinsics.pixel_height= 480;
    tracker_intrinsics.hfov= 60.0; // degrees
    tracker_intrinsics.vfov= 45.0; // degrees
    tracker_intrinsics.znear= 0.01; // meters
    tracker_intrinsics.zfar= 200.0; // meters
    tracker_intrinsics.camera_matrix= {
		554.2563, 0, 320.0, // f_x, 0, c_x
		0, 554.2563, 240.0, // 0, f_y, c_y
		0.0, 0.0, 1.0 
	};
    tracker_intrinsics.distortion_coefficients= {
        -0.10771770030260086, 0.1213262677192688, 0.04875476285815239, // K1, K2, K3
        0.00091733073350042105, 0.00010589254816295579};  // P1, P2
};

configuru::Config WMFMonoVideoConfig::writeToJSON()
{
	configuru::Config pt = WMFVideoConfig::writeToJSON();

    pt["is_valid"]= is_valid;
    pt["version"]= WMFMonoVideoConfig::CONFIG_VERSION;
    pt["max_poll_failure_count"]= max_poll_failure_count;

	pt["are_intrinsics_user_calibrated"]= areIntrinsicsUserCalibrated;
	writeMonoTrackerIntrinsics(pt, tracker_intrinsics);

    return pt;
}

void WMFMonoVideoConfig::readFromJSON(const configuru::Config &pt)
{
    int config_version = pt.get_or<int>("version", 0);
    if (config_version == WMFMonoVideoConfig::CONFIG_VERSION)
    {
		WMFVideoConfig::readFromJSON(pt);

		areIntrinsicsUserCalibrated= pt.get_or<bool>("are_intrinsics_user_calibrated", areIntrinsicsUserCalibrated);
		readMonoTrackerIntrinsics(pt, tracker_intrinsics);
    }
    else
    {
        MIKAN_LOG_WARNING("WMFMonoTrackerConfig") <<
            "Config version " << config_version << " does not match expected version " <<
            WMFMonoVideoConfig::CONFIG_VERSION << ", Using defaults.";
    }
}

// -- WMFMonoTracker
WMFMonoVideoSource::WMFMonoVideoSource()
    : m_cfg()
	, m_videoDevice(nullptr)
    , m_DriverType(WMFMonoVideoSource::WindowsMediaFramework)
{
}

WMFMonoVideoSource::~WMFMonoVideoSource()
{
    if (getIsOpen())
    {
        MIKAN_LOG_ERROR("~WMFMonoTracker") << "Tracker deleted without calling close() first!";
    }
}

bool WMFMonoVideoSource::open() // Opens the first HID device for the tracker
{
    VideoDeviceEnumerator enumerator;
    bool success = false;

    // Skip over everything that isn't a WMF camera
    while (enumerator.isValid() && enumerator.getDeviceType() != eDeviceType::MonoVideoSource)
    {
        enumerator.next();
    }

    if (enumerator.isValid())
    {
        success = open(&enumerator);
    }

    return success;
}

// -- IDeviceInterface
bool WMFMonoVideoSource::matchesDeviceEnumerator(const DeviceEnumerator *enumerator) const
{
    // Down-cast the enumerator so we can use the correct get_path.
    const VideoDeviceEnumerator* pEnum = static_cast<const VideoDeviceEnumerator*>(enumerator);

    bool matches = false;

    if (pEnum->getDeviceType() == eDeviceType::MonoVideoSource)
    {
        std::string enumerator_path = pEnum->getDevicePath();

        matches = (enumerator_path == m_device_identifier);
    }

    return matches;
}

bool WMFMonoVideoSource::open(const DeviceEnumerator *enumerator)
{
    const VideoDeviceEnumerator *cameraEnumerator = static_cast<const VideoDeviceEnumerator*>(enumerator);
    const char *curDevPath = cameraEnumerator->getDevicePath();
	const int cameraIndex = cameraEnumerator->getCameraIndex();

    bool bSuccess = true;
    
    if (getIsOpen())
    {
        MIKAN_LOG_WARNING("WMFMonoTracker::open") << "WMFMonoTracker(" << curDevPath << ") already open. Ignoring request.";
    }
    else
    {
		const WMFCameraEnumerator *wmf_enumerator= cameraEnumerator->getWMFCameraEnumerator();
		const char *unique_id= wmf_enumerator->getUniqueIdentifier();

        MIKAN_LOG_INFO("WMFMonoTracker::open") << "Opening WMFMonoTracker(" << curDevPath << ", camera_index=" << cameraIndex << ")";

		// Remember the path to this camera
        m_device_identifier = curDevPath;

		// Build a config file name from the unique id
		char config_name[256];
        StringUtils::formatString(config_name, sizeof(config_name), "WMFMonoCamera_%s", unique_id);

        // Load the config file for the tracker
        m_cfg = std::make_shared<WMFMonoVideoConfig>(config_name);
        m_cfg->load();

		// Fetch the camera capabilities
		m_capabilities= wmf_enumerator->getVideoCapabilities();
        if (m_capabilities != nullptr)
        {
		    // If no mode is specified, then default to the first mode
			bool bWasModeUnset= false;
		    if (m_cfg->current_mode == "")
		    {
			    m_cfg->current_mode= m_capabilities->supportedModes[0].modeName;
				bWasModeUnset= true;
		    }

		    // Find the camera mode by name
		    m_currentMode= m_capabilities->findVideoMode(m_cfg->current_mode);
		    if (m_currentMode != nullptr)
		    {
			    // Copy the tracker intrinsics over from the capabilities
				// if there is no user calibration set
				if (bWasModeUnset || !m_cfg->areIntrinsicsUserCalibrated)
				{
					m_cfg->areIntrinsicsUserCalibrated= false;
					m_cfg->tracker_intrinsics = m_currentMode->intrinsics.intrinsics.mono;
				}

			    // Attempt to find a compatible WMF video format
			    std::string mfvideoformat= std::string("MFVideoFormat_")+m_currentMode->bufferFormat;
			    int desiredFormatIndex= 
				    wmf_enumerator->getDeviceInfo()->findBestDeviceFormatIndex(
					    (unsigned int)m_currentMode->bufferPixelWidth,
					    (unsigned int)m_currentMode->bufferPixelHeight,
					    (unsigned int)m_currentMode->frameRate,
					    mfvideoformat.c_str());

			    if (desiredFormatIndex != INVALID_DEVICE_FORMAT_INDEX)
			    {
				    m_videoDevice = new WMFVideoDevice(*wmf_enumerator->getDeviceInfo());

				    if (m_videoDevice->open(desiredFormatIndex, m_cfg, m_listener))
				    {
					    bSuccess = true;
				    }
			    }
		    }

		    // Save the config back out again in case defaults changed
            m_cfg->save();
        }
        else
        {
            MIKAN_LOG_ERROR("WMFMonoVideoSource::open") << "Unable to WMFMonoTracker(" << curDevPath << "). Unable to get tracker capabilities.";
            bSuccess= false;
        }
    }
    
    if (!bSuccess)
    {
        close();
    }

    return bSuccess;
}

bool WMFMonoVideoSource::getIsOpen() const
{
    return m_videoDevice != nullptr && m_videoDevice->getIsOpen();
}


void WMFMonoVideoSource::close()
{
	if (m_videoDevice != nullptr)
	{
		delete m_videoDevice;
		m_videoDevice= nullptr;
	}
}

bool WMFMonoVideoSource::startVideoStream()
{
	if (getIsOpen())
	{
		return m_videoDevice->startVideoStream();
	}

	return false;
}

bool WMFMonoVideoSource::getIsVideoStreaming() const
{
	return getIsOpen() && m_videoDevice->getIsVideoStreaming();
}

void WMFMonoVideoSource::stopVideoStream()
{
	if (getIsOpen())
	{
		m_videoDevice->stopVideoStream();
	}
}

eDeviceType WMFMonoVideoSource::getDeviceType() const
{
    return eDeviceType::MonoVideoSource;
}

IVideoSourceInterface::eDriverType WMFMonoVideoSource::getDriverType() const
{
    return IVideoSourceInterface::eDriverType::WindowsMediaFramework;
}

std::string WMFMonoVideoSource::getFriendlyName() const
{
	return m_videoDevice->m_deviceInfo.deviceFriendlyName;
}

std::string WMFMonoVideoSource::getUSBDevicePath() const
{
    return m_device_identifier;
}

bool WMFMonoVideoSource::getVideoFrameDimensions(
    int *out_width,
    int *out_height,
    int *out_stride) const
{
    bool bSuccess = true;

    if (out_width != nullptr)
    {
        int width = (int)m_currentMode->intrinsics.intrinsics.mono.pixel_width;

        if (out_stride != nullptr)
        {
            // Assume 3 bytes per pixel?
            *out_stride = 3 * width;
        }

        *out_width = width;
    }

    if (out_height != nullptr)
    {
        int height = (int)m_currentMode->intrinsics.intrinsics.mono.pixel_height;

        *out_height = height;
    }

    return bSuccess;
}

bool WMFMonoVideoSource::getIsFrameMirrored() const
{ 
	return m_currentMode->isFrameMirrored; 
}

bool WMFMonoVideoSource::getIsBufferMirrored() const
{ 
	return false; // Only ever true for stereo video feeds
}

void WMFMonoVideoSource::loadSettings()
{
	const VideoPropertyConstraint *constraints= m_videoDevice->getVideoPropertyConstraints();

    m_cfg->load();

	for (int prop_index = 0; prop_index < (int)VideoPropertyType::COUNT; ++prop_index)
	{
		const VideoPropertyType prop_type = (VideoPropertyType)prop_index;
		const VideoPropertyConstraint &constraint= constraints[prop_index];

		if (constraint.is_supported)
		{
			int currentValue= getVideoProperty(prop_type);
			int desiredValue= m_cfg->video_properties[prop_index];

			if (desiredValue != currentValue)
			{
				bool bUpdateConfig= false;

				if (desiredValue < constraint.min_value || 
					desiredValue > constraint.max_value)
				{
					desiredValue= constraint.default_value;
					bUpdateConfig= true;
				}

				setVideoProperty(prop_type, desiredValue, bUpdateConfig);
			}
		}
	}
}

void WMFMonoVideoSource::saveSettings()
{
    m_cfg->save();
}

bool WMFMonoVideoSource::getAvailableTrackerModes(std::vector<std::string> &out_mode_names) const
{
	if (m_capabilities)
	{
		m_capabilities->getAvailableVideoModes(out_mode_names);
		return true;
	}

	return false;
}

const VideoModeConfig *WMFMonoVideoSource::getVideoMode() const
{
	return m_currentMode;
}

bool WMFMonoVideoSource::setVideoMode(const std::string mode_name)
{
	const VideoModeConfig *new_mode= m_capabilities->findVideoMode(mode_name);

	if (new_mode != nullptr && new_mode != m_currentMode)
	{		
		const WMFDeviceFormatInfo *deviceInfo= m_videoDevice->getCurrentDeviceFormat();
		std::string mfvideoformat= std::string("MFVideoFormat_")+new_mode->bufferFormat;
		int desiredFormatIndex= m_videoDevice->m_deviceInfo.findBestDeviceFormatIndex(
			(unsigned int)new_mode->bufferPixelWidth,
			(unsigned int)new_mode->bufferPixelHeight,
			(unsigned int)new_mode->frameRate,
			mfvideoformat.c_str());

		m_cfg->areIntrinsicsUserCalibrated= false;
		m_cfg->tracker_intrinsics= new_mode->intrinsics.intrinsics.mono;
		m_currentMode= new_mode;

		if (desiredFormatIndex != INVALID_DEVICE_FORMAT_INDEX)
		{
			m_videoDevice->open(desiredFormatIndex, m_cfg, m_listener);
		}

		m_cfg->save();

		return true;
	}

	return false;
}

double WMFMonoVideoSource::getFrameWidth() const
{
	return (double)m_currentMode->intrinsics.intrinsics.mono.pixel_width;
}

double WMFMonoVideoSource::getFrameHeight() const
{
	return (double)m_currentMode->intrinsics.intrinsics.mono.pixel_height;
}

double WMFMonoVideoSource::getFrameRate() const
{
	return (double)m_currentMode->frameRate;
}

bool WMFMonoVideoSource::getVideoPropertyConstraint(const VideoPropertyType property_type, VideoPropertyConstraint &outConstraint) const
{
	
	return m_videoDevice->getVideoPropertyConstraint(property_type, outConstraint);
}

void WMFMonoVideoSource::setVideoProperty(const VideoPropertyType property_type, int desired_value, bool bUpdateConfig)
{
	m_videoDevice->setVideoProperty(property_type, desired_value);

	if (bUpdateConfig)
	{
		m_cfg->video_properties[(int)property_type] = desired_value;
	}
}

int WMFMonoVideoSource::getVideoProperty(const VideoPropertyType property_type) const
{
	return m_videoDevice->getVideoProperty(property_type);
}

void WMFMonoVideoSource::getCameraIntrinsics(
	MikanVideoSourceIntrinsics& out_tracker_intrinsics) const
{
    out_tracker_intrinsics.intrinsics_type= MONO_CAMERA_INTRINSICS;
    out_tracker_intrinsics.intrinsics.mono= m_cfg->tracker_intrinsics;
}

void WMFMonoVideoSource::setCameraIntrinsics(
    const MikanVideoSourceIntrinsics& tracker_intrinsics)
{
    assert(tracker_intrinsics.intrinsics_type == MONO_CAMERA_INTRINSICS);
    m_cfg->tracker_intrinsics= tracker_intrinsics.intrinsics.mono;
	m_cfg->areIntrinsicsUserCalibrated = true;
}

MikanQuatd WMFMonoVideoSource::getCameraOffsetOrientation() const
{
    return m_cfg->orientationOffset;
}

MikanVector3d WMFMonoVideoSource::getCameraOffsetPosition() const
{
    return m_cfg->positionOffset;
}

void WMFMonoVideoSource::setCameraPoseOffset(const MikanQuatd& q, const MikanVector3d& p)
{
    m_cfg->orientationOffset= q;
    m_cfg->positionOffset= p;
    m_cfg->save();
}

void WMFMonoVideoSource::getFOV(float &outHFOV, float &outVFOV) const
{
    outHFOV = static_cast<float>(m_cfg->tracker_intrinsics.hfov);
    outVFOV = static_cast<float>(m_cfg->tracker_intrinsics.vfov);
}

void WMFMonoVideoSource::getZRange(float &outZNear, float &outZFar) const
{
    outZNear = static_cast<float>(m_cfg->tracker_intrinsics.znear);
    outZFar = static_cast<float>(m_cfg->tracker_intrinsics.zfar);
}

void WMFMonoVideoSource::setVideoSourceListener(IVideoSourceListener *listener)
{
	m_listener= listener;
}