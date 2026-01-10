#include "TextureSourceQueries.h"
#include "ClientTextureSourceSystem.h"
#include "ClientTextureSourceComponent.h"
#include "ProjectManager.h"
#include "SpoutTextureSourceSystem.h"
#include "SpoutTextureSourceComponent.h"

namespace TextureSourceQueries
{
	TextureSourceComponentList getTextureSourceComponentList(ProjectManagerPtr projectManager)
	{
		TextureSourceComponentList componentList;

		auto clientTextureSourceSystem = projectManager->getSystemOfType<ClientTextureSourceSystem>();
		auto clientTextureSourceComponents = clientTextureSourceSystem->getTextureSourceComponentList();
		componentList.insert(componentList.end(), clientTextureSourceComponents.begin(), clientTextureSourceComponents.end());

		auto spoutTextureSourceSystem = projectManager->getSystemOfType<SpoutTextureSourceSystem>();
		auto spoutTextureSourceComponents = spoutTextureSourceSystem->getTextureSourceComponentList();
		componentList.insert(componentList.end(), spoutTextureSourceComponents.begin(), spoutTextureSourceComponents.end());

		return componentList;
	}

	TextureSourceIdList getTextureSourceIdList(ProjectManagerPtr projectManager)
	{
		TextureSourceIdList textureSourceIdList;

		auto clientTextureSourceSystem = projectManager->getSystemOfType<ClientTextureSourceSystem>();
		auto clientTextureSourceIds = clientTextureSourceSystem->getTextureSourceIdList();
		textureSourceIdList.insert(textureSourceIdList.end(), clientTextureSourceIds.begin(), clientTextureSourceIds.end());

		auto spoutTextureSourceSystem = projectManager->getSystemOfType<SpoutTextureSourceSystem>();
		auto spoutTextureSourceIds = spoutTextureSourceSystem->getTextureSourceIdList();
		textureSourceIdList.insert(textureSourceIdList.end(), spoutTextureSourceIds.begin(), spoutTextureSourceIds.end());

		return textureSourceIdList;
	}

	TextureSourceComponentPtr getTextureSourceById(ProjectManagerPtr projectManager, MikanTextureSourceID textureSourceId)
	{
		auto clientTextureSourceSystem = projectManager->getSystemOfType<ClientTextureSourceSystem>();
		auto clientTextureSourcePtr = clientTextureSourceSystem->getClientTextureSourceById(textureSourceId);
		if (clientTextureSourcePtr)
		{
			return clientTextureSourcePtr;
		}

		auto spoutTextureSourceSystem = projectManager->getSystemOfType<SpoutTextureSourceSystem>();
		auto spoutTextureSourcePtr = spoutTextureSourceSystem->getSpoutTextureSourceById(textureSourceId);
		if (spoutTextureSourcePtr)
		{
			return spoutTextureSourcePtr;
		}

		return TextureSourceComponentPtr();
	}

	eTextureSourceType getTextureSourceType(ProjectManagerPtr projectManager, MikanTextureSourceID textureSourceId)
	{
		auto clientTextureSourceSystem = projectManager->getSystemOfType<ClientTextureSourceSystem>();
		auto clientTextureSourcePtr = clientTextureSourceSystem->getClientTextureSourceById(textureSourceId);
		if (clientTextureSourcePtr)
		{
			return eTextureSourceType::client;
		}

		auto spoutTextureSourceSystem = projectManager->getSystemOfType<SpoutTextureSourceSystem>();
		auto spoutTextureSourcePtr = spoutTextureSourceSystem->getSpoutTextureSourceById(textureSourceId);
		if (spoutTextureSourcePtr)
		{
			return eTextureSourceType::spout;
		}

		return eTextureSourceType::INVALID;
	}

	bool removeTextureSource(ProjectManagerPtr projectManager, MikanTextureSourceID textureSourceId)
	{
		switch (getTextureSourceType(projectManager, textureSourceId))
		{
			case eTextureSourceType::client:
			{
				auto clientTextureSourceSystem = projectManager->getSystemOfType<ClientTextureSourceSystem>();
				return clientTextureSourceSystem->removeClientTextureSource(textureSourceId);
			}
			case eTextureSourceType::spout:
			{
				auto spoutTextureSourceSystem = projectManager->getSystemOfType<SpoutTextureSourceSystem>();
				return spoutTextureSourceSystem->removeSpoutTextureSource(textureSourceId);
			}
		}

		return false;
	}
}
