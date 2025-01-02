// -- includes -----
#include "GStreamerVideo.h"
#include "DeviceInterface.h"
#include "Logger.h"
#include "GStreamerCameraEnumerator.h"
#include "WorkerThread.h"

//#ifdef _MSC_VER
//#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strncpy
//#endif

#include <opencv2/opencv.hpp>
//#include <opencv2/videoio/videoio_c.h>
//#include <opencv2/videoio.hpp>


// -- WMF Video Device -----
GStreamerVideoDevice::GStreamerVideoDevice(const int deviceIndex, VideoCapabilitiesConfigConstPtr videoCaps)
	: m_deviceIndex(deviceIndex)
	//, m_videoCapabilities(videoCaps)
	, m_videoFrameProcessor(nullptr)
{
	//memset(&m_videoPropertyConstraints, sizeof(m_videoPropertyConstraints), 0);
}

GStreamerVideoDevice::~GStreamerVideoDevice()
{
	close();
}

bool GStreamerVideoDevice::open(
	GStreamerVideoConfigPtr cfg,
	IVideoSourceListener* videoSourceListener)
{
	if (getIsOpen())
	{
		return true;
	}

	// Close the device if it's currently open
	if (getIsOpen())
	{
		close();
	}

	// Create a new video frame processor to manage frame reading from a thread
	m_videoFrameProcessor = new GStreamerVideoFrameProcessor(m_deviceIndex, videoSourceListener);

	// Attempt to open the desired video mode
	//const VideoModeConfig& desiredModeConfig = m_videoCapabilities->supportedModes[desiredModeIndex];
	//if (m_videoFrameProcessor->openVideoMode(desiredModeConfig))
	if (m_videoFrameProcessor->openVideoMode(cfg))
	{
	#if 0
		// Update the property constraints for the current video format
		for (int prop_index = 0; prop_index < (int)VideoPropertyType::COUNT; ++prop_index)
		{
			getVideoPropertyConstraint((VideoPropertyType)prop_index, m_videoPropertyConstraints[prop_index]);
		}

		// Apply video property settings stored in config onto the camera
		for (int prop_index = 0; prop_index < (int)VideoPropertyType::COUNT; ++prop_index)
		{
			const VideoPropertyType prop_type = (VideoPropertyType)prop_index;
			const VideoPropertyConstraint& constraint = m_videoPropertyConstraints[prop_index];

			if (constraint.is_supported)
			{
				// Use the properties from the config if we used this video mode previously
				if (desiredModeConfig.modeName == cfg->current_mode)
				{
					int currentValue = getVideoProperty(prop_type);
					int desiredValue = cfg->video_properties[prop_index];

					if (desiredValue != currentValue ||
						prop_type == VideoPropertyType::Focus) // always set focus to disable auto-focus
					{
						// Use the desired value if it is in-range
						if (desiredValue >= constraint.min_value &&
							desiredValue <= constraint.max_value)
						{
							setVideoProperty(prop_type, desiredValue);
						}
						// Otherwise update the config to use the current value
						else
						{
							cfg->video_properties[prop_index] = currentValue;
						}
					}
				}
				// Otherwise use the current value for the property
				// and update the config to match
				else
				{
					int currentValue = getVideoProperty(prop_type);

					if (currentValue >= constraint.min_value &&
						currentValue <= constraint.max_value)
					{
						cfg->video_properties[prop_index] = currentValue;
					}
					else
					{
						// If the current value is somehow out-of-range
						// fallback to the default value
						setVideoProperty(prop_type, constraint.default_value);
						cfg->video_properties[prop_index] = constraint.default_value;
					}
				}
			}
		}

		// Remember which video format index that was last successfully opened
		m_deviceModeIndex = desiredModeIndex;
		cfg->current_mode = desiredModeConfig.modeName;
	#endif
	}

	if (!m_videoFrameProcessor->getIsCameraOpen())
	{
		close();
		return false;
	}

	return true;
}

bool GStreamerVideoDevice::getIsOpen() const
{
	return m_videoFrameProcessor != nullptr && m_videoFrameProcessor->getIsCameraOpen();
}

void GStreamerVideoDevice::close()
{
	if (m_videoFrameProcessor != nullptr)
	{
		m_videoFrameProcessor->closeVideoMode();

		delete m_videoFrameProcessor;
		m_videoFrameProcessor = nullptr;
	}
}

bool GStreamerVideoDevice::startVideoStream()
{
	if (getIsOpen() && !getIsVideoStreaming())
	{
		return m_videoFrameProcessor->startVideoFrameThread();
	}

	return false;
}

bool GStreamerVideoDevice::getIsVideoStreaming()
{
	if (getIsOpen())
	{
		return m_videoFrameProcessor->getIsThreadRunning();
	}

	return false;
}

void GStreamerVideoDevice::stopVideoStream()
{
	if (getIsOpen())
	{
		m_videoFrameProcessor->stopVideoFrameThread();
	}
}

const VideoModeConfig* GStreamerVideoDevice::getCurrentVideoMode() const
{
	return nullptr;
	//return
	//	(m_deviceModeIndex != -1)
	//	? &m_videoCapabilities->supportedModes[m_deviceModeIndex]
	//	: nullptr;
}

bool GStreamerVideoDevice::getVideoPropertyConstraint(const VideoPropertyType property_type, VideoPropertyConstraint& outConstraint) const
{
	bool bSuccess = false;

#if 0
	switch (property_type)
	{
		case VideoPropertyType::Brightness:
		case VideoPropertyType::Contrast:
		case VideoPropertyType::Hue:
		case VideoPropertyType::Saturation:
		case VideoPropertyType::Sharpness:
		case VideoPropertyType::Gamma:
		case VideoPropertyType::WhiteBalance:
		case VideoPropertyType::RedBalance:
		case VideoPropertyType::GreenBalance:
		case VideoPropertyType::BlueBalance:
		case VideoPropertyType::Gain:
		case VideoPropertyType::Pan:
		case VideoPropertyType::Tilt:
		case VideoPropertyType::Roll:
		case VideoPropertyType::Zoom:
		case VideoPropertyType::Exposure:
		case VideoPropertyType::Iris:
		case VideoPropertyType::Focus:
			memset(&outConstraint, 0, sizeof(VideoPropertyConstraint));
			bSuccess = true;
			break;
}
#endif

	return bSuccess;
}

void GStreamerVideoDevice::setVideoProperty(const VideoPropertyType property_type, int desired_value)
{
	if (m_videoFrameProcessor == nullptr)
		return;

#if 0
	cv::VideoCapture* device = m_videoFrameProcessor->getVideoCaptureInterface();

	switch (property_type)
	{
		case VideoPropertyType::Brightness:
			device->set(cv::CAP_PROP_BRIGHTNESS, (double)desired_value);
			break;
		case VideoPropertyType::Contrast:
			device->set(cv::CAP_PROP_CONTRAST, (double)desired_value);
			break;
		case VideoPropertyType::Hue:
			device->set(cv::CAP_PROP_HUE, (double)desired_value);
			break;
		case VideoPropertyType::Saturation:
			device->set(cv::CAP_PROP_SATURATION, (double)desired_value);
			break;
		case VideoPropertyType::Sharpness:
			device->set(cv::CAP_PROP_SHARPNESS, (double)desired_value);
			break;
		case VideoPropertyType::Gamma:
			device->set(cv::CAP_PROP_GAMMA, (double)desired_value);
			break;
		case VideoPropertyType::WhiteBalance:
			device->set(cv::CAP_PROP_WB_TEMPERATURE, (double)desired_value);
			break;
		case VideoPropertyType::RedBalance:
		case VideoPropertyType::GreenBalance:
		case VideoPropertyType::BlueBalance:
			// not supported
			break;
		case VideoPropertyType::Gain:
			device->set(cv::CAP_PROP_GAIN, (double)desired_value);
			break;
		case VideoPropertyType::Pan:
			device->set(cv::CAP_PROP_PAN, (double)desired_value);
			break;
		case VideoPropertyType::Tilt:
			device->set(cv::CAP_PROP_TILT, (double)desired_value);
			break;
		case VideoPropertyType::Roll:
			device->set(cv::CAP_PROP_ROLL, (double)desired_value);
			break;
		case VideoPropertyType::Zoom:
			device->set(cv::CAP_PROP_ZOOM, (double)desired_value);
			break;
		case VideoPropertyType::Exposure:
			device->set(cv::CAP_PROP_EXPOSURE, (double)desired_value);
			break;
		case VideoPropertyType::Iris:
			device->set(cv::CAP_PROP_IRIS, (double)desired_value);
			break;
		case VideoPropertyType::Focus:
			device->set(cv::CAP_PROP_FOCUS, (double)desired_value);
			break;
}
#endif
}

int GStreamerVideoDevice::getVideoProperty(const VideoPropertyType property_type) const
{
	if (m_videoFrameProcessor == nullptr)
		return 0;

	int value = 0;
#if 0
	cv::VideoCapture* device = m_videoFrameProcessor->getVideoCaptureInterface();

	switch (property_type)
	{
		case VideoPropertyType::Brightness:
			value = (int)device->get(cv::CAP_PROP_BRIGHTNESS);
			break;
		case VideoPropertyType::Contrast:
			value = (int)device->get(cv::CAP_PROP_CONTRAST);
			break;
		case VideoPropertyType::Hue:
			value = (int)device->get(cv::CAP_PROP_HUE);
			break;
		case VideoPropertyType::Saturation:
			value = (int)device->get(cv::CAP_PROP_SATURATION);
			break;
		case VideoPropertyType::Sharpness:
			value = (int)device->get(cv::CAP_PROP_SHARPNESS);
			break;
		case VideoPropertyType::Gamma:
			value = (int)device->get(cv::CAP_PROP_GAMMA);
			break;
		case VideoPropertyType::WhiteBalance:
			value = (int)device->get(cv::CAP_PROP_WB_TEMPERATURE);
			break;
		case VideoPropertyType::RedBalance:
		case VideoPropertyType::GreenBalance:
		case VideoPropertyType::BlueBalance:
			// not supported
			break;
		case VideoPropertyType::Gain:
			value = (int)device->get(cv::CAP_PROP_GAIN);
			break;
		case VideoPropertyType::Pan:
			value = (int)device->get(cv::CAP_PROP_PAN);
			break;
		case VideoPropertyType::Tilt:
			value = (int)device->get(cv::CAP_PROP_TILT);
			break;
		case VideoPropertyType::Roll:
			value = (int)device->get(cv::CAP_PROP_ROLL);
			break;
		case VideoPropertyType::Zoom:
			value = (int)device->get(cv::CAP_PROP_ZOOM);
			break;
		case VideoPropertyType::Exposure:
			value = (int)device->get(cv::CAP_PROP_EXPOSURE);
			break;
		case VideoPropertyType::Iris:
			value = (int)device->get(cv::CAP_PROP_IRIS);
			break;
		case VideoPropertyType::Focus:
			value = (int)device->get(cv::CAP_PROP_FOCUS);
			break;
}
#endif

	return value;
}

// -- GStreamer Video Frame Processor -----
GStreamerVideoFrameProcessor::GStreamerVideoFrameProcessor(
	const int deviceIndex,
	IVideoSourceListener* listener)
	: WorkerThread("GStreamerVideoFrameProcessor")
	, m_deviceIndex(deviceIndex)
	//, m_videoCapture(new cv::VideoCapture)
	, m_videoFrame(new cv::Mat)
	, m_videoSourceListener(listener)
{}

GStreamerVideoFrameProcessor::~GStreamerVideoFrameProcessor(void)
{
	closeVideoMode();

	//if (m_videoCapture)
	//{
	//	delete m_videoCapture;
	//	m_videoCapture = nullptr;
	//}

	if (m_videoFrame)
	{
		delete m_videoFrame;
		m_videoFrame = nullptr;
	}
}

bool GStreamerVideoFrameProcessor::openVideoMode(GStreamerVideoConfigPtr cfg)
{
#if 0
	if (m_videoCapture->open(m_deviceIndex, cv::CAP_ANY))
	{
		int currentPixelWidth = m_videoCapture->get(cv::CAP_PROP_FRAME_WIDTH);
		int currentPixelHeight = m_videoCapture->get(cv::CAP_PROP_FRAME_HEIGHT);
		float currentFPS = (float)m_videoCapture->get(cv::CAP_PROP_FPS);

		if (currentPixelWidth != videoModeConfig.bufferPixelWidth)
		{
			m_videoCapture->set(cv::CAP_PROP_FRAME_WIDTH, (double)videoModeConfig.bufferPixelWidth);
		}

		if (currentPixelHeight != videoModeConfig.bufferPixelHeight)
		{
			m_videoCapture->set(cv::CAP_PROP_FRAME_HEIGHT, (double)videoModeConfig.bufferPixelHeight);
		}

		if (currentFPS != videoModeConfig.frameRate)
		{
			m_videoCapture->set(cv::CAP_PROP_FPS, (double)videoModeConfig.frameRate);
		}

		return true;
	}
#endif

	return false;
}

void GStreamerVideoFrameProcessor::closeVideoMode()
{
	stopVideoFrameThread();

	//if (m_videoCapture != nullptr)
	//{
	//	m_videoCapture->release();
	//}

	MIKAN_LOG_INFO("GStreamerVideoFrameProcessor::dispose") << "Disposing video frame grabber for device: " << m_deviceIndex;
}

bool GStreamerVideoFrameProcessor::getIsCameraOpen() const
{
	//return m_videoCapture->isOpened();
	return false;
}

bool GStreamerVideoFrameProcessor::startVideoFrameThread()
{
	if (getIsCameraOpen() && !getIsThreadRunning())
	{
		WorkerThread::startThread();
	}

	return getIsThreadRunning();
}

bool GStreamerVideoFrameProcessor::doWork()
{
	bool bKeepRunning = true;

	//if (m_videoCapture->isOpened())
	//{
	//	if (m_videoCapture->read(*m_videoFrame))
	//	{
	//		m_videoSourceListener->notifyVideoFrameReceived(m_videoFrame->data);
	//	}
	//}
	//else
	//{
	//	MIKAN_MT_LOG_INFO("GStreamerVideoFrameProcessor::doWork") << "GStreamer Video capture closed: " << m_deviceIndex;
	//	bKeepRunning = false;
	//}

	return bKeepRunning;
}

void GStreamerVideoFrameProcessor::stopVideoFrameThread()
{
	MIKAN_LOG_INFO("GStreamerVideoFrameProcessor::stop") << "Stopping video frame grabbing on device: " << m_deviceIndex;

	if (getIsThreadRunning())
	{
		WorkerThread::stopThread();
	}
}

bool GStreamerVideoFrameProcessor::getIsThreadRunning() const
{
	return hasThreadStarted() && !hasThreadEnded();
}