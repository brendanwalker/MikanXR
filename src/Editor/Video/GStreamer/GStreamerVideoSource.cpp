// -- includes -----
#include "GStreamerVideoSource.h"
#include "Logger.h"
#include "MikanGStreamerModule.h"
#include "VideoDeviceEnumerator.h"
#include "VideoCapabilitiesConfig.h"
#include "VideoSourceManager.h"
#include "GStreamerCameraEnumerator.h"

#include <algorithm>
#include <memory>

GStreamerVideoSource::GStreamerVideoSource(IVideoSourceListener* listener)
	: m_listener(listener)
	, m_cfg()
	, m_videoModeConfig()
	, m_deviceIdentifier()
	, m_videoDevice(nullptr)
	, m_driverType(IVideoSourceInterface::eDriverType::INVALID)
{
	std::memset(&m_gstreamerVideoMode, 0, sizeof(MikanGStreamerVideoMode));
}

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
		MikanGStreamerSettings settings;
		settings.protocol = m_cfg->protocol;
		settings.address = m_cfg->address.c_str();
		settings.path = m_cfg->path.c_str();
		settings.port = m_cfg->port;

		IMikanGStreamerModule* gstreamerModule= VideoSourceManager::getInstance()->getGStreamerModule();
		m_videoDevice = gstreamerModule->createVideoDevice(settings);
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
	m_videoModeConfig= nullptr;
	m_videoDevice = nullptr;
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
		m_gstreamerVideoMode,
		&GStreamerVideoSource::onVideoModeChanged,
		&GStreamerVideoSource::onVideoFrameReceived,
		this);
}

void GStreamerVideoSource::onVideoModeChanged(const struct MikanGStreamerVideoMode& newVideoMode, void* userdata)
{
	auto* gstreamerVideoSource= reinterpret_cast<GStreamerVideoSource *>(userdata);

	VideoModeConfigPtr videoModeConfig= std::make_shared<VideoModeConfig>();
	videoModeConfig->modeName= newVideoMode.modeName;
	videoModeConfig->frameRate= newVideoMode.frameRate;
	videoModeConfig->bufferPixelWidth= newVideoMode.bufferPixelWidth;
	videoModeConfig->bufferPixelHeight= newVideoMode.bufferPixelHeight;
	videoModeConfig->bufferFormat= newVideoMode.bufferFormat;
	videoModeConfig->frameSections.push_back({0, 0});
	videoModeConfig->intrinsics.makeMonoIntrinsics() = gstreamerVideoSource->m_cfg->cameraIntrinsics;

	// Store the new video mode
	gstreamerVideoSource->m_gstreamerVideoMode = newVideoMode;
	gstreamerVideoSource->m_videoModeConfig = videoModeConfig;

	// Notify the listener that the video frame size has changed
	gstreamerVideoSource->m_listener->notifyVideoFrameSizeChanged();
}

void GStreamerVideoSource::onVideoFrameReceived(const struct MikanGStreamerBuffer& newBuffer, void* userdata)
{
	auto* gstreamerVideoSource= reinterpret_cast<GStreamerVideoSource *>(userdata);

	// Notify the listener that a new video frame has been received
	IVideoSourceListener::FrameBuffer frameInfo = {newBuffer.data, newBuffer.byte_count};
	gstreamerVideoSource->m_listener->notifyVideoFrameReceived(frameInfo); 
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
	return m_videoModeConfig.get();
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

	return videoMode != nullptr ? (double)videoMode->bufferPixelHeight : 0.0;
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
	outCameraIntrinsics.makeMonoIntrinsics()= m_cfg->cameraIntrinsics;
}

void GStreamerVideoSource::setCameraIntrinsics(
	const MikanVideoSourceIntrinsics& videoSourceIntrinsics)
{
	m_cfg->cameraIntrinsics = videoSourceIntrinsics.getMonoIntrinsics();
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