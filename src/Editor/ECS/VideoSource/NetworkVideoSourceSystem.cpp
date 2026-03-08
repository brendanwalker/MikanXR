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

// -- NetworkVideoSourceSystemDefinition -----
NetworkVideoSourceSystemDefinition::NetworkVideoSourceSystemDefinition(
	const std::string& configName, IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

configuru::Config NetworkVideoSourceSystemDefinition::writeToJSON()
{
	configuru::Config pt = Super::writeToJSON();

	return pt;
}

void NetworkVideoSourceSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	Super::readFromJSON(pt);
}

// -- NetworkVideoSourceSystem -----
NetworkVideoSourceSystem::NetworkVideoSourceSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}

void NetworkVideoSourceSystem::update(float deltaTime)
{
    Super::update(deltaTime);

    if (m_networkVideoDeviceManager)
    {
        m_networkVideoDeviceManager->update(deltaTime);
	}
}

void NetworkVideoSourceSystem::dispose()
{
	Super::dispose();
}

VideoSourceIdList NetworkVideoSourceSystem::getVideoSourceIdList() const
{
	VideoSourceIdList videoSourceIdList;
	for (const auto& it : Super::getComponentMap())
	{
        NetworkVideoSourceComponentPtr componentPtr = it.second.lock();
		if (componentPtr)
		{
			videoSourceIdList.push_back(componentPtr->getVideoSourceId());
		}
	}
	return videoSourceIdList;
}

bool NetworkVideoSourceSystem::ensureNetworkDeviceManager()
{
	if (m_networkVideoDeviceManager)
		return true;

	if (!createNetworkVideoDeviceManager(NETWORK_VIDEO_DEVICE_MODULE_NAME))
	{
		MIKAN_LOG_ERROR("NetworkVideoSourceSystem::init") <<
			"Failed to load network video device module " << NETWORK_VIDEO_DEVICE_MODULE_NAME;
		return false;
	}

	return true;
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

void NetworkVideoSourceSystem::additionalComponentFactory(
	MikanObjectPtr ownerComponentObject,
	NetworkVideoSourceDefinitionPtr componentDefinition)
{
	// Lazy create a network video device manager, if possible
	ensureNetworkDeviceManager();
}