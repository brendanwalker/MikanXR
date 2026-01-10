#include "ClientTextureSourceSystem.h"
#include "ClientTextureSourceComponent.h"
#include "App.h"
#include "ProjectConfig.h"
#include "MikanObject.h"
#include "MikanCoreTypes.h"
#include "MikanPropertyDatabase.h"

#include <assert.h>

// -- ClientTextureSourceSystemConfig -----
const std::string ClientTextureSourceSystemConfig::k_clientTextureSourceListPropertyId = "clientTextureSourceList";

configuru::Config ClientTextureSourceSystemConfig::writeToJSON()
{
	configuru::Config pt = MikanObjectSystemDefinition::writeToJSON();

	pt["next_texture_source_id"] = m_nextTextureSourceId;

	std::vector<configuru::Config> clientTextureSourceConfigs;
	for (auto textureSource : m_clientTextureSourceList)
	{
		clientTextureSourceConfigs.push_back(textureSource->writeToJSON());
	}
	pt.insert_or_assign(k_clientTextureSourceListPropertyId, clientTextureSourceConfigs);

	return pt;
}

void ClientTextureSourceSystemConfig::readFromJSON(const configuru::Config& pt)
{
	MikanObjectSystemDefinition::readFromJSON(pt);

	m_nextTextureSourceId = pt.get_or<int>("next_texture_source_id", m_nextTextureSourceId);

	// Read in the client texture sources
	m_clientTextureSourceList.clear();
	if (pt.has_key(k_clientTextureSourceListPropertyId))
	{
		for (const configuru::Config& textureSource_pt : pt[k_clientTextureSourceListPropertyId].as_array())
		{
			auto definitionPtr = std::make_shared<ClientTextureSourceDefinition>();

			definitionPtr->readFromJSON(textureSource_pt);
			m_clientTextureSourceList.push_back(definitionPtr);

			addChildConfig(definitionPtr);
		}
	}
}

ClientTextureSourceDefinitionConstPtr ClientTextureSourceSystemConfig::getClientTextureSourceConfigConst(
	MikanTextureSourceID textureSourceId) const
{
	auto it = std::find_if(
		m_clientTextureSourceList.begin(), m_clientTextureSourceList.end(),
		[textureSourceId](ClientTextureSourceDefinitionPtr configPtr) {
			return configPtr->getTextureSourceId() == textureSourceId;
		});
	if (it != m_clientTextureSourceList.end())
	{
		return ClientTextureSourceDefinitionConstPtr(*it);
	}
	return ClientTextureSourceDefinitionConstPtr();
}

ClientTextureSourceDefinitionPtr ClientTextureSourceSystemConfig::getClientTextureSourceConfig(
	MikanTextureSourceID textureSourceId)
{
	auto constPtr = getClientTextureSourceConfigConst(textureSourceId);
	if (constPtr)
	{
		return std::const_pointer_cast<ClientTextureSourceDefinition>(constPtr);
	}
	return ClientTextureSourceDefinitionPtr();
}

ClientTextureSourceDefinitionPtr ClientTextureSourceSystemConfig::allocateClientTextureSourceDefinition(
	const MikanClientTextureSourceInfo& textureSourceInfo)
{
	ClientTextureSourceDefinitionPtr textureSourcePtr =
		std::make_shared<ClientTextureSourceDefinition>(m_nextTextureSourceId, textureSourceInfo);
	m_nextTextureSourceId++;

	return textureSourcePtr;
}

bool ClientTextureSourceSystemConfig::addClientTextureSourceDefinition(
	ClientTextureSourceDefinitionPtr textureSourceDefinitionPtr)
{
	if (!getClientTextureSourceConfig(textureSourceDefinitionPtr->getTextureSourceId()))
	{
		m_clientTextureSourceList.push_back(textureSourceDefinitionPtr);
		addChildConfig(textureSourceDefinitionPtr);

		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_clientTextureSourceListPropertyId));
		return true;
	}

	return false;
}

bool ClientTextureSourceSystemConfig::removeClientTextureSourceDefinition(
	MikanTextureSourceID textureSourceId)
{
	auto it = std::find_if(
		m_clientTextureSourceList.begin(), m_clientTextureSourceList.end(),
		[textureSourceId](ClientTextureSourceDefinitionPtr configPtr) {
			return configPtr->getTextureSourceId() == textureSourceId;
		});

	if (it != m_clientTextureSourceList.end())
	{
		removeChildConfig(*it);

		m_clientTextureSourceList.erase(it);
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_clientTextureSourceListPropertyId));

		return true;
	}

	return false;
}

// -- ClientTextureSourceSystem -----

ClientTextureSourceSystemConfigConstPtr ClientTextureSourceSystem::getClientTextureSourceSystemConfigConst() const
{
	return std::static_pointer_cast<const ClientTextureSourceSystemConfig>(getDefinitionConst());
}

ClientTextureSourceSystemConfigPtr ClientTextureSourceSystem::getClientTextureSourceSystemConfig()
{
	return std::static_pointer_cast<ClientTextureSourceSystemConfig>(getDefinition());
}

bool ClientTextureSourceSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
	MikanObjectSystem::init(definitionPtr);

    ClientTextureSourceSystemConfigConstPtr systemConfig = getClientTextureSourceSystemConfigConst();

    for (const auto& sourceConfig : systemConfig->getClientTextureSourceList())
    {
        createClientTextureSourceObject(sourceConfig);
    }

    return true;
}

void ClientTextureSourceSystem::dispose()
{
    m_clientTextureSourceComponents.clear();
	MikanObjectSystem::dispose();
}

MikanComponentPtr ClientTextureSourceSystem::getComponentById(int componentId) const
{
    return getClientTextureSourceById(componentId);
}

TextureSourceComponentList ClientTextureSourceSystem::getTextureSourceComponentList() const
{
    TextureSourceComponentList textureSourceComponentList;
    for (const auto& it : m_clientTextureSourceComponents)
    {
        TextureSourceComponentPtr componentPtr = it.second.lock();
        if (componentPtr)
        {
            textureSourceComponentList.push_back(componentPtr);
        }
    }
	return textureSourceComponentList;
}

TextureSourceIdList ClientTextureSourceSystem::getTextureSourceIdList() const
{
	TextureSourceIdList textureSourceIdList;
    for (const auto& it : m_clientTextureSourceComponents)
    {
        ClientTextureSourceComponentPtr componentPtr = it.second.lock();
        if (componentPtr)
        {
            textureSourceIdList.push_back(componentPtr->getTextureSourceId());
        }
    }
	return textureSourceIdList;
}

ClientTextureSourceComponentPtr ClientTextureSourceSystem::getClientTextureSourceById(MikanTextureSourceID TextureSourceId) const
{
    auto iter = m_clientTextureSourceComponents.find(TextureSourceId);
    if (iter != m_clientTextureSourceComponents.end())
    {
        return iter->second.lock();
    }

    return ClientTextureSourceComponentPtr();
}

ClientTextureSourceComponentPtr ClientTextureSourceSystem::getClientTextureSourceByName(const std::string& TextureSourceName) const
{
    for (auto it = m_clientTextureSourceComponents.begin(); it != m_clientTextureSourceComponents.end(); it++)
    {
        ClientTextureSourceComponentPtr componentPtr = it->second.lock();

        if (componentPtr && componentPtr->getDefinition()->getComponentName() == TextureSourceName)
        {
            return componentPtr;
        }
    }

    return ClientTextureSourceComponentPtr();
}

ClientTextureSourceComponentPtr ClientTextureSourceSystem::addNewClientTextureSource()
{
    MikanClientTextureSourceInfo TextureSourceInfo = {};
	TextureSourceInfo.client_source_name.setValue("New Client Video Source");

	return addNewClientTextureSource(TextureSourceInfo);
}

ClientTextureSourceComponentPtr ClientTextureSourceSystem::addNewClientTextureSource(
    const MikanClientTextureSourceInfo& TextureSourceInfo)
{
    ClientTextureSourceSystemConfigPtr systemConfig = getClientTextureSourceSystemConfig();

    return
        createClientTextureSourceObject(
            systemConfig->allocateClientTextureSourceDefinition(TextureSourceInfo));
}

bool ClientTextureSourceSystem::removeClientTextureSource(MikanTextureSourceID TextureSourceId)
{
    ClientTextureSourceSystemConfigPtr systemConfig = getClientTextureSourceSystemConfig();

    return
        disposeClientTextureSourceObject(TextureSourceId) &&
        systemConfig->removeClientTextureSourceDefinition(TextureSourceId);
}

ClientTextureSourceComponentPtr ClientTextureSourceSystem::createClientTextureSourceObject(
    ClientTextureSourceDefinitionPtr TextureSourceDefinition)
{
    MikanObjectPtr TextureSourceObject = newObject();
    TextureSourceObject->setName(TextureSourceDefinition->getComponentName());

    // Make the ClientTextureSource component the root of the object
    auto TextureSourceComponentPtr = TextureSourceObject->addComponent<ClientTextureSourceComponent>();
    TextureSourceComponentPtr->setDefinition(TextureSourceDefinition);

    // Init the object once all components are added
    TextureSourceObject->init();

    // Keep track of all the client texture sources in the system
    m_clientTextureSourceComponents.insert({TextureSourceDefinition->getTextureSourceId(), TextureSourceComponentPtr});

    // Register the definition with the system config
    getClientTextureSourceSystemConfig()->addClientTextureSourceDefinition(TextureSourceDefinition);

    return TextureSourceComponentPtr;
}

bool ClientTextureSourceSystem::disposeClientTextureSourceObject(MikanTextureSourceID TextureSourceId)
{
    auto it = m_clientTextureSourceComponents.find(TextureSourceId);
    if (it != m_clientTextureSourceComponents.end())
    {
        ClientTextureSourceComponentPtr stencilComponentPtr = it->second.lock();

        // Remove for component list
        m_clientTextureSourceComponents.erase(it);

        // Free the corresponding object
        deleteObject(stencilComponentPtr->getOwnerObject());

        return true;
    }

    return false;
}

void ClientTextureSourceSystem::registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase)
{
	propertyDatabase->registerPropertiesForSystem<ClientTextureSourceSystem>();
	propertyDatabase->registerPropertiesForComponent<ClientTextureSourceSystem, ClientTextureSourceComponent>();
}