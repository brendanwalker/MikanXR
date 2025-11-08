#include "ClientTextureSourceSystem.h"
#include "TextureSourceSystem.h"
#include "ClientTextureSourceComponent.h"
#include "App.h"
#include "ProjectConfig.h"
#include "TextureSourceSystemConfig.h"
#include "MikanObject.h"
#include "MikanCoreTypes.h"

#include <assert.h>

bool ClientTextureSourceSystem::init()
{
	MikanObjectSystem::init();

    TextureSourceSystemConfigConstPtr TextureSourceSystemConfig = 
        getProjectConfig()->textureSourceSystemConfig;

    for (const auto& sourceConfig : TextureSourceSystemConfig->getClientTextureSourceList())
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
    TextureSourceSystemConfigPtr TextureSourceSystemConfig = 
        getProjectConfig()->textureSourceSystemConfig;

    MikanTextureSourceID TextureSourceId = TextureSourceSystemConfig->addClientTextureSource(TextureSourceInfo);
    if (TextureSourceId != INVALID_MIKAN_ID)
    {
        ClientTextureSourceDefinitionPtr configPtr = TextureSourceSystemConfig->getClientTextureSourceConfig(TextureSourceId);
        assert(configPtr != nullptr);

        return createClientTextureSourceObject(configPtr);
    }

    return ClientTextureSourceComponentPtr();
}

bool ClientTextureSourceSystem::removeClientTextureSource(MikanTextureSourceID TextureSourceId)
{
    TextureSourceSystemConfigPtr TextureSourceSystemConfig = 
        getProjectConfig()->textureSourceSystemConfig;

    return
        disposeClientTextureSourceObject(TextureSourceId) &&
        TextureSourceSystemConfig->removeTextureSource(TextureSourceId);
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

    // Keep track of all the client video sources in the system
    m_clientTextureSourceComponents.insert({TextureSourceDefinition->getTextureSourceId(), TextureSourceComponentPtr});

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
