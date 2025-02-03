// -- includes -----
#include "VideoDeviceEnumerator.h"
#include "GStreamerCameraEnumerator.h"
#ifdef _WIN32
#include "WMFCameraEnumerator.h"
#endif
#include "OpenCVCameraEnumerator.h"

#include "assert.h"
#include "string.h"

// -- globals -----

// -- TrackerDeviceEnumerator -----
VideoDeviceEnumerator::VideoDeviceEnumerator()
	: DeviceEnumerator()
	, m_enumeratorIndex(0)
    , m_cameraIndex(-1)
{
#ifdef _WIN32
	m_enumerators.push_back({eVideoDeviceApi::WMF, nullptr});
#else
	m_enumerators.push_back({eVideoDeviceApi::OPENCV, nullptr});
#endif
	m_enumerators.push_back({eVideoDeviceApi::GSTREAMER, nullptr});

	allocateChildEnumerator();

	if (isValid())
	{
        m_cameraIndex= 0;
	}
	else
    {
        next();
    }
}

VideoDeviceEnumerator::~VideoDeviceEnumerator()
{
	for (auto &entry : m_enumerators)
	{
		delete entry.enumerator;
	}
}

const char *VideoDeviceEnumerator::getDevicePath() const
{
    return 
		isValid()
		? m_enumerators[m_enumeratorIndex].enumerator->getDevicePath()
		: nullptr;
}

eDeviceType VideoDeviceEnumerator::getDeviceType() const
{
	return
		isValid()
		? m_enumerators[m_enumeratorIndex].enumerator->getDeviceType()
		: eDeviceType::INVALID;
}

int VideoDeviceEnumerator::getUsbVendorId() const
{
	return 
		isValid()
		? m_enumerators[m_enumeratorIndex].enumerator->getUsbVendorId()
		: -1;
}

int VideoDeviceEnumerator::getUsbProductId() const
{
	return 
		isValid()
		? m_enumerators[m_enumeratorIndex].enumerator->getUsbProductId()
		: -1;
}

eVideoDeviceApi VideoDeviceEnumerator::getVideoApi() const
{
	return
		isValid()
		? m_enumerators[m_enumeratorIndex].api_type
		: eVideoDeviceApi::INVALID;
}

#ifdef _WIN32
const WMFCameraEnumerator* VideoDeviceEnumerator::getWMFCameraEnumerator() const
{
	return 
		(getVideoApi() == eVideoDeviceApi::WMF)
		? static_cast<WMFCameraEnumerator*>(m_enumerators[m_enumeratorIndex].enumerator)
		: nullptr;
}
#endif // _WIN32

const OpenCVCameraEnumerator* VideoDeviceEnumerator::getOpenCVCameraEnumerator() const
{
	return
		(getVideoApi() == eVideoDeviceApi::OPENCV)
		? static_cast<OpenCVCameraEnumerator*>(m_enumerators[m_enumeratorIndex].enumerator)
		: nullptr;
}

const GStreamerCameraEnumerator* VideoDeviceEnumerator::getGStreamerCameraEnumerator() const
{
	return
		(getVideoApi() == eVideoDeviceApi::GSTREAMER)
		? static_cast<GStreamerCameraEnumerator*>(m_enumerators[m_enumeratorIndex].enumerator)
		: nullptr;
}

bool VideoDeviceEnumerator::isValid() const
{
	if (m_enumeratorIndex < m_enumerators.size())
	{
		if (m_enumerators[m_enumeratorIndex].enumerator != nullptr)
		{
			return m_enumerators[m_enumeratorIndex].enumerator->isValid();
		}
	}

	return false;
}

bool VideoDeviceEnumerator::next()
{
    bool foundValid = false;

    while (!foundValid && m_enumeratorIndex < m_enumerators.size())
    {
		if (isValid())
		{
            ++m_cameraIndex;

			m_enumerators[m_enumeratorIndex].enumerator->next();
			foundValid = m_enumerators[m_enumeratorIndex].enumerator->isValid();
		}
		else
		{
			++m_enumeratorIndex;

			if (m_enumeratorIndex < m_enumerators.size())
			{
				m_cameraIndex = 0;
				allocateChildEnumerator();
				foundValid = m_enumerators[m_enumeratorIndex].enumerator->isValid();
			}
		}
    }

    return foundValid;
}

void VideoDeviceEnumerator::allocateChildEnumerator()
{
	EnumeratorEntry &entry= m_enumerators[m_enumeratorIndex];

    switch (entry.api_type)
	{
	case eVideoDeviceApi::OPENCV:
		entry.enumerator = new OpenCVCameraEnumerator;
		break;
	case eVideoDeviceApi::WMF:
		#ifdef _WIN32
		entry.enumerator = new WMFCameraEnumerator;
		#endif
		break;
	case eVideoDeviceApi::GSTREAMER:
		entry.enumerator = new GStreamerCameraEnumerator;
		break;
	}
}