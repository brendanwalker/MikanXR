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
#include <chrono>

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
    // Poll for async manager init completion
    if (m_networkVideoManagerState == eNetworkVideoManagerState::initializing)
    {
        if (m_networkVideoManagerFuture.wait_for(std::chrono::seconds(0)) == std::future_status::ready)
        {
            auto result = m_networkVideoManagerFuture.get();
            if (result.manager)
            {
                m_networkVideoDeviceModule = result.module;
                m_networkVideoDeviceManager = result.manager;
                m_networkVideoManagerState = eNetworkVideoManagerState::ready;
                MIKAN_LOG_INFO("NetworkVideoSourceSystem::update")
                    << "Network video device manager is ready";

                // Retry openVideoSource() on any components that were waiting
                for (const auto& [id, weakComp] : Super::getComponentMap())
                {
                    if (auto comp = weakComp.lock(); comp && comp->isPendingOpen())
                        comp->openVideoSource();
                }
            }
            else
            {
                m_networkVideoManagerState = eNetworkVideoManagerState::failed;
                MIKAN_LOG_ERROR("NetworkVideoSourceSystem::update")
                    << "Async network video device manager init failed";
            }
        }
    }

    Super::update(deltaTime);

    if (m_networkVideoDeviceManager)
    {
        m_networkVideoDeviceManager->update(deltaTime);
	}
}

void NetworkVideoSourceSystem::dispose()
{
    // If async init is still in-flight, block until it completes so we can
    // safely clean up whatever it allocated
    if (m_networkVideoManagerFuture.valid())
    {
        auto result = m_networkVideoManagerFuture.get();
        m_networkVideoDeviceModule = result.module;
        m_networkVideoDeviceManager = result.manager;
    }

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
	switch (m_networkVideoManagerState)
	{
	case eNetworkVideoManagerState::ready:
		return true;
	case eNetworkVideoManagerState::initializing:
		return false;
	case eNetworkVideoManagerState::failed:
		return false;
	default:
		break;
	}

	// Bail if we didn't select a valid runtime type to use
	const std::string moduleName = NETWORK_VIDEO_DEVICE_MODULE_NAME;
	if (moduleName.empty())
	{
		m_networkVideoManagerState = eNetworkVideoManagerState::failed;
		return false;
	}

	// Special case: If GStreamer is selected, see if it is installed.
	// Do this cheap check on the main thread before launching the async task.
	if (moduleName == GSTREAMER_VIDEO_DEVICE_MODULE_NAME)
	{
		const char* envVar = std::getenv("GSTREAMER_1_0_ROOT_MINGW_X86_64");
		if (envVar == nullptr)
		{
			MIKAN_LOG_WARNING("NetworkVideoSourceSystem::ensureNetworkDeviceManager")
				<< "GStreamer not installed. Skipping network video manager init.";
			// Not a failure — just no manager available
			m_networkVideoManagerState = eNetworkVideoManagerState::failed;
			return false;
		}
	}

	// Launch async init to avoid blocking the main thread
	MIKAN_LOG_INFO("NetworkVideoSourceSystem::ensureNetworkDeviceManager")
		<< "Launching async init for network video device module " << moduleName;
	m_networkVideoManagerState = eNetworkVideoManagerState::initializing;
	m_networkVideoManagerFuture = std::async(
		std::launch::async,
		&NetworkVideoSourceSystem::initNetworkVideoDeviceManagerOnThread,
		moduleName);

	return false;
}

NetworkVideoSourceSystem::NetworkVideoDeviceManagerInitResult
NetworkVideoSourceSystem::initNetworkVideoDeviceManagerOnThread(const std::string& moduleName)
{
	NetworkVideoDeviceManagerInitResult result;

	// Attempt to load the video device module
	result.module = getMikanModuleManager()->getModule<INetworkVideoDeviceModule>(moduleName);
	if (result.module)
	{
		MIKAN_LOG_INFO("NetworkVideoSourceSystem::initNetworkVideoDeviceManagerOnThread")
			<< "Loaded module " << moduleName;

		// Attempt to create a device manager
		result.manager = result.module->createNetworkVideoDeviceManager();
		if (result.manager)
		{
			MIKAN_LOG_INFO("NetworkVideoSourceSystem::initNetworkVideoDeviceManagerOnThread")
				<< "Allocated network video device manager for " << moduleName;

			// Attempt to startup the network video device manager
			if (result.manager->startup())
			{
				MIKAN_LOG_INFO("NetworkVideoSourceSystem::initNetworkVideoDeviceManagerOnThread")
					<< "Started NetworkVideoDeviceManger for " << moduleName;

				return result;
			}
			else
			{
				MIKAN_LOG_WARNING("NetworkVideoSourceSystem::initNetworkVideoDeviceManagerOnThread")
					<< "Failed to startup NetworkVideoDeviceManger for " << moduleName;
			}
		}
		else
		{
			MIKAN_LOG_WARNING("NetworkVideoSourceSystem::initNetworkVideoDeviceManagerOnThread")
				<< "Failed to allocate NetworkVideoDeviceManger for " << moduleName;
		}
	}
	else
	{
		MIKAN_LOG_ERROR("NetworkVideoSourceSystem::initNetworkVideoDeviceManagerOnThread")
			<< "Failed to load module " << moduleName;
	}

	// Clean up on failure
	if (result.manager)
	{
		result.manager->shutdown();
		result.manager = nullptr;
	}
	if (result.module)
	{
		getMikanModuleManager()->disposeModule(result.module);
		result.module = nullptr;
	}

	return result;
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