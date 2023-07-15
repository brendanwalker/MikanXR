#include "OpenCVCameraEnumerator.h"
#include "VideoCapabilitiesConfig.h"
#include "StringUtils.h"

#include <opencv2/core.hpp>     // Basic OpenCV structures (cv::Mat)
#include <opencv2/videoio.hpp>  // Video write

#define MAX_OPENCV_CAMERA_PORTS 16

OpenCVCameraEnumerator::OpenCVCameraEnumerator()
	: DeviceEnumerator()
	, m_currentDeviceCapabilities(nullptr)
	, m_devicePath("")
	, m_deviceIndex(-1)
{
	next();
}

bool OpenCVCameraEnumerator::isValid() const
{
	return m_currentDeviceCapabilities != nullptr;
}

eDeviceType OpenCVCameraEnumerator::getDeviceType() const
{
	return isValid() ? m_currentDeviceCapabilities->deviceType : eDeviceType::INVALID;
}

bool OpenCVCameraEnumerator::next()
{
	bool bFoundValid = false;
	++m_deviceIndex;

	while (!bFoundValid && m_deviceIndex < MAX_OPENCV_CAMERA_PORTS)
	{
		if (tryFetchDeviceCapabilities())
		{			
			bFoundValid = true;
		}
		else
		{
			++m_deviceIndex;
		}
	}

	return bFoundValid;
}

bool OpenCVCameraEnumerator::tryFetchDeviceCapabilities()
{
	m_currentDeviceCapabilities.reset();

	cv::VideoCapture videoSource;
	if (videoSource.open(m_deviceIndex))
	{
		cv::Mat frame;
		if (videoSource.read(frame))
		{
			char szDeviceName[128];
			StringUtils::formatString(
				szDeviceName, sizeof(szDeviceName), 
				"OpenCV_%s_%d", 
				videoSource.getBackendName().c_str(),
				m_deviceIndex);

			m_devicePath= szDeviceName;

			m_currentDeviceCapabilities = std::make_shared<VideoCapabilitiesConfig>(m_devicePath);
			m_currentDeviceCapabilities->friendlyName = m_devicePath;
			m_currentDeviceCapabilities->deviceType = eDeviceType::MonoVideoSource;

			VideoModeConfig videoMode;
			videoMode.frameRate = (float)videoSource.get(cv::CAP_PROP_FPS);
			videoMode.isFrameMirrored = false;
			videoMode.isBufferMirrored = false;
			videoMode.bufferPixelWidth = (int)videoSource.get(cv::CAP_PROP_FRAME_WIDTH);
			videoMode.bufferPixelHeight = (int)videoSource.get(cv::CAP_PROP_FRAME_HEIGHT);
			// 4 character string property packed into a double. Gross.
			double formatValue = videoSource.get(cv::CAP_PROP_FOURCC);
			videoMode.bufferFormat= ((char*)(&formatValue));
			videoMode.frameSections.push_back({0, 0});
			// Invalid camera intrinsics
			memset(&videoMode.intrinsics, sizeof(MikanVideoSourceIntrinsics), 0);

			char szModeName[128];
			StringUtils::formatString(
				szModeName, sizeof(szModeName),
				"%s_%dx%d@%.2ffps",
				videoMode.bufferFormat.c_str(),
				videoMode.bufferPixelWidth,
				videoMode.bufferPixelHeight,
				videoMode.frameRate);
			videoMode.modeName = szModeName;

			m_currentDeviceCapabilities->supportedModes.push_back(videoMode);
		}

		videoSource.release();
	}

	return m_currentDeviceCapabilities != nullptr;
}