#include "App.h"
#include "MikanCoreTypes.h"
#include "MikanObject.h"
#include "ProjectConfig.h"
#include "SpoutTextureSourceComponent.h"
#include "SpoutTextureSourceSystem.h"
#include "TextureSourceSystemConfig.h"
#include "TextureSourceSystem.h"

#include <assert.h>

bool SpoutTextureSourceSystem::init()
{
	MikanObjectSystem::init();
    TextureSourceSystemConfigConstPtr TextureSourceSystemConfig = 
        getProjectConfig()->textureSourceSystemConfig;

    for (const auto& sourceConfig : TextureSourceSystemConfig->getSpoutTextureSourceList())
    {
        createSpoutTextureSourceObject(sourceConfig);
    }

    return true;
}

void SpoutTextureSourceSystem::dispose()
{
    m_spoutTextureSourceComponents.clear();
	MikanObjectSystem::dispose();
}

TextureSourceComponentList SpoutTextureSourceSystem::getTextureSourceComponentList() const
{
	TextureSourceComponentList textureSourceComponentList;
	for (const auto& it : m_spoutTextureSourceComponents)
	{
		TextureSourceComponentPtr componentPtr = it.second.lock();
		if (componentPtr)
		{
			textureSourceComponentList.push_back(componentPtr);
		}
	}
	return textureSourceComponentList;
}

TextureSourceIdList SpoutTextureSourceSystem::getTextureSourceIdList() const
{
	TextureSourceIdList textureSourceIdList;
	for (const auto& it : m_spoutTextureSourceComponents)
	{
        SpoutTextureSourceComponentPtr componentPtr = it.second.lock();
		if (componentPtr)
		{
            textureSourceIdList.push_back(componentPtr->getTextureSourceId());
		}
	}
	return textureSourceIdList;
}

SpoutTextureSourceComponentPtr SpoutTextureSourceSystem::getSpoutTextureSourceById(MikanTextureSourceID TextureSourceId) const
{
    auto iter = m_spoutTextureSourceComponents.find(TextureSourceId);
    if (iter != m_spoutTextureSourceComponents.end())
    {
        return iter->second.lock();
    }

    return SpoutTextureSourceComponentPtr();
}

SpoutTextureSourceComponentPtr SpoutTextureSourceSystem::getSpoutTextureSourceByName(const std::string& TextureSourceName) const
{
    for (auto it = m_spoutTextureSourceComponents.begin(); it != m_spoutTextureSourceComponents.end(); it++)
    {
        SpoutTextureSourceComponentPtr componentPtr = it->second.lock();

        if (componentPtr && componentPtr->getDefinition()->getComponentName() == TextureSourceName)
        {
            return componentPtr;
        }
    }

    return SpoutTextureSourceComponentPtr();
}

SpoutTextureSourceComponentPtr SpoutTextureSourceSystem::addNewSpoutTextureSource()
{
    MikanSpoutTextureSourceInfo TextureSourceInfo = {};
	TextureSourceInfo.spout_source_name.setValue("Spout Source Name");

	return addNewSpoutTextureSource(TextureSourceInfo);
}

SpoutTextureSourceComponentPtr SpoutTextureSourceSystem::addNewSpoutTextureSource(
    const MikanSpoutTextureSourceInfo& TextureSourceInfo)
{
    TextureSourceSystemConfigPtr TextureSourceSystemConfig =
        getProjectConfig()->textureSourceSystemConfig;

    return
        createSpoutTextureSourceObject(
            TextureSourceSystemConfig->allocateSpoutTextureSourceDefinition(TextureSourceInfo));
}

bool SpoutTextureSourceSystem::removeSpoutTextureSource(MikanTextureSourceID TextureSourceId)
{
    TextureSourceSystemConfigPtr TextureSourceSystemConfig = 
        getProjectConfig()->textureSourceSystemConfig;

    return
        disposeSpoutTextureSourceObject(TextureSourceId) &&
        TextureSourceSystemConfig->removeTextureSource(TextureSourceId);
}

SpoutTextureSourceComponentPtr SpoutTextureSourceSystem::createSpoutTextureSourceObject(
    SpoutTextureSourceDefinitionPtr TextureSourceDefinition)
{
    MikanObjectPtr TextureSourceObject = newObject();
    TextureSourceObject->setName(TextureSourceDefinition->getComponentName());

    // Make the SpoutTextureSource component the root of the object
    auto TextureSourceComponentPtr = TextureSourceObject->addComponent<SpoutTextureSourceComponent>();
    TextureSourceComponentPtr->setDefinition(TextureSourceDefinition);

    // Init the object once all components are added
    TextureSourceObject->init();

    // Keep track of all the spout video sources in the system
    m_spoutTextureSourceComponents.insert({ TextureSourceDefinition->getTextureSourceId(), TextureSourceComponentPtr });

    // Register the definition with the texture source system
    getProjectConfig()->textureSourceSystemConfig->addSpoutTextureSourceDefinition(TextureSourceDefinition);

    return TextureSourceComponentPtr;
}

bool SpoutTextureSourceSystem::disposeSpoutTextureSourceObject(MikanTextureSourceID TextureSourceId)
{
    auto it = m_spoutTextureSourceComponents.find(TextureSourceId);
    if (it != m_spoutTextureSourceComponents.end())
    {
        SpoutTextureSourceComponentPtr stencilComponentPtr = it->second.lock();

        // Remove for component list
        m_spoutTextureSourceComponents.erase(it);

        // Free the corresponding object
        deleteObject(stencilComponentPtr->getOwnerObject());

        return true;
    }

    return false;
}
