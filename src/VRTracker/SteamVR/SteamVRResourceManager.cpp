#include "SteamVRResourceManager.h"
#include "SteamVRRenderModelResource.h"

SteamVRResourceManager::SteamVRResourceManager()
{
}

SteamVRResourceManager::~SteamVRResourceManager()
{
	cleanup();
}

void SteamVRResourceManager::init()
{
}

void SteamVRResourceManager::cleanup()
{
	for (auto it = m_renderModelCache.begin(); it != m_renderModelCache.end(); ++it)
	{
		SteamVRRenderModelResource *resource= it->second;
		
		resource->disposeRenderResources();
		delete resource;
	}

	m_renderModelCache.clear();
}

SteamVRRenderModelResource* SteamVRResourceManager::fetchRenderModel(
	const std::string& renderModelName)
{
	if (m_renderModelCache.find(renderModelName) != m_renderModelCache.end())
	{
		return m_renderModelCache[renderModelName];
	}
	else
	{
		SteamVRRenderModelResource* resource = new SteamVRRenderModelResource(renderModelName);

		if (resource->createRenderResources())
		{
			m_renderModelCache[renderModelName]= resource;

			return resource;
		}
		else
		{
			delete resource;

			return nullptr;
		}
	}
}