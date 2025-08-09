#include "App.h"
#include "MikanObject.h"
#include "NetworkVideoSourceSystem.h"
#include "NetworkVideoSourceComponent.h"
#include "ProjectConfig.h"
#include "VideoSourceSystemConfig.h"
#include "VideoSourceSystem.h"

#include <assert.h>

bool NetworkVideoSourceSystem::init()
{
	MikanObjectSystem::init();

    VideoSourceSystemConfigConstPtr videoSourceSystemConfig = 
        App::getInstance()->getProfileConfig()->videoSourceSystemConfig;

    for (const auto& sourceConfig : videoSourceSystemConfig->getNetworkedVideoSourceList())
    {
        createNetworkVideoSourceObject(sourceConfig);
    }

    return true;
}

void NetworkVideoSourceSystem::dispose()
{
    m_networkVideoSourceComponents.clear();
	MikanObjectSystem::dispose();
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

NetworkVideoSourceComponentPtr NetworkVideoSourceSystem::addNewNetworkVideoSource(
    const MikanNetworkVideoSourceInfo& videoSourceInfo)
{
    VideoSourceSystemConfigPtr videoSourceSystemConfig = 
        App::getInstance()->getProfileConfig()->videoSourceSystemConfig;

    MikanVideoSourceID videoSourceId = videoSourceSystemConfig->addNetworkedVideoSource(videoSourceInfo);
    if (videoSourceId != INVALID_MIKAN_ID)
    {
        NetworkVideoSourceDefinitionPtr configPtr = videoSourceSystemConfig->getNetworkedVideoSourceConfig(videoSourceId);
        assert(configPtr != nullptr);

        return createNetworkVideoSourceObject(configPtr);
    }

    return NetworkVideoSourceComponentPtr();
}

bool NetworkVideoSourceSystem::removeNetworkVideoSource(MikanVideoSourceID videoSourceId)
{
    VideoSourceSystemConfigPtr videoSourceSystemConfig = 
        App::getInstance()->getProfileConfig()->videoSourceSystemConfig;

    return
        disposeNetworkVideoSourceObject(videoSourceId) &&
        videoSourceSystemConfig->removeVideoSource(videoSourceId);
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
