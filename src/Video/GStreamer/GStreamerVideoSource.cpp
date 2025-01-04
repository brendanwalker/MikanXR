// -- includes -----
#include "GStreamerVideoSource.h"
#include "Logger.h"
#include "GStreamerVideo.h"
#include "VideoDeviceEnumerator.h"
#include "VideoCapabilitiesConfig.h"
#include "GStreamerCameraEnumerator.h"

#include <algorithm>
#include <memory>

GStreamerVideoSource::GStreamerVideoSource(IVideoSourceListener* listener)
	: m_listener(listener)
	, m_cfg()
	, m_currentVideoMode()
	, m_deviceIdentifier()
	, m_videoDevice(nullptr)
	, m_driverType(IVideoSourceInterface::eDriverType::INVALID)
{}

GStreamerVideoSource::~GStreamerVideoSource()
{
	if (getIsOpen())
	{
		MIKAN_LOG_ERROR("~GStreamerVideoSource") << "VideoSource deleted without calling close() first!";
	}
}

bool GStreamerVideoSource::open() // Opens the first GStreamer camera port
{
	GStreamerCameraEnumerator enumerator;
	bool success = false;

	if (enumerator.isValid())
	{
		success = open(&enumerator);
	}

	return success;
}

// -- IDeviceInterface
bool GStreamerVideoSource::matchesDeviceEnumerator(const DeviceEnumerator* enumerator) const
{
	// Down-cast the enumerator so we can use the correct get_path.
	const VideoDeviceEnumerator* pEnum = static_cast<const VideoDeviceEnumerator*>(enumerator);

	return pEnum->getDevicePath() == m_deviceIdentifier;
}

bool GStreamerVideoSource::open(const DeviceEnumerator* enumerator)
{
	const VideoDeviceEnumerator* videoDeviceEnumerator = static_cast<const VideoDeviceEnumerator*>(enumerator);
	const int cameraIndex = videoDeviceEnumerator->getCameraIndex();

	bool bSuccess = true;

	// Remember the device path
	m_devicePath = videoDeviceEnumerator->getDevicePath();

	if (getIsOpen())
	{
		MIKAN_LOG_WARNING("GStreamerVideoSource::open") 
			<< "GStreamerVideoSource(" << m_devicePath 
			<< ") already open. Ignoring request.";
	}
	else
	{
		const GStreamerCameraEnumerator* cameraEnumerator = videoDeviceEnumerator->getGStreamerCameraEnumerator();

		MIKAN_LOG_INFO("GStreamerVideoSource::open") << 
			"Opening GStreamerVideoSource(" << m_devicePath << 
			", camera_index=" << cameraIndex << ")";

		// Use the device URI as the identifier, but sanitize it for use as a filename
		m_deviceIdentifier = m_devicePath;
		std::replace(m_deviceIdentifier.begin(), m_deviceIdentifier.end(), '.', '_');
		std::replace(m_deviceIdentifier.begin(), m_deviceIdentifier.end(), ',', '_');
		std::replace(m_deviceIdentifier.begin(), m_deviceIdentifier.end(), ':', '_');
		std::replace(m_deviceIdentifier.begin(), m_deviceIdentifier.end(), '/', '_');

		// Get the underlying driver type from the GStreamer enumerator
		m_driverType = IVideoSourceInterface::eDriverType::GStreamer;

		// Load the config file for the tracker
		m_cfg = std::make_shared<GStreamerVideoConfig>(m_deviceIdentifier);
		m_cfg->load();

		// Apply the IPAddress/port/path to the config if it's unset/changes
		if (m_cfg->applyDevicePath(m_devicePath))
		{
			m_cfg->save();
		}

		// Create a new GStreamer video device to manage the video stream
		m_videoDevice = new GStreamerVideoDevice(cameraIndex, m_cfg, m_listener);
		m_videoDevice->open();
	}

	if (!bSuccess)
	{
		close();
	}

	return bSuccess;
}

bool GStreamerVideoSource::getIsOpen() const
{
	return m_videoDevice != nullptr && m_videoDevice->getIsOpen();
}

void GStreamerVideoSource::close()
{
	m_currentVideoMode= nullptr;

	if (m_videoDevice != nullptr)
	{
		delete m_videoDevice;
		m_videoDevice = nullptr;
	}
}

bool GStreamerVideoSource::startVideoStream()
{
	if (getIsOpen())
	{
		return m_videoDevice->startVideoStream();
	}

	return false;
}

bool GStreamerVideoSource::getIsVideoStreaming() const
{
	return getIsOpen() && m_videoDevice->getIsVideoStreaming();
}

void GStreamerVideoSource::stopVideoStream()
{
	if (getIsOpen())
	{
		m_videoDevice->stopVideoStream();
	}
}

bool GStreamerVideoSource::wantsUpdate() const
{
	return getIsVideoStreaming();
}

void GStreamerVideoSource::update(float deltaTime)
{
	assert(getIsVideoStreaming());

	m_videoDevice->tryPullSample(
		m_currentVideoMode,
		[this](VideoModeConfigPtr newVideoMode) {
			// Store the new video mode first so that getVideoMode() calls return a valid pointer
			m_currentVideoMode = newVideoMode;

			// Tell the listener that the video frame size has changed
			// So that it can rebuild buffers BEFORE the next frame is received
			// later in the tryPullSample function
			m_listener->notifyVideoFrameSizeChanged();
		});
}

eDeviceType GStreamerVideoSource::getDeviceType() const
{
	return eDeviceType::MonoVideoSource;
}

IVideoSourceInterface::eDriverType GStreamerVideoSource::getDriverType() const
{
	return m_driverType;
}

std::string GStreamerVideoSource::getFriendlyName() const
{
	return m_devicePath;
}

std::string GStreamerVideoSource::getUSBDevicePath() const
{
	return m_deviceIdentifier;
}

bool GStreamerVideoSource::getVideoFrameDimensions(
	int* out_width,
	int* out_height,
	int* out_stride) const
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

void GStreamerVideoSource::loadSettings()
{
	m_cfg->load();
}

void GStreamerVideoSource::saveSettings()
{
	m_cfg->save();
}

bool GStreamerVideoSource::getAvailableTrackerModes(std::vector<std::string>& out_mode_names) const
{
	return false;
}

const VideoModeConfig* GStreamerVideoSource::getVideoMode() const
{
	return m_currentVideoMode.get();
}

bool GStreamerVideoSource::setVideoMode(const std::string mode_name)
{
	// Mode is determined by the stream
	return false;
}

double GStreamerVideoSource::getFrameWidth() const
{
	const VideoModeConfig* videoMode = getVideoMode();

	return videoMode != nullptr ? (double)videoMode->bufferPixelWidth : 0.0;
}

double GStreamerVideoSource::getFrameHeight() const
{
	const VideoModeConfig* videoMode = getVideoMode();

	return videoMode != nullptr ? (double)videoMode->bufferPixelWidth : 0.0;
}

double GStreamerVideoSource::getFrameRate() const
{
	const VideoModeConfig* videoMode = getVideoMode();

	return videoMode != nullptr ? (double)videoMode->frameRate : 0.0;
}

bool GStreamerVideoSource::getVideoPropertyConstraint(const VideoPropertyType property_type, VideoPropertyConstraint& outConstraint) const
{
	return false;
}

void GStreamerVideoSource::setVideoProperty(const VideoPropertyType property_type, int desired_value, bool bUpdateConfig)
{
}

int GStreamerVideoSource::getVideoProperty(const VideoPropertyType property_type) const
{
	return 0;
}

void GStreamerVideoSource::getCameraIntrinsics(
	MikanVideoSourceIntrinsics& outCameraIntrinsics) const
{
	outCameraIntrinsics.setMonoIntrinsics(m_cfg->cameraIntrinsics);
}

void GStreamerVideoSource::setCameraIntrinsics(
	const MikanVideoSourceIntrinsics& videoSourceIntrinsics)
{
	assert(videoSourceIntrinsics.intrinsics_type == MONO_CAMERA_INTRINSICS);

	auto cameraIntrinsics = videoSourceIntrinsics.intrinsics_ptr.getSharedPointer();
	auto monoIntrinsics = std::static_pointer_cast<MikanMonoIntrinsics>(cameraIntrinsics);

	m_cfg->cameraIntrinsics = *monoIntrinsics.get();
}

MikanQuatd GStreamerVideoSource::getCameraOffsetOrientation() const
{
	return m_cfg->orientationOffset;
}

MikanVector3d GStreamerVideoSource::getCameraOffsetPosition() const
{
	return m_cfg->positionOffset;
}

void GStreamerVideoSource::setCameraPoseOffset(const MikanQuatd& q, const MikanVector3d& p)
{
	m_cfg->orientationOffset = q;
	m_cfg->positionOffset = p;
	m_cfg->save();
}

void GStreamerVideoSource::getFOV(float& outHFOV, float& outVFOV) const
{
	outHFOV = static_cast<float>(m_cfg->cameraIntrinsics.hfov);
	outVFOV = static_cast<float>(m_cfg->cameraIntrinsics.vfov);
}

void GStreamerVideoSource::getZRange(float& outZNear, float& outZFar) const
{
	outZNear = static_cast<float>(m_cfg->cameraIntrinsics.znear);
	outZFar = static_cast<float>(m_cfg->cameraIntrinsics.zfar);
}