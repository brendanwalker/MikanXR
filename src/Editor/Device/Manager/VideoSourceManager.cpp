//-- includes -----
#include "VideoSourceManager.h"
#include "VideoDeviceEnumerator.h"
#include "DeviceManager.h"
#include "DeviceView.h"
#include "Logger.h"
#include "MathUtility.h"
#include "PathUtils.h"
#include "VideoSourceView.h"
#include "VideoCapabilitiesConfig.h"
#include "WMFCameraEnumerator.h"
#include "IMkWindow.h"

#include <cstdlib>
#include <fstream>
#include <type_traits>

#include <easy/profiler.h>

#include "dylib.hpp"
#include "MikanGStreamerModule.h"

VideoSourceManager* VideoSourceManager::m_instance= nullptr;

//-- Video Source Manager Config -----
VideoSourceManagerConfig::VideoSourceManagerConfig(const std::string& fnamebase)
	: CommonConfig(fnamebase)
{

};

configuru::Config VideoSourceManagerConfig::writeToJSON()
{
	configuru::Config pt= CommonConfig::writeToJSON();

	writeStdValueVector<std::string>(pt, "video_source_uris", videoSourceURIs);

	return pt;
}

void VideoSourceManagerConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	readStdValueVector<std::string>(pt, "video_source_uris", videoSourceURIs);
}

//-- Video Source Manager Private Implementation -----
class MikanGStreamerPlugin
{
public:
	using AllocModuleFunctionPtr = std::add_pointer<IMikanGStreamerModule*()>::type;
	using FreeModuleFunctionPtr = std::add_pointer<void(IMikanGStreamerModule*)>::type;

	MikanGStreamerPlugin()
		: m_gstreamerLib(nullptr)
		, m_allocModule(nullptr)
		, m_freeModule(nullptr)
		, m_moduleInterface(nullptr)
	{
	}

	~MikanGStreamerPlugin()
	{
		shutdown();	
	}

	inline IMikanGStreamerModule* getModuleInterface() const
	{
		return m_moduleInterface;
	}

	bool startup()
	{
		bool success = false;

		try
		{
			// Assume plugin module is in the same place as the executable
			std::string modulePath = PathUtils::getModulePath().string();

			// Load the GStreamer library
			m_gstreamerLib = new dylib(modulePath.c_str(), "MikanGStreamer");

			// Fetch the functions we need
			m_allocModule= m_gstreamerLib->get_function<IMikanGStreamerModule*()>("AllocatePluginModule");
			m_freeModule= m_gstreamerLib->get_function<void(IMikanGStreamerModule*)>("FreePluginModule");
			
			// Attempt to allocate the module
			m_moduleInterface= m_allocModule();

			// Attempt to initiate the module
			if (m_moduleInterface != nullptr)
			{
				success= m_moduleInterface->startup();
				if (!success)
				{
					MIKAN_LOG_ERROR("VideoSourceManagerImpl::startup") << "Failed to initialize GStreamer module!";
				}
			}
		}
		catch (std::exception* e)
		{
			MIKAN_LOG_ERROR("VideoSourceManagerImpl::startup") << "Failed to load GStreamer module: " << e->what();
			return false;
		}

		if (!success)
		{
			shutdown();
		}

		return success;
	}

	void shutdown()
	{
		// Clean up the GStreamer library
		if (m_gstreamerLib != nullptr)
		{
			if (m_freeModule != nullptr)
			{
				if (m_moduleInterface != nullptr)
				{
					m_freeModule(m_moduleInterface);
				}
			}

			delete m_gstreamerLib;
		}

		m_gstreamerLib = nullptr;
		m_allocModule = nullptr;
		m_freeModule = nullptr;
		m_moduleInterface = nullptr;
	}

private:
	dylib* m_gstreamerLib;
	AllocModuleFunctionPtr m_allocModule;
	FreeModuleFunctionPtr m_freeModule;
	IMikanGStreamerModule* m_moduleInterface;
};

//-- Video Source Manager -----
VideoSourceManager::VideoSourceManager()
	: DeviceManager()
	, m_mikanGStreamerPlugin(new MikanGStreamerPlugin)
	, m_cfg()
	, m_supportedTrackers(new VideoCapabilitiesSet)
{
	// Share the supported tracker list with the tracker enumerators
	WMFCameraEnumerator::s_videoCapabilitiesSet = m_supportedTrackers;
}

VideoSourceManager::~VideoSourceManager()
{
	delete m_mikanGStreamerPlugin;
}

bool VideoSourceManager::startup(IMkWindow *ownerWindow)
{
	EASY_FUNCTION();

	bool bSuccess = DeviceManager::startup(ownerWindow);

	if (bSuccess)
	{
		// Set the singleton pointer
		m_instance = this;

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

		// Attempt to load the GStreamer module
		const char* envVar = std::getenv("GSTREAMER_1_0_ROOT_MINGW_X86_64");
		if (envVar != nullptr)
		{
			MIKAN_LOG_INFO("VideoSourceManager::startup") << "Found GStreamer install: " << envVar;

			if (!m_mikanGStreamerPlugin->startup())
			{
				MIKAN_LOG_WARNING("VideoSourceManager::startup") << "Failed to load any GStreamer library!";
			}
		}
		else
		{
			MIKAN_LOG_WARNING("VideoSourceManager::startup") << "GStreamer not installed. Skipping MikanGStreamer module load.";
		}

		// Refresh the tracker list
		updateConnectedDeviceViews();
	}

	return bSuccess;
}

void VideoSourceManager::update(float deltaTime)
{
	EASY_FUNCTION();

	DeviceManager::update(deltaTime);

	// Update any video sources that do their processing on the main thread
	for (int videoSourceId = 0; videoSourceId < k_max_devices; ++videoSourceId)
	{
		VideoSourceViewPtr videoSourceView = getVideoSourceViewPtr(videoSourceId);
		IVideoSourceInterface* videoSourceInterface= videoSourceView->getVideoSourceInterface();

		if (videoSourceInterface != nullptr && videoSourceInterface->wantsUpdate())
		{
			videoSourceInterface->update(deltaTime);
		}
	}
}

void VideoSourceManager::shutdown()
{
	// Close all video sources
	DeviceManager::shutdown();

	// Close down the GStreamer plugin after all streams are stopped
	m_mikanGStreamerPlugin->shutdown();

	m_instance = nullptr;
}

class IMikanGStreamerModule* VideoSourceManager::getGStreamerModule() const
{
	return m_mikanGStreamerPlugin->getModuleInterface();
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

VideoSourceViewPtr VideoSourceManager::getVideoSourceViewByPath(const std::string& devicePath) const
{
	for (int videoSourceId = 0; videoSourceId < k_max_devices; ++videoSourceId)
	{
		VideoSourceViewPtr videoSourcePtr = getVideoSourceViewPtr(videoSourceId);

		if (videoSourcePtr->getIsOpen() && videoSourcePtr->getUSBDevicePath() == devicePath)
		{
			return videoSourcePtr;
		}	
	}

	return VideoSourceViewPtr();
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