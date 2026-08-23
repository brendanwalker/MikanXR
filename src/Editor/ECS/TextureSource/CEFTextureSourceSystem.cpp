#include "CEFTextureSourceSystem.h"
#include "CEFTextureSourceComponent.h"

// -- CEFTextureSourceSystemDefinition -----
CEFTextureSourceSystemDefinition::CEFTextureSourceSystemDefinition(const std::string& configName,
																   IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- CEFTextureSourceSystem -----
CEFTextureSourceSystem::CEFTextureSourceSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}

TextureSourceComponentList CEFTextureSourceSystem::getTextureSourceComponentList() const
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

TextureSourceIdList CEFTextureSourceSystem::getTextureSourceIdList() const
{
	TextureSourceIdList textureSourceIdList;
	for (const auto& it : Super::getComponentMap())
	{
		CEFTextureSourceComponentPtr componentPtr= it.second.lock();
		if (componentPtr)
		{
			textureSourceIdList.push_back(componentPtr->getTextureSourceId());
		}
	}
	return textureSourceIdList;
}
