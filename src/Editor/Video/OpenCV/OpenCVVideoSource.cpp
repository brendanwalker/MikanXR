// -- includes -----
#include "OpenCVVideoSource.h"
#include "Logger.h"
#include "OpenCVVideo.h"
#include "VideoDeviceEnumerator.h"
#include "VideoCapabilitiesConfig.h"
#include "OpenCVCameraEnumerator.h"

#include <memory>

#ifdef _MSC_VER
    #pragma warning (disable: 4996) // 'This function or variable may be unsafe': strncpy
#endif


OpenCVVideoSource::OpenCVVideoSource(IVideoSourceListener* listener)
    : m_listener(listener)
	, m_videoCapabilities()
	, m_currentModeIndex(-1)
	, m_cfg()
    , m_deviceIdentifier()
    , m_videoDevice(nullptr)
    , m_driverType(IVideoSourceInterface::eDriverType::INVALID)
{
}

OpenCVVideoSource::~OpenCVVideoSource()
{
    if (getIsOpen())
    {
        MIKAN_LOG_ERROR("~OpenCVVideoSource") << "VideoSource deleted without calling close() first!";
    }
}

bool OpenCVVideoSource::open() // Opens the first OpenCV camera port
{
    OpenCVCameraEnumerator enumerator;
    bool success = false;

    if (enumerator.isValid())
    {
        success = open(&enumerator);
    }

    return success;
}

// -- IDeviceInterface
bool OpenCVVideoSource::matchesDeviceEnumerator(const DeviceEnumerator *enumerator) const
{
    // Down-cast the enumerator so we can use the correct get_path.
    const VideoDeviceEnumerator *pEnum = static_cast<const VideoDeviceEnumerator*>(enumerator);

    return pEnum->getDevicePath() == m_deviceIdentifier;
}

bool OpenCVVideoSource::open(const DeviceEnumerator *enumerator)
{
    const VideoDeviceEnumerator* videoDeviceEnumerator = static_cast<const VideoDeviceEnumerator*>(enumerator);
    const char *devicePath = videoDeviceEnumerator->getDevicePath();
	const int cameraIndex = videoDeviceEnumerator->getCameraIndex();

    bool bSuccess = true;
    
    if (getIsOpen())
    {
        MIKAN_LOG_WARNING("OpenCVVideoSource::open") << "OpenCVVideoSource(" << devicePath << ") already open. Ignoring request.";
    }
    else
    {
		const OpenCVCameraEnumerator *cameraEnumerator= videoDeviceEnumerator->getOpenCVCameraEnumerator();

        MIKAN_LOG_INFO("OpenCVVideoSource::open") << "Opening OpenCVVideoSource(" << devicePath << ", camera_index=" << cameraIndex << ")";

		// Remember the path to this camera
        m_deviceIdentifier = devicePath;

		// TODO: Get the underlying driver type from the opencv enumerator
		m_driverType= IVideoSourceInterface::eDriverType::OpenCV;

		// Copy the video capabilities from the enumerator
		m_videoCapabilities = cameraEnumerator->getVideoCapabilities();

        // Load the config file for the tracker
        m_cfg = std::make_shared<OpenCVVideoConfig>(m_deviceIdentifier);
        m_cfg->load();

		// If no mode is specified, then default to the first mode
		if (m_cfg->current_mode == "")
		{
			m_cfg->current_mode= m_videoCapabilities->supportedModes[0].modeName;
		}

		// Create a new OpenCV video device to manage the video stream
		m_videoDevice =
			new OpenCVVideoDevice(
				cameraIndex,
				cameraEnumerator->getVideoCapabilities());

		// Set the video mode based on what was loaded from the config
		bSuccess= setVideoMode(m_cfg->current_mode);

		// Save the config back out again in case defaults changed
        m_cfg->save();
    }
    
    if (!bSuccess)
    {
        close();
    }

    return bSuccess;
}

bool OpenCVVideoSource::getIsOpen() const
{
    return m_videoDevice != nullptr && m_videoDevice->getIsOpen();
}

void OpenCVVideoSource::close()
{
	m_currentModeIndex= -1;

    if (m_videoDevice != nullptr)
    {
        delete m_videoDevice;
        m_videoDevice = nullptr;
    }
}

eVideoStreamingStatus OpenCVVideoSource::startVideoStream()
{
	if (getIsOpen() && m_videoDevice->startVideoStream())
	{
		return eVideoStreamingStatus::started;
	}

	return eVideoStreamingStatus::stopped;
}

eVideoStreamingStatus OpenCVVideoSource::getVideoStreamingStatus() const
{
	return 
		getIsOpen() && m_videoDevice->getIsVideoStreaming()
		? eVideoStreamingStatus::started
		: eVideoStreamingStatus::stopped;
}

void OpenCVVideoSource::stopVideoStream()
{
	if (getIsOpen())
	{
		m_videoDevice->stopVideoStream();
	}
}

eDeviceType OpenCVVideoSource::getDeviceType() const
{
    return eDeviceType::MonoVideoSource;
}

IVideoSourceInterface::eDriverType OpenCVVideoSource::getDriverType() const
{
    return m_driverType;
}

std::string OpenCVVideoSource::getUSBDevicePath() const
{
    return m_deviceIdentifier;
}

bool OpenCVVideoSource::getVideoFrameDimensions(
    int *out_width,
    int *out_height,
    int *out_stride) const
{
	const VideoModeConfig* videoMode = getVideoMode();
	if (videoMode == nullptr)
		return false;

    bool bSuccess = true;

    if (out_width != nullptr)
    {
        int width = (int)videoMode->bufferPixelWidth;

        if (out_stride != nullptr)
        {
            // Assume 3 bytes per pixel?
            *out_stride = 3 * width;
        }

        *out_width = width;
    }

    if (out_height != nullptr)
    {
        int height = (int)videoMode->bufferPixelHeight;

        *out_height = height;
    }

    return bSuccess;
}

void OpenCVVideoSource::loadSettings()
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

void OpenCVVideoSource::saveSettings()
{
    m_cfg->save();
}

bool OpenCVVideoSource::getAvailableTrackerModes(std::vector<std::string> &out_mode_names) const
{
	m_videoCapabilities->getAvailableVideoModes(out_mode_names);
	return true;
}

const VideoModeConfig *OpenCVVideoSource::getVideoMode() const
{
	return m_currentModeIndex != -1 ? &m_videoCapabilities->supportedModes[m_currentModeIndex] : nullptr;
}

bool OpenCVVideoSource::setVideoMode(const std::string mode_name)
{
	const int newModeIndex= m_videoCapabilities->findVideoModeIndex(mode_name);

	if (newModeIndex != -1 && newModeIndex != m_currentModeIndex)
	{	
		const VideoModeConfig &newVideoMode= m_videoCapabilities->supportedModes[newModeIndex];

		if (m_videoDevice->open(newModeIndex, m_cfg, m_listener))
		{
			// TODO: Update the intrinsics in the config
			//m_cfg.trackerIntrinsics= new_mode->intrinsics.intrinsics.mono;

			// Update the current mode name on the config
			m_cfg->current_mode= newVideoMode.modeName;

			// Remember the index of the currently selected video mode
			m_currentModeIndex = newModeIndex;

			// Save the config back to disk
			m_cfg->save();

			return true;
		}
	}

	return false;
}

double OpenCVVideoSource::getFrameWidth() const
{
	const VideoModeConfig* videoMode= getVideoMode();

	return videoMode != nullptr ? (double)videoMode->bufferPixelWidth : 0.0;
}

double OpenCVVideoSource::getFrameHeight() const
{
	const VideoModeConfig* videoMode = getVideoMode();

	return videoMode != nullptr ? (double)videoMode->bufferPixelWidth : 0.0;
}

double OpenCVVideoSource::getFrameRate() const
{
	const VideoModeConfig* videoMode = getVideoMode();

	return videoMode != nullptr ? (double)videoMode->frameRate : 0.0;
}

bool OpenCVVideoSource::getVideoPropertyConstraint(const VideoPropertyType property_type, VideoPropertyConstraint &outConstraint) const
{
	return m_videoDevice->getVideoPropertyConstraint(property_type, outConstraint);
}

void OpenCVVideoSource::setVideoProperty(const VideoPropertyType property_type, int desired_value, bool bUpdateConfig)
{
	m_videoDevice->setVideoProperty(property_type, desired_value);

	if (bUpdateConfig)
	{
		m_cfg->video_properties[(int)property_type] = desired_value;
	}
}

int OpenCVVideoSource::getVideoProperty(const VideoPropertyType property_type) const
{
	return m_videoDevice->getVideoProperty(property_type);
}

void OpenCVVideoSource::getCameraIntrinsics(
	MikanVideoSourceIntrinsics& outCameraIntrinsics) const
{
	outCameraIntrinsics.makeMonoIntrinsics()= m_cfg->cameraIntrinsics;
}

void OpenCVVideoSource::setCameraIntrinsics(
	const MikanVideoSourceIntrinsics& videoSourceIntrinsics)
{
	m_cfg->cameraIntrinsics = videoSourceIntrinsics.getMonoIntrinsics();
}

MikanQuatd OpenCVVideoSource::getCameraOffsetOrientation() const
{
	return m_cfg->orientationOffset;
}

MikanVector3d OpenCVVideoSource::getCameraOffsetPosition() const
{
	return m_cfg->positionOffset;
}

void OpenCVVideoSource::setCameraPoseOffset(const MikanQuatd& q, const MikanVector3d& p)
{
	m_cfg->orientationOffset = q;
	m_cfg->positionOffset = p;
	m_cfg->save();
}

void OpenCVVideoSource::getFOV(float &outHFOV, float &outVFOV) const
{
	outHFOV = static_cast<float>(m_cfg->cameraIntrinsics.hfov);
	outVFOV = static_cast<float>(m_cfg->cameraIntrinsics.vfov);
}

void OpenCVVideoSource::getZRange(float &outZNear, float &outZFar) const
{
    outZNear = static_cast<float>(m_cfg->cameraIntrinsics.znear);
    outZFar = static_cast<float>(m_cfg->cameraIntrinsics.zfar);
}