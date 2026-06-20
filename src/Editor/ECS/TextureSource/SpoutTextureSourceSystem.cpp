#include "SpoutTextureSourceSystem.h"
#include "SpoutTextureSourceComponent.h"
#include "App.h"
#include "ProjectConfig.h"
#include "MikanObject.h"
#include "MikanCoreTypes.h"
#include "MikanPropertyDatabase.h"

#include "SpoutLibrary.h"

#include <assert.h>

// -- SpoutTextureSourceSystemDefinition -----
SpoutTextureSourceSystemDefinition::SpoutTextureSourceSystemDefinition(
	const std::string& configName, IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- SpoutTextureSourceSystem -----
SpoutTextureSourceSystem::SpoutTextureSourceSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}

bool SpoutTextureSourceSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
	Super::init(definitionPtr);

	m_spoutLibrary= GetSpout();

	return true;
}

void SpoutTextureSourceSystem::dispose()
{
	if (m_spoutLibrary)
	{
		m_spoutLibrary->Release();
		m_spoutLibrary= nullptr;
	}

	Super::dispose();
}

TextureSourceComponentList SpoutTextureSourceSystem::getTextureSourceComponentList() const
{
	TextureSourceComponentList textureSourceComponentList;
	for (const auto& it : Super::getComponentMap())
	{
		TextureSourceComponentPtr componentPtr= it.second.lock();
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
	for (const auto& it : Super::getComponentMap())
	{
		SpoutTextureSourceComponentPtr componentPtr= it.second.lock();
		if (componentPtr)
		{
			textureSourceIdList.push_back(componentPtr->getTextureSourceId());
		}
	}
	return textureSourceIdList;
}

void SpoutTextureSourceSystem::getAvailableSpoutSenderNames(std::vector<std::string>& outSenderNames) const
{
	if (m_spoutLibrary != nullptr)
	{
		for (int i= 0; i < m_spoutLibrary->GetSenderCount(); i++)
		{
			char sendername[256];
			if (m_spoutLibrary->GetSender(i, sendername, 256))
			{
				outSenderNames.push_back(std::string(sendername));
			}
		}
	}
}