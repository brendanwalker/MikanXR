#include "ClientTextureSourceSystem.h"
#include "ClientTextureSourceComponent.h"
#include "App.h"
#include "ProjectConfig.h"
#include "MikanObject.h"
#include "MikanCoreTypes.h"
#include "MikanPropertyDatabase.h"

#include <assert.h>

// -- ClientTextureSourceSystemDefinition -----
ClientTextureSourceSystemDefinition::ClientTextureSourceSystemDefinition(
	const std::string& configName, IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- ClientTextureSourceSystem -----
ClientTextureSourceSystem::ClientTextureSourceSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}

TextureSourceComponentList ClientTextureSourceSystem::getTextureSourceComponentList() const
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

TextureSourceIdList ClientTextureSourceSystem::getTextureSourceIdList() const
{
	TextureSourceIdList textureSourceIdList;
	for (const auto& it : Super::getComponentMap())
	{
		ClientTextureSourceComponentPtr componentPtr= it.second.lock();
		if (componentPtr)
		{
			textureSourceIdList.push_back(componentPtr->getTextureSourceId());
		}
	}
	return textureSourceIdList;
}