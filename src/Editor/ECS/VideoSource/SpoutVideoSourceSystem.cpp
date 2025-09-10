#include "App.h"
#include "MikanObject.h"
#include "ProjectConfig.h"
#include "SpoutVideoSourceComponent.h"
#include "SpoutVideoSourceSystem.h"
#include "VideoSourceSystemConfig.h"
#include "VideoSourceSystem.h"

#include <assert.h>

bool SpoutVideoSourceSystem::init()
{
	MikanObjectSystem::init();
    VideoSourceSystemConfigConstPtr videoSourceSystemConfig = 
        App::getInstance()->getProfileConfig()->videoSourceSystemConfig;

    for (const auto& sourceConfig : videoSourceSystemConfig->getSpoutVideoSourceList())
    {
        createSpoutVideoSourceObject(sourceConfig);
    }

    return true;
}

void SpoutVideoSourceSystem::dispose()
{
    m_spoutVideoSourceComponents.clear();
	MikanObjectSystem::dispose();
}

VideoSourceIdList SpoutVideoSourceSystem::getVideoSourceIdList() const
{
	VideoSourceIdList videoSourceIdList;
	for (const auto& it : m_spoutVideoSourceComponents)
	{
        SpoutVideoSourceComponentPtr componentPtr = it.second.lock();
		if (componentPtr)
		{
			videoSourceIdList.push_back(componentPtr->getVideoSourceId());
		}
	}
	return videoSourceIdList;
}

SpoutVideoSourceComponentPtr SpoutVideoSourceSystem::getSpoutVideoSourceById(MikanVideoSourceID videoSourceId) const
{
    auto iter = m_spoutVideoSourceComponents.find(videoSourceId);
    if (iter != m_spoutVideoSourceComponents.end())
    {
        return iter->second.lock();
    }

    return SpoutVideoSourceComponentPtr();
}

SpoutVideoSourceComponentPtr SpoutVideoSourceSystem::getSpoutVideoSourceByName(const std::string& videoSourceName) const
{
    for (auto it = m_spoutVideoSourceComponents.begin(); it != m_spoutVideoSourceComponents.end(); it++)
    {
        SpoutVideoSourceComponentPtr componentPtr = it->second.lock();

        if (componentPtr && componentPtr->getDefinition()->getComponentName() == videoSourceName)
        {
            return componentPtr;
        }
    }

    return SpoutVideoSourceComponentPtr();
}

SpoutVideoSourceComponentPtr SpoutVideoSourceSystem::addNewSpoutVideoSource()
{
    MikanSpoutVideoSourceInfo videoSourceInfo = {};
	videoSourceInfo.spout_source_name.setValue("Spout Source Name");

	return addNewSpoutVideoSource(videoSourceInfo);
}

SpoutVideoSourceComponentPtr SpoutVideoSourceSystem::addNewSpoutVideoSource(
    const MikanSpoutVideoSourceInfo& videoSourceInfo)
{
    VideoSourceSystemConfigPtr videoSourceSystemConfig = 
        App::getInstance()->getProfileConfig()->videoSourceSystemConfig;

    MikanVideoSourceID videoSourceId = videoSourceSystemConfig->addSpoutVideoSource(videoSourceInfo);
    if (videoSourceId != INVALID_MIKAN_ID)
    {
        SpoutVideoSourceDefinitionPtr configPtr = videoSourceSystemConfig->getSpoutVideoSourceConfig(videoSourceId);
        assert(configPtr != nullptr);

        return createSpoutVideoSourceObject(configPtr);
    }

    return SpoutVideoSourceComponentPtr();
}

bool SpoutVideoSourceSystem::removeSpoutVideoSource(MikanVideoSourceID videoSourceId)
{
    VideoSourceSystemConfigPtr videoSourceSystemConfig = 
        App::getInstance()->getProfileConfig()->videoSourceSystemConfig;

    return
        disposeSpoutVideoSourceObject(videoSourceId) &&
        videoSourceSystemConfig->removeVideoSource(videoSourceId);
}

SpoutVideoSourceComponentPtr SpoutVideoSourceSystem::createSpoutVideoSourceObject(
    SpoutVideoSourceDefinitionPtr videoSourceDefinition)
{
    MikanObjectPtr videoSourceObject = newObject();
    videoSourceObject->setName(videoSourceDefinition->getComponentName());

    // Make the SpoutVideoSource component the root of the object
    auto videoSourceComponentPtr = videoSourceObject->addComponent<SpoutVideoSourceComponent>();
    videoSourceComponentPtr->setDefinition(videoSourceDefinition);

    // Init the object once all components are added
    videoSourceObject->init();

    // Keep track of all the spout video sources in the system
    m_spoutVideoSourceComponents.insert({ videoSourceDefinition->getVideoSourceId(), videoSourceComponentPtr });

    return videoSourceComponentPtr;
}

bool SpoutVideoSourceSystem::disposeSpoutVideoSourceObject(MikanVideoSourceID videoSourceId)
{
    auto it = m_spoutVideoSourceComponents.find(videoSourceId);
    if (it != m_spoutVideoSourceComponents.end())
    {
        SpoutVideoSourceComponentPtr stencilComponentPtr = it->second.lock();

        // Remove for component list
        m_spoutVideoSourceComponents.erase(it);

        // Free the corresponding object
        deleteObject(stencilComponentPtr->getOwnerObject());

        return true;
    }

    return false;
}
