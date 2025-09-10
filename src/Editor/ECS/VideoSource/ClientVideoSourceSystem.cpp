#include "ClientVideoSourceSystem.h"
#include "VideoSourceSystem.h"
#include "ClientVideoSourceComponent.h"
#include "App.h"
#include "ProjectConfig.h"
#include "VideoSourceSystemConfig.h"
#include "MikanObject.h"

#include <assert.h>

bool ClientVideoSourceSystem::init()
{
	MikanObjectSystem::init();

    VideoSourceSystemConfigConstPtr videoSourceSystemConfig = 
        App::getInstance()->getProfileConfig()->videoSourceSystemConfig;

    for (const auto& sourceConfig : videoSourceSystemConfig->getClientVideoSourceList())
    {
        createClientVideoSourceObject(sourceConfig);
    }

    return true;
}

void ClientVideoSourceSystem::dispose()
{
    m_clientVideoSourceComponents.clear();
	MikanObjectSystem::dispose();
}

VideoSourceIdList ClientVideoSourceSystem::getVideoSourceIdList() const
{
	VideoSourceIdList videoSourceIdList;
    for (const auto& it : m_clientVideoSourceComponents)
    {
        ClientVideoSourceComponentPtr componentPtr = it.second.lock();
        if (componentPtr)
        {
            videoSourceIdList.push_back(componentPtr->getVideoSourceId());
        }
    }
	return videoSourceIdList;
}

ClientVideoSourceComponentPtr ClientVideoSourceSystem::getClientVideoSourceById(MikanVideoSourceID videoSourceId) const
{
    auto iter = m_clientVideoSourceComponents.find(videoSourceId);
    if (iter != m_clientVideoSourceComponents.end())
    {
        return iter->second.lock();
    }

    return ClientVideoSourceComponentPtr();
}

ClientVideoSourceComponentPtr ClientVideoSourceSystem::getClientVideoSourceByName(const std::string& videoSourceName) const
{
    for (auto it = m_clientVideoSourceComponents.begin(); it != m_clientVideoSourceComponents.end(); it++)
    {
        ClientVideoSourceComponentPtr componentPtr = it->second.lock();

        if (componentPtr && componentPtr->getDefinition()->getComponentName() == videoSourceName)
        {
            return componentPtr;
        }
    }

    return ClientVideoSourceComponentPtr();
}

ClientVideoSourceComponentPtr ClientVideoSourceSystem::addNewClientVideoSource()
{
    MikanClientVideoSourceInfo videoSourceInfo = {};
	videoSourceInfo.client_source_name.setValue("New Client Video Source");

	return addNewClientVideoSource(videoSourceInfo);
}

ClientVideoSourceComponentPtr ClientVideoSourceSystem::addNewClientVideoSource(
    const MikanClientVideoSourceInfo& videoSourceInfo)
{
    VideoSourceSystemConfigPtr videoSourceSystemConfig = 
        App::getInstance()->getProfileConfig()->videoSourceSystemConfig;

    MikanVideoSourceID videoSourceId = videoSourceSystemConfig->addClientVideoSource(videoSourceInfo);
    if (videoSourceId != INVALID_MIKAN_ID)
    {
        ClientVideoSourceDefinitionPtr configPtr = videoSourceSystemConfig->getClientVideoSourceConfig(videoSourceId);
        assert(configPtr != nullptr);

        return createClientVideoSourceObject(configPtr);
    }

    return ClientVideoSourceComponentPtr();
}

bool ClientVideoSourceSystem::removeClientVideoSource(MikanVideoSourceID videoSourceId)
{
    VideoSourceSystemConfigPtr videoSourceSystemConfig = 
        App::getInstance()->getProfileConfig()->videoSourceSystemConfig;

    return
        disposeClientVideoSourceObject(videoSourceId) &&
        videoSourceSystemConfig->removeVideoSource(videoSourceId);
}

ClientVideoSourceComponentPtr ClientVideoSourceSystem::createClientVideoSourceObject(
    ClientVideoSourceDefinitionPtr videoSourceDefinition)
{
    MikanObjectPtr videoSourceObject = newObject();
    videoSourceObject->setName(videoSourceDefinition->getComponentName());

    // Make the ClientVideoSource component the root of the object
    auto videoSourceComponentPtr = videoSourceObject->addComponent<ClientVideoSourceComponent>();
    videoSourceComponentPtr->setDefinition(videoSourceDefinition);

    // Init the object once all components are added
    videoSourceObject->init();

    // Keep track of all the client video sources in the system
    m_clientVideoSourceComponents.insert({videoSourceDefinition->getVideoSourceId(), videoSourceComponentPtr});

    return videoSourceComponentPtr;
}

bool ClientVideoSourceSystem::disposeClientVideoSourceObject(MikanVideoSourceID videoSourceId)
{
    auto it = m_clientVideoSourceComponents.find(videoSourceId);
    if (it != m_clientVideoSourceComponents.end())
    {
        ClientVideoSourceComponentPtr stencilComponentPtr = it->second.lock();

        // Remove for component list
        m_clientVideoSourceComponents.erase(it);

        // Free the corresponding object
        deleteObject(stencilComponentPtr->getOwnerObject());

        return true;
    }

    return false;
}
