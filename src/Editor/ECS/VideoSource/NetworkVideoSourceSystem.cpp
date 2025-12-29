#include "App.h"
#include "INetworkVideoDeviceManager.h"
#include "INetworkVideoDeviceModule.h"
#include "Logger.h"
#include "MikanObject.h"
#include "MikanModuleManager.h"
#include "NetworkVideoSourceSystem.h"
#include "NetworkVideoSourceComponent.h"
#include "ProjectConfig.h"
#include "VideoSourceSystemConfig.h"
#include "VideoSourceSystem.h"

#include <assert.h>

#define GSTREAMER_VIDEO_DEVICE_MODULE_NAME  "MikanGStreamerVideo"
#define NETWORK_VIDEO_DEVICE_MODULE_NAME    GSTREAMER_VIDEO_DEVICE_MODULE_NAME

bool NetworkVideoSourceSystem::init()
{
	MikanObjectSystem::init();

    VideoSourceSystemConfigConstPtr videoSourceSystemConfig = getProjectConfig()->videoSourceSystemConfig;

	if (!createNetworkVideoDeviceManager(NETWORK_VIDEO_DEVICE_MODULE_NAME))
	{
		MIKAN_LOG_ERROR("NetworkVideoSourceSystem::init") <<
			"Failed to load network video device module " << NETWORK_VIDEO_DEVICE_MODULE_NAME;
		return false;
	}

    for (const auto& sourceConfig : videoSourceSystemConfig->getNetworkedVideoSourceList())
    {
        createNetworkVideoSourceObject(sourceConfig);
    }

    return true;
}

void NetworkVideoSourceSystem::update(float deltaTime)
{
    MikanObjectSystem::update(deltaTime);

    if (m_networkVideoDeviceManager)
    {
        m_networkVideoDeviceManager->update(deltaTime);
	}
}

void NetworkVideoSourceSystem::dispose()
{
    m_networkVideoSourceComponents.clear();
	MikanObjectSystem::dispose();
}

VideoSourceIdList NetworkVideoSourceSystem::getVideoSourceIdList() const
{
	VideoSourceIdList videoSourceIdList;
	for (const auto& it : m_networkVideoSourceComponents)
	{
        NetworkVideoSourceComponentPtr componentPtr = it.second.lock();
		if (componentPtr)
		{
			videoSourceIdList.push_back(componentPtr->getVideoSourceId());
		}
	}
	return videoSourceIdList;
}

NetworkVideoSourceComponentPtr NetworkVideoSourceSystem::getNetworkVideoSourceById(MikanVideoSourceID videoSourceId) const
{
    auto iter = m_networkVideoSourceComponents.find(videoSourceId);
    if (iter != m_networkVideoSourceComponents.end())
    {
        return iter->second.lock();
    }

    return NetworkVideoSourceComponentPtr();
}

NetworkVideoSourceComponentPtr NetworkVideoSourceSystem::getNetworkVideoSourceByName(const std::string& videoSourceName) const
{
    for (auto it = m_networkVideoSourceComponents.begin(); it != m_networkVideoSourceComponents.end(); it++)
    {
        NetworkVideoSourceComponentPtr componentPtr = it->second.lock();

        if (componentPtr && componentPtr->getDefinition()->getComponentName() == videoSourceName)
        {
            return componentPtr;
        }
    }

    return NetworkVideoSourceComponentPtr();
}

NetworkVideoSourceComponentPtr NetworkVideoSourceSystem::addNewNetworkVideoSource()
{
    MikanNetworkVideoSourceInfo defaultInfo = {};
	defaultInfo.network_source_name.setValue("New Network Video Source");
    defaultInfo.url.setValue("rtsp://<username>:<password>@<IP_address>:<port>/<stream_path>");
    defaultInfo.intrinsics.intrinsics_type = INVALID_CAMERA_INTRINSICS;

	return addNewNetworkVideoSource(defaultInfo);
}

NetworkVideoSourceComponentPtr NetworkVideoSourceSystem::addNewNetworkVideoSource(
    const MikanNetworkVideoSourceInfo& videoSourceInfo)
{
    VideoSourceSystemConfigPtr videoSourceSystemConfig =
        getProjectConfig()->videoSourceSystemConfig;

    return
        createNetworkVideoSourceObject(
            videoSourceSystemConfig->allocateNetworkedVideoSourceDefinition(videoSourceInfo));
}

bool NetworkVideoSourceSystem::removeNetworkVideoSource(MikanVideoSourceID videoSourceId)
{
    VideoSourceSystemConfigPtr videoSourceSystemConfig = 
        getProjectConfig()->videoSourceSystemConfig;

    return
        disposeNetworkVideoSourceObject(videoSourceId) &&
        videoSourceSystemConfig->removeVideoSource(videoSourceId);
}

bool NetworkVideoSourceSystem::createNetworkVideoDeviceManager(const std::string& moduleName)
{
	// Bail if we didn't select a valid runtime type to use
	if (moduleName.empty())
		return false;

	// Special case: If GStreamer is selected, see if it is installed
    if (moduleName == GSTREAMER_VIDEO_DEVICE_MODULE_NAME)
    {
		const char* envVar = std::getenv("GSTREAMER_1_0_ROOT_MINGW_X86_64");
        if (envVar == nullptr)
        {
            MIKAN_LOG_WARNING("NetworkVideoSourceSystem::createNetworkVideoDeviceManager") 
                << "GStreamer not installed. Skipping network video manager init.";

			// Don't treat this as a failure, just skip module loading
            return true;
        }
    }

	// Attempt to load the video device module
    m_networkVideoDeviceModule = getMikanModuleManager()->getModule<INetworkVideoDeviceModule>(moduleName);
	if (!m_networkVideoDeviceModule)
	{
		MIKAN_LOG_ERROR("NetworkVideoSourceSystem::createNetworkVideoDeviceManager") 
            << "Failed to load module" << moduleName;
		return false;
	}

	// Attempt to create a vr device manager
    m_networkVideoDeviceManager = m_networkVideoDeviceModule->createNetworkVideoDeviceManager();
	if (!m_networkVideoDeviceManager)
	{
		MIKAN_LOG_WARNING("NetworkVideoSourceSystem::createNetworkVideoDeviceManager") 
            << "Failed to create UsbVideoDeviceManager";
		return false;
	}

    return true;
}

void NetworkVideoSourceSystem::disposeNetworkVideoDeviceManager()
{
	if (m_networkVideoDeviceManager)
	{
        m_networkVideoDeviceManager->shutdown();
        m_networkVideoDeviceManager = nullptr;
	}

	if (m_networkVideoDeviceModule)
	{
		getMikanModuleManager()->disposeModule(m_networkVideoDeviceModule);
        m_networkVideoDeviceModule = nullptr;
	}
}

NetworkVideoSourceComponentPtr NetworkVideoSourceSystem::createNetworkVideoSourceObject(
    NetworkVideoSourceDefinitionPtr videoSourceDefinition)
{
    MikanObjectPtr videoSourceObject = newObject();
    videoSourceObject->setName(videoSourceDefinition->getComponentName());

    // Make the NetworkVideoSource component the root of the object
    auto videoSourceComponentPtr = videoSourceObject->addComponent<NetworkVideoSourceComponent>();
    videoSourceComponentPtr->setDefinition(videoSourceDefinition);

    // Init the object once all components are added
    videoSourceObject->init();

    // Keep track of all the network video sources in the system
    m_networkVideoSourceComponents.insert({ videoSourceDefinition->getVideoSourceId(), videoSourceComponentPtr });

    // Register the definition with the video source system
    getProjectConfig()->videoSourceSystemConfig->addNetworkedVideoSourceDefinition(videoSourceDefinition);

    return videoSourceComponentPtr;
}

bool NetworkVideoSourceSystem::disposeNetworkVideoSourceObject(MikanVideoSourceID videoSourceId)
{
    auto it = m_networkVideoSourceComponents.find(videoSourceId);
    if (it != m_networkVideoSourceComponents.end())
    {
        NetworkVideoSourceComponentPtr stencilComponentPtr = it->second.lock();

        // Remove for component list
        m_networkVideoSourceComponents.erase(it);

        // Free the corresponding object
        deleteObject(stencilComponentPtr->getOwnerObject());

        return true;
    }

    return false;
}
