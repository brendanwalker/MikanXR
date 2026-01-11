#include "SpoutTextureSourceSystem.h"
#include "SpoutTextureSourceComponent.h"
#include "App.h"
#include "ProjectConfig.h"
#include "MikanObject.h"
#include "MikanCoreTypes.h"
#include "MikanPropertyDatabase.h"

#include "SpoutLibrary.h"

#include <assert.h>

// -- SpoutTextureSourceSystemConfig -----
const std::string SpoutTextureSourceSystemDefinition::k_spoutTextureSourceListPropertyId = "spoutTextureSourceList";

configuru::Config SpoutTextureSourceSystemDefinition::writeToJSON()
{
	configuru::Config pt = MikanObjectSystemDefinition::writeToJSON();

	pt["next_texture_source_id"] = m_nextTextureSourceId;

	std::vector<configuru::Config> spoutTextureSourceConfigs;
	for (auto textureSource : m_spoutTextureSourceList)
	{
		spoutTextureSourceConfigs.push_back(textureSource->writeToJSON());
	}
	pt.insert_or_assign(k_spoutTextureSourceListPropertyId, spoutTextureSourceConfigs);

	return pt;
}

void SpoutTextureSourceSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanObjectSystemDefinition::readFromJSON(pt);

	m_nextTextureSourceId = pt.get_or<int>("next_texture_source_id", m_nextTextureSourceId);

	// Read in the spout texture sources
	m_spoutTextureSourceList.clear();
	if (pt.has_key(k_spoutTextureSourceListPropertyId))
	{
		for (const configuru::Config& textureSource_pt : pt[k_spoutTextureSourceListPropertyId].as_array())
		{
			auto definitionPtr = std::make_shared<SpoutTextureSourceDefinition>();

			definitionPtr->readFromJSON(textureSource_pt);
			m_spoutTextureSourceList.push_back(definitionPtr);

			addChildConfig(definitionPtr);
		}
	}
}

SpoutTextureSourceDefinitionConstPtr SpoutTextureSourceSystemDefinition::getSpoutTextureSourceConfigConst(
	MikanTextureSourceID textureSourceId) const
{
	auto it = std::find_if(
		m_spoutTextureSourceList.begin(), m_spoutTextureSourceList.end(),
		[textureSourceId](SpoutTextureSourceDefinitionPtr configPtr) {
			return configPtr->getTextureSourceId() == textureSourceId;
		});
	if (it != m_spoutTextureSourceList.end())
	{
		return SpoutTextureSourceDefinitionConstPtr(*it);
	}
	return SpoutTextureSourceDefinitionConstPtr();
}

SpoutTextureSourceDefinitionPtr SpoutTextureSourceSystemDefinition::getSpoutTextureSourceConfig(
	MikanTextureSourceID textureSourceId)
{
	auto constPtr = getSpoutTextureSourceConfigConst(textureSourceId);
	if (constPtr)
	{
		return std::const_pointer_cast<SpoutTextureSourceDefinition>(constPtr);
	}
	return SpoutTextureSourceDefinitionPtr();
}

SpoutTextureSourceDefinitionPtr SpoutTextureSourceSystemDefinition::allocateSpoutTextureSourceDefinition(
	const MikanSpoutTextureSourceInfo& textureSourceInfo)
{
	SpoutTextureSourceDefinitionPtr textureSourcePtr =
		std::make_shared<SpoutTextureSourceDefinition>(m_nextTextureSourceId, textureSourceInfo);
	m_nextTextureSourceId++;

	return textureSourcePtr;
}

bool SpoutTextureSourceSystemDefinition::addSpoutTextureSourceDefinition(
	SpoutTextureSourceDefinitionPtr textureSourceDefinitionPtr)
{
	if (!getSpoutTextureSourceConfig(textureSourceDefinitionPtr->getTextureSourceId()))
	{
		m_spoutTextureSourceList.push_back(textureSourceDefinitionPtr);
		addChildConfig(textureSourceDefinitionPtr);

		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_spoutTextureSourceListPropertyId));
		return true;
	}

	return false;
}

bool SpoutTextureSourceSystemDefinition::removeSpoutTextureSourceDefinition(
	MikanTextureSourceID textureSourceId)
{
	auto it = std::find_if(
		m_spoutTextureSourceList.begin(), m_spoutTextureSourceList.end(),
		[textureSourceId](SpoutTextureSourceDefinitionPtr configPtr) {
			return configPtr->getTextureSourceId() == textureSourceId;
		});

	if (it != m_spoutTextureSourceList.end())
	{
		removeChildConfig(*it);

		m_spoutTextureSourceList.erase(it);
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_spoutTextureSourceListPropertyId));

		return true;
	}

	return false;
}

// -- SpoutTextureSourceSystem -----

SpoutTextureSourceSystemDefinitionConstPtr SpoutTextureSourceSystem::getSpoutTextureSourceSystemConfigConst() const
{
	return std::static_pointer_cast<const SpoutTextureSourceSystemDefinition>(getDefinitionConst());
}

SpoutTextureSourceSystemDefinitionPtr SpoutTextureSourceSystem::getSpoutTextureSourceSystemConfig()
{
	return std::static_pointer_cast<SpoutTextureSourceSystemDefinition>(getDefinition());
}

SpoutTextureSourceSystem::SpoutTextureSourceSystem(ProjectManagerPtr ownerObjectSystem) 
    : MikanObjectSystem(ownerObjectSystem)
{
}

bool SpoutTextureSourceSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
	MikanObjectSystem::init(definitionPtr);

    SpoutTextureSourceSystemDefinitionConstPtr systemConfig = getSpoutTextureSourceSystemConfigConst();

    for (const auto& sourceConfig : systemConfig->getSpoutTextureSourceList())
    {
        createSpoutTextureSourceObject(sourceConfig);
    }

    m_spoutLibrary = GetSpout();

    return true;
}

void SpoutTextureSourceSystem::dispose()
{
    if (m_spoutLibrary)
    {
        m_spoutLibrary->Release();
		m_spoutLibrary = nullptr;
    }

    m_spoutTextureSourceComponents.clear();
	MikanObjectSystem::dispose();
}

MikanComponentPtr SpoutTextureSourceSystem::getComponentById(int componentId) const
{
	return getSpoutTextureSourceById(componentId);
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
    SpoutTextureSourceSystemDefinitionPtr systemConfig = getSpoutTextureSourceSystemConfig();

    return
        createSpoutTextureSourceObject(
            systemConfig->allocateSpoutTextureSourceDefinition(TextureSourceInfo));
}

bool SpoutTextureSourceSystem::removeSpoutTextureSource(MikanTextureSourceID TextureSourceId)
{
    SpoutTextureSourceSystemDefinitionPtr systemConfig = getSpoutTextureSourceSystemConfig();

    return
        disposeSpoutTextureSourceObject(TextureSourceId) &&
        systemConfig->removeSpoutTextureSourceDefinition(TextureSourceId);
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

    // Register the definition with the system config
    getSpoutTextureSourceSystemConfig()->addSpoutTextureSourceDefinition(TextureSourceDefinition);

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

void SpoutTextureSourceSystem::getAvailableSpoutSenderNames(std::vector<std::string>& outSenderNames) const
{
    if (m_spoutLibrary != nullptr)
    {
        for (int i = 0; i < m_spoutLibrary->GetSenderCount(); i++)
        {
            char sendername[256];
            if (m_spoutLibrary->GetSender(i, sendername, 256))
            {
                outSenderNames.push_back(std::string(sendername));
            }
		}
    }
}

void SpoutTextureSourceSystem::registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase)
{
	propertyDatabase->registerPropertiesForSystem<SpoutTextureSourceSystem>();
	propertyDatabase->registerPropertiesForComponent<SpoutTextureSourceSystem, SpoutTextureSourceComponent>();
}