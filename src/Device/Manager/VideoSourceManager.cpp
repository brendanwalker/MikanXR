//-- includes -----
#include "VideoSourceManager.h"
#include "VideoDeviceEnumerator.h"
#include "DeviceManager.h"
#include "DeviceView.h"
#include "Logger.h"
#include "MathUtility.h"
#include "VideoSourceView.h"
#include "VideoCapabilitiesConfig.h"
#include "WMFCameraEnumerator.h"

#include <fstream>

VideoSourceManager* VideoSourceManager::m_instance= nullptr;

//-- Video Source Manager Config -----
VideoSourceManagerConfig::VideoSourceManagerConfig(const std::string& fnamebase)
	: CommonConfig(fnamebase)
{

};

const configuru::Config VideoSourceManagerConfig::writeToJSON()
{
	configuru::Config pt{
		{"rtmp_server_port", rtmp_server_port},
	};

	return pt;
}

void VideoSourceManagerConfig::readFromJSON(const configuru::Config& pt)
{
	rtmp_server_port = pt.get_or<unsigned int>("rtmp_server_port", rtmp_server_port);
}

//-- Video Source Manager -----
VideoSourceManager::VideoSourceManager()
	: DeviceManager()
	, m_cfg()
	, m_supportedTrackers(new VideoCapabilitiesSet)
{
	// Share the supported tracker list with the tracker enumerators
	WMFCameraEnumerator::s_supportedTrackers = m_supportedTrackers;
}

VideoSourceManager::~VideoSourceManager()
{
}

bool VideoSourceManager::startup()
{
	EASY_FUNCTION();

	bool bSuccess = DeviceManager::startup();

	if (bSuccess)
	{
		// Load the config file (if it exists)
		m_cfg.load();

		// Save it back out (in case it doesn't exist yet)
		m_cfg.save();

		// Fetch the config files for all the trackers we support
		if (!m_supportedTrackers->reloadSupportedVideoCapabilities())
		{
			MIKAN_LOG_ERROR("VideoSourceManager::startup") << "Failed to load any tracker capability files!";
			return false;
		}

		// Refresh the tracker list
		updateConnectedDeviceViews();

		// Set the singleton pointer
		m_instance= this;
	}

	return bSuccess;
}

void VideoSourceManager::update(float deltaTime)
{
}

void VideoSourceManager::shutdown()
{
	m_instance = nullptr;

	DeviceManager::shutdown();
}

void VideoSourceManager::closeAllVideoSources()
{
	for (int videoSourceId = 0; videoSourceId < k_max_devices; ++videoSourceId)
	{
		VideoSourceViewPtr videoSourceView = getVideoSourceViewPtr(videoSourceId);

		if (videoSourceView->getIsOpen())
		{
			videoSourceView->close();
		}
	}
}

DeviceEnumerator* VideoSourceManager::allocateDeviceEnumerator()
{
	return new VideoDeviceEnumerator();
}

void VideoSourceManager::freeDeviceEnumerator(DeviceEnumerator* enumerator)
{
	delete static_cast<VideoDeviceEnumerator*>(enumerator);
}

DeviceView* VideoSourceManager::allocateDeviceView(int device_id)
{
	return new VideoSourceView(device_id);
}

VideoSourceViewPtr VideoSourceManager::getVideoSourceViewPtr(int device_id) const
{
	assert(m_deviceViews != nullptr);

	return std::static_pointer_cast<VideoSourceView>(m_deviceViews[device_id]);
}

VideoSourceList VideoSourceManager::getVideoSourceList() const
{
	VideoSourceList result;

	for (int videoSourceId = 0; videoSourceId < k_max_devices; ++videoSourceId)
	{
		VideoSourceViewPtr videoSourcePtr = getVideoSourceViewPtr(videoSourceId);

		if (videoSourcePtr->getIsOpen())
		{
			result.push_back(videoSourcePtr);
		}
	}

	return result;
}

// -- VideoSourceListIterator -----
VideoSourceListIterator::VideoSourceListIterator(const std::string& usbPath)
{
	m_videoSourceList = VideoSourceManager::getInstance()->getVideoSourceList();
	m_listIndex= -1;

	for (int testIndex = 0; testIndex < m_videoSourceList.size(); ++testIndex)
	{
		VideoSourceViewPtr videoSourcePtr = m_videoSourceList[testIndex];

		if (videoSourcePtr->getUSBDevicePath() == usbPath)
		{
			m_listIndex = testIndex;
			break;
		}
	}

	// If no matching video source, was found, just pick the first one
	if (m_listIndex == -1 && m_videoSourceList.size() > 0)
	{
		m_listIndex = 0;
	}
}

VideoSourceViewPtr VideoSourceListIterator::getCurrent() const
{
	return 
		(m_listIndex >= 0 && m_listIndex < m_videoSourceList.size()) 
		? m_videoSourceList[m_listIndex] 
		: VideoSourceViewPtr();
}

bool VideoSourceListIterator::goPrevious()
{
	int sourceCount= (int)m_videoSourceList.size();
	int oldListIndex= m_listIndex;

	m_listIndex=
		(sourceCount > 0)
		? (m_listIndex + sourceCount - 1) % sourceCount
		: -1;

	return m_listIndex != oldListIndex;
}

bool VideoSourceListIterator::goNext()
{
	int sourceCount = (int)m_videoSourceList.size();
	int oldListIndex = m_listIndex;

	m_listIndex =
		(sourceCount > 0)
		? (m_listIndex + 1) % sourceCount
		: -1;

	return m_listIndex != oldListIndex;
}

bool VideoSourceListIterator::goToIndex(int new_index)
{
	int sourceCount = (int)m_videoSourceList.size();

	if (new_index >= 0 && new_index < sourceCount)
	{
		int oldListIndex = m_listIndex;

		m_listIndex= new_index;

		return m_listIndex != oldListIndex;
	}

	return false;
}