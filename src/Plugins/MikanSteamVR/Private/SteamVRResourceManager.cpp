#include "SteamVRResourceManager.h"
#include "SteamVRRenderModelResource.h"

SteamVRResourceManager::SteamVRResourceManager() {}

SteamVRResourceManager::~SteamVRResourceManager() { cleanup(); }

void SteamVRResourceManager::init(IMkGraphicsContext* ownerWindow) { m_ownerContext= ownerWindow; }

void SteamVRResourceManager::cleanup()
{
	for (auto it= m_renderModelCache.begin(); it != m_renderModelCache.end(); ++it)
	{
		SteamVRRenderModelResource* resource= it->second;

		resource->disposeRenderResources();
		delete resource;
	}

	m_renderModelCache.clear();
}

SteamVRRenderModelResource* SteamVRResourceManager::fetchRenderModel(const std::string& renderModelName)
{
	auto it= m_renderModelCache.find(renderModelName);
	if (it != m_renderModelCache.end())
	{
		return it->second;
	}

	SteamVRRenderModelResource* resource= new SteamVRRenderModelResource(m_ownerContext);
	resource->setRenderModelName(renderModelName);

	// Phase 1 only: load raw SteamVR data (safe on background thread)
	if (resource->loadSteamVRResources())
	{
		m_renderModelCache[renderModelName]= resource;
		return resource;
	}

	delete resource;
	return nullptr;
}

void SteamVRResourceManager::createPendingGraphicsResources()
{
	for (auto& kvpair : m_renderModelCache)
	{
		SteamVRRenderModelResource* resource= kvpair.second;
		if (!resource->hasGraphicsResources())
		{
			resource->createGraphicsResources();
		}
	}
}