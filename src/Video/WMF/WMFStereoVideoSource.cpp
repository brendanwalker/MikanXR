// -- includes -----
#include "WMFStereoVideoSource.h"
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

// -- public methods
// -- WMF Stereo Tracker Config
const int WMFStereoVideoConfig::CONFIG_VERSION = 1;

WMFStereoVideoConfig::WMFStereoVideoConfig(const std::string &fnamebase)
    : WMFVideoConfig(fnamebase)
{
	tracker_intrinsics.pixel_width= 640;
	tracker_intrinsics.pixel_height= 480;
    tracker_intrinsics.hfov= 60.0; // degrees
    tracker_intrinsics.vfov= 45.0; // degrees
    tracker_intrinsics.znear= 10.0; // cm
    tracker_intrinsics.zfar= 200.0; // cm

	tracker_intrinsics.left_camera_matrix = {
		554.2563, 0, 320.0, // f_x, 0, c_x
		0, 554.2563, 240.0, // 0, f_y, c_y
		0.0, 0.0, 1.0 
	};
    tracker_intrinsics.right_camera_matrix = {
		554.2563, 0, 320.0, // f_x, 0, c_x
		0, 554.2563, 240.0, // 0, f_y, c_y
		0.0, 0.0, 1.0
    };

    tracker_intrinsics.left_distortion_coefficients= {
        -0.10771770030260086, 0.1213262677192688, 0.04875476285815239, // K1, K2, K3
        0.00091733073350042105, 0.00010589254816295579};  // P1, P2
    tracker_intrinsics.right_distortion_coefficients= {
        -0.10771770030260086, 0.1213262677192688, 0.04875476285815239, // K1, K2, K3
        0.00091733073350042105, 0.00010589254816295579};  // P1, P2

    // All identity matrices
	const MikanMatrix3d matrix3_identity = {
		1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 1.0
	};

	const MikanMatrix4x3d matrix4x3_identity = {
		1.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 1.0,
		0.0, 0.0, 0.0
	};

    tracker_intrinsics.left_rectification_rotation= matrix3_identity;
    tracker_intrinsics.right_rectification_rotation= matrix3_identity;
    tracker_intrinsics.left_rectification_projection= matrix4x3_identity;
    tracker_intrinsics.right_rectification_projection= matrix4x3_identity;
    tracker_intrinsics.rotation_between_cameras= matrix3_identity;
    tracker_intrinsics.translation_between_cameras= {0.0, 0.0, 0.0};
    tracker_intrinsics.essential_matrix= matrix3_identity;
    tracker_intrinsics.fundamental_matrix= matrix3_identity;
    tracker_intrinsics.reprojection_matrix= {
		1.0, 0.0, 0.0, 0.0,
		0.0, 1.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0,
		0.0, 0.0, 0.0, 0.0
	};
};

configuru::Config WMFStereoVideoConfig::writeToJSON()
{
    configuru::Config pt= WMFVideoConfig::writeToJSON();

    pt["version"]= WMFStereoVideoConfig::CONFIG_VERSION;
	writeStereoTrackerIntrinsics(pt, tracker_intrinsics);

    return pt;
}

void WMFStereoVideoConfig::readFromJSON(const configuru::Config &pt)
{
	WMFVideoConfig::readFromJSON(pt);

    int config_version = pt.get_or<int>("version", 0);
    if (config_version == WMFStereoVideoConfig::CONFIG_VERSION)
    {
		WMFVideoConfig::readFromJSON(pt);
		readStereoTrackerIntrinsics(pt, tracker_intrinsics);
    }
    else
    {
        MIKAN_LOG_WARNING("WMFStereoTrackerConfig") <<
            "Config version " << config_version << " does not match expected version " <<
            WMFStereoVideoConfig::CONFIG_VERSION << ", Using defaults.";
    }
}

// -- WMFStereoTracker
WMFStereoVideoSource::WMFStereoVideoSource(IVideoSourceListener* listener)
    : m_listener(listener)
	, m_cfg()
	, m_videoDevice(nullptr)
    , m_DriverType(WMFStereoVideoSource::WindowsMediaFramework)
{
}

WMFStereoVideoSource::~WMFStereoVideoSource()
{
    if (getIsOpen())
    {
        MIKAN_LOG_ERROR("~WMFStereoTracker") << "Tracker deleted without calling close() first!";
    }
}

bool WMFStereoVideoSource::open() // Opens the first HID device for the tracker
{
    VideoDeviceEnumerator enumerator;
    bool success = false;

    // Skip over everything that isn't a WMF camera
    while (enumerator.isValid() && enumerator.getDeviceType() != eDeviceType::StereoVideoSource)
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
bool WMFStereoVideoSource::matchesDeviceEnumerator(const DeviceEnumerator *enumerator) const
{
    // Down-cast the enumerator so we can use the correct get_path.
    const VideoDeviceEnumerator *pEnum = static_cast<const VideoDeviceEnumerator *>(enumerator);

    bool matches = false;

    if (pEnum->getDeviceType() == eDeviceType::StereoVideoSource)
    {
        std::string enumerator_path = pEnum->getDevicePath();

        matches = (enumerator_path == m_device_identifier);
    }

    return matches;
}

bool WMFStereoVideoSource::open(const DeviceEnumerator *enumerator)
{
    const VideoDeviceEnumerator *tracker_enumerator = static_cast<const VideoDeviceEnumerator *>(enumerator);
    const char *cur_dev_path = tracker_enumerator->getDevicePath();
	const int camera_index = tracker_enumerator->getCameraIndex();

    bool bSuccess = true;
    
    if (getIsOpen())
    {
        MIKAN_LOG_WARNING("WMFStereoTracker::open") << "WMFStereoTracker(" << cur_dev_path << ") already open. Ignoring request.";
    }
    else
    {
		const WMFCameraEnumerator *wmf_enumerator= tracker_enumerator->getWMFCameraEnumerator();
		const char *unique_id= wmf_enumerator->getUniqueIdentifier();

        MIKAN_LOG_INFO("WMFStereoTracker::open") << "Opening WMFStereoTracker(" << cur_dev_path << ", camera_index=" << camera_index << ")";

		// Remember the path to this camera
        m_device_identifier = cur_dev_path;

		// Build a config file name from the unique id
		char config_name[256];
        StringUtils::formatString(config_name, sizeof(config_name), "WMFStereoCamera_%s", unique_id);

        // Load the config file for the tracker
        m_cfg = std::make_shared<WMFStereoVideoConfig>(config_name);
        m_cfg->load();

		// Fetch the camera capabilities
		m_capabilities= wmf_enumerator->getVideoCapabilities();
        if (m_capabilities != nullptr)
        {
		    // If no mode is specified, then default to the first mode
		    if (m_cfg->current_mode == "")
		    {
			    m_cfg->current_mode= m_capabilities->supportedModes[0].modeName;
		    }

		    // Find the camera mode by name
		    m_currentMode= m_capabilities->findVideoMode(m_cfg->current_mode);
		    if (m_currentMode != nullptr)
		    {
			    // Copy the tracker intrinsics over from the capabilities
			    m_cfg->tracker_intrinsics= m_currentMode->intrinsics.getStereoIntrinsics();

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
            MIKAN_LOG_ERROR("WMFStereoVideoSource::open") << "Unable to WMFStereoTracker(" << cur_dev_path << "). Unable to get tracker capabilities.";
            bSuccess= false;
        }
    }
    
    if (!bSuccess)
    {
        close();
    }

    return bSuccess;
}

bool WMFStereoVideoSource::getIsOpen() const
{
    return m_videoDevice != nullptr && m_videoDevice->getIsOpen();
}


void WMFStereoVideoSource::close()
{
	if (m_videoDevice != nullptr)
	{
		delete m_videoDevice;
		m_videoDevice= nullptr;
	}
}

bool WMFStereoVideoSource::startVideoStream()
{
	if (getIsOpen())
	{
		return m_videoDevice->startVideoStream();
	}

	return false;
}

bool WMFStereoVideoSource::getIsVideoStreaming() const
{
	return getIsOpen() && m_videoDevice->getIsVideoStreaming();
}

void WMFStereoVideoSource::stopVideoStream()
{
	if (getIsOpen())
	{
		m_videoDevice->stopVideoStream();
	}
}

eDeviceType WMFStereoVideoSource::getDeviceType() const
{
    return eDeviceType::StereoVideoSource;
}

IVideoSourceInterface::eDriverType WMFStereoVideoSource::getDriverType() const
{
    return m_DriverType;
}

std::string WMFStereoVideoSource::getFriendlyName() const
{
	return m_videoDevice->m_deviceInfo.deviceFriendlyName;
}

std::string WMFStereoVideoSource::getUSBDevicePath() const
{
    return m_device_identifier;
}

bool WMFStereoVideoSource::getVideoFrameDimensions(
    int *out_width,
    int *out_height,
    int *out_stride) const
{
    bool bSuccess = true;

    if (out_width != nullptr)
    {
        int width = (int)m_currentMode->intrinsics.getStereoIntrinsics().pixel_width;

        if (out_stride != nullptr)
        {
            // Assume 3 bytes per pixel?
            *out_stride = 3 * width;
        }

        *out_width = width;
    }

    if (out_height != nullptr)
    {
        int height = (int)m_currentMode->intrinsics.getStereoIntrinsics().pixel_height;

        *out_height = height;
    }

    return bSuccess;
}

bool WMFStereoVideoSource::getIsFrameMirrored() const
{ 
	return m_currentMode->isFrameMirrored; 
}

bool WMFStereoVideoSource::getIsBufferMirrored() const
{ 
	return m_currentMode->isBufferMirrored; 
}

void WMFStereoVideoSource::loadSettings()
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

void WMFStereoVideoSource::saveSettings()
{
    m_cfg->save();
}

bool WMFStereoVideoSource::getAvailableTrackerModes(std::vector<std::string> &out_mode_names) const
{
	if (m_capabilities)
	{
		m_capabilities->getAvailableVideoModes(out_mode_names);
		return true;
	}

	return false;
}

const VideoModeConfig * WMFStereoVideoSource::getVideoMode() const
{
	return m_currentMode;
}

bool WMFStereoVideoSource::setVideoMode(const std::string mode_name)
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

		m_cfg->tracker_intrinsics= new_mode->intrinsics.getStereoIntrinsics();
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

double WMFStereoVideoSource::getFrameWidth() const
{
	return (double)m_currentMode->intrinsics.getStereoIntrinsics().pixel_width;
}

double WMFStereoVideoSource::getFrameHeight() const
{
	return (double)m_currentMode->intrinsics.getStereoIntrinsics().pixel_height;
}

double WMFStereoVideoSource::getFrameRate() const
{
	return (double)m_currentMode->frameRate;
}

bool WMFStereoVideoSource::getVideoPropertyConstraint(const VideoPropertyType property_type, VideoPropertyConstraint &outConstraint) const
{
	return m_videoDevice->getVideoPropertyConstraint(property_type, outConstraint);
}

void WMFStereoVideoSource::setVideoProperty(const VideoPropertyType property_type, int desired_value, bool bUpdateConfig)
{
	m_videoDevice->setVideoProperty(property_type, desired_value);

	if (bUpdateConfig)
	{
		m_cfg->video_properties[(int)property_type] = desired_value;
	}
}

int WMFStereoVideoSource::getVideoProperty(const VideoPropertyType property_type) const
{
	return m_videoDevice->getVideoProperty(property_type);
}

void WMFStereoVideoSource::getCameraIntrinsics(
    MikanVideoSourceIntrinsics& out_tracker_intrinsics) const
{
    out_tracker_intrinsics.intrinsics_type= STEREO_CAMERA_INTRINSICS;
    out_tracker_intrinsics.setStereoIntrinsics(m_cfg->tracker_intrinsics);
}

void WMFStereoVideoSource::setCameraIntrinsics(
    const MikanVideoSourceIntrinsics& tracker_intrinsics)
{
    assert(tracker_intrinsics.intrinsics_type == STEREO_CAMERA_INTRINSICS);
    m_cfg->tracker_intrinsics= tracker_intrinsics.getStereoIntrinsics();
}

MikanQuatd WMFStereoVideoSource::getCameraOffsetOrientation() const
{
    return m_cfg->orientationOffset;
}

MikanVector3d WMFStereoVideoSource::getCameraOffsetPosition() const
{
    return m_cfg->positionOffset;
}

void WMFStereoVideoSource::setCameraPoseOffset(const MikanQuatd& q, const MikanVector3d& p)
{
    m_cfg->orientationOffset= q;
    m_cfg->positionOffset= p;
}

void WMFStereoVideoSource::getFOV(float &outHFOV, float &outVFOV) const
{
	outHFOV = static_cast<float>(m_cfg->tracker_intrinsics.hfov);
	outVFOV = static_cast<float>(m_cfg->tracker_intrinsics.vfov);
}

void WMFStereoVideoSource::getZRange(float &outZNear, float &outZFar) const
{
    outZNear = static_cast<float>(m_cfg->tracker_intrinsics.znear);
    outZFar = static_cast<float>(m_cfg->tracker_intrinsics.zfar);
}