#include "App.h"
#include "INetworkVideoDeviceManager.h"
#include "INetworkVideoDeviceModule.h"
#include "Logger.h"
#include "MikanObject.h"
#include "MikanModuleManager.h"
#include "MikanPropertyDatabase.h"
#include "NetworkVideoSourceSystem.h"
#include "NetworkVideoSourceComponent.h"
#include "ProjectConfig.h"

#include <assert.h>

#define GSTREAMER_VIDEO_DEVICE_MODULE_NAME  "MikanGStreamerVideo"
#define NETWORK_VIDEO_DEVICE_MODULE_NAME    GSTREAMER_VIDEO_DEVICE_MODULE_NAME

// -- VideoSourceSystemConfig -----
const std::string NetworkVideoSourceSystemConfig::k_networkedVideoSourceListPropertyId = "networkVideoSourceList";

configuru::Config NetworkVideoSourceSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["next_video_source_id"] = m_nextVideoSourceId;

	std::vector<configuru::Config> networkedVideoSourceConfigs;
	for (auto videoSource : m_networkedVideoSourceList)
	{
		networkedVideoSourceConfigs.push_back(videoSource->writeToJSON());
	}
	pt.insert_or_assign(std::string("networked_video_sources"), networkedVideoSourceConfigs);

	return pt;
}

void NetworkVideoSourceSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_nextVideoSourceId = pt.get_or<int>("next_video_source_id", m_nextVideoSourceId);

	// Read in the networked video sources
	m_networkedVideoSourceList.clear();
	if (pt.has_key("networked_video_sources"))
	{
		for (const configuru::Config& videoSource_pt : pt["networked_video_sources"].as_array())
		{
			auto definitionPtr = std::make_shared<NetworkVideoSourceDefinition>();

			definitionPtr->readFromJSON(videoSource_pt);
			m_networkedVideoSourceList.push_back(definitionPtr);

			addChildConfig(definitionPtr);
		}
	}
}

NetworkVideoSourceDefinitionConstPtr NetworkVideoSourceSystemConfig::getNetworkedVideoSourceConfigConst(
	MikanVideoSourceID videoSourceId) const
{
	auto it = std::find_if(
		m_networkedVideoSourceList.begin(), m_networkedVideoSourceList.end(),
		[videoSourceId](NetworkVideoSourceDefinitionPtr configPtr) {
			return configPtr->getVideoSourceId() == videoSourceId;
		});

	if (it != m_networkedVideoSourceList.end())
	{
		return NetworkVideoSourceDefinitionConstPtr(*it);
	}

	return NetworkVideoSourceDefinitionConstPtr();
}

NetworkVideoSourceDefinitionPtr NetworkVideoSourceSystemConfig::getNetworkedVideoSourceConfig(
	MikanVideoSourceID videoSourceId)
{
	return std::const_pointer_cast<NetworkVideoSourceDefinition>(
		getNetworkedVideoSourceConfigConst(videoSourceId));
}

NetworkVideoSourceDefinitionPtr NetworkVideoSourceSystemConfig::allocateNetworkedVideoSourceDefinition(
	const MikanNetworkVideoSourceInfo& videoSourceInfo)
{
	NetworkVideoSourceDefinitionPtr videoSourcePtr =
		std::make_shared<NetworkVideoSourceDefinition>(m_nextVideoSourceId, videoSourceInfo);
	m_nextVideoSourceId++;

	return videoSourcePtr;
}

bool NetworkVideoSourceSystemConfig::addNetworkedVideoSourceDefinition(
	NetworkVideoSourceDefinitionPtr videoSourceDefinitionPtr)
{
	if (!getNetworkedVideoSourceConfig(videoSourceDefinitionPtr->getVideoSourceId()))
	{
		m_networkedVideoSourceList.push_back(videoSourceDefinitionPtr);
		addChildConfig(videoSourceDefinitionPtr);

		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_networkedVideoSourceListPropertyId));
		return true;
	}

	return false;
}

bool NetworkVideoSourceSystemConfig::removeNetworkedVideoSourceDefinition(MikanVideoSourceID videoSourceId)
{
	auto it = std::find_if(
		m_networkedVideoSourceList.begin(), m_networkedVideoSourceList.end(),
		[videoSourceId](NetworkVideoSourceDefinitionPtr configPtr) {
			return configPtr->getVideoSourceId() == videoSourceId;
		});

	if (it != m_networkedVideoSourceList.end())
	{
		removeChildConfig(*it);

		m_networkedVideoSourceList.erase(it);
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_networkedVideoSourceListPropertyId));

		return true;
	}

	return false;
}

// -- NetworkVideoSourceSystem -----
NetworkVideoSourceSystemConfigConstPtr NetworkVideoSourceSystem::getNetworkVideoSourceSystemConfigConst() const
{
	return std::static_pointer_cast<const NetworkVideoSourceSystemConfig>(getDefinitionConst());
}

NetworkVideoSourceSystemConfigPtr NetworkVideoSourceSystem::getNetworkVideoSourceSystemConfig()
{
	return std::static_pointer_cast<NetworkVideoSourceSystemConfig>(getDefinition());
}

bool NetworkVideoSourceSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
	MikanObjectSystem::init(definitionPtr);

    NetworkVideoSourceSystemConfigConstPtr videoSourceSystemConfig = 
		getProjectConfig()->networkVideoSourceSystemConfig;

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

MikanComponentPtr NetworkVideoSourceSystem::getComponentById(int componentId) const
{
	return getNetworkVideoSourceById(componentId);
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
	defaultInfo.network_source_name.setValue("NetworkVideo");
    defaultInfo.url.setValue("");
    defaultInfo.intrinsics.intrinsics_type = INVALID_CAMERA_INTRINSICS;

	return addNewNetworkVideoSource(defaultInfo);
}

NetworkVideoSourceComponentPtr NetworkVideoSourceSystem::addNewNetworkVideoSource(
    const MikanNetworkVideoSourceInfo& videoSourceInfo)
{
    NetworkVideoSourceSystemConfigPtr videoSourceSystemConfig =
        getProjectConfig()->networkVideoSourceSystemConfig;

    return
        createNetworkVideoSourceObject(
            videoSourceSystemConfig->allocateNetworkedVideoSourceDefinition(videoSourceInfo));
}

bool NetworkVideoSourceSystem::removeNetworkVideoSource(MikanVideoSourceID videoSourceId)
{
    NetworkVideoSourceSystemConfigPtr videoSourceSystemConfig = 
        getProjectConfig()->networkVideoSourceSystemConfig;

    return
        disposeNetworkVideoSourceObject(videoSourceId) &&
        videoSourceSystemConfig->removeNetworkedVideoSourceDefinition(videoSourceId);
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
	bool bSuccess = false;
	m_networkVideoDeviceModule = getMikanModuleManager()->getModule<INetworkVideoDeviceModule>(moduleName);
	if (m_networkVideoDeviceModule)
	{
		MIKAN_LOG_INFO("NetworkVideoSourceSystem::createNetworkVideoDeviceManager")
			<< "Loaded module " << moduleName;

		// Attempt to create a device manager
		m_networkVideoDeviceManager = m_networkVideoDeviceModule->createNetworkVideoDeviceManager();
		if (m_networkVideoDeviceManager)
		{
			MIKAN_LOG_INFO("NetworkVideoSourceSystem::createNetworkVideoDeviceManager")
				<< "Allocated network video device manager for " << moduleName;

			// Attempt to startup the network video device manager
			if (m_networkVideoDeviceManager->startup())
			{
				MIKAN_LOG_INFO("NetworkVideoSourceSystem::createNetworkVideoDeviceManager")
					<< "Started NetworkVideoDeviceManger for " << moduleName;

				// Listen for device manager changes
				bSuccess = true;
			}
			else
			{
				MIKAN_LOG_WARNING("NetworkVideoSourceSystem::createNetworkVideoDeviceManager")
					<< "Failed to startup UsbVideoDeviceManger for " << moduleName;
			}
		}
		else
		{
			MIKAN_LOG_WARNING("NetworkVideoSourceSystem::createNetworkVideoDeviceManager")
				<< "Failed to allocate UsbVideoDeviceManger for " << moduleName;
		}
	}
	else
	{
		MIKAN_LOG_ERROR("NetworkVideoSourceSystem::createNetworkVideoDeviceManager")
			<< "Failed to load module" << moduleName;
	}

	// Clean up if anything failed
	if (!bSuccess)
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

	return bSuccess;
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
    getProjectConfig()->networkVideoSourceSystemConfig->addNetworkedVideoSourceDefinition(videoSourceDefinition);

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

void NetworkVideoSourceSystem::registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase)
{
	propertyDatabase->registerPropertiesForSystem<NetworkVideoSourceSystem>();
	propertyDatabase->registerPropertiesForComponent<NetworkVideoSourceSystem, NetworkVideoSourceComponent>();
}