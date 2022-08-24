#include "GlModelResourceManager.h"
#include "GlRenderModelResource.h"
#include "GlVertexDefinition.h"

GlModelResourceManager::GlModelResourceManager()
{
}

GlModelResourceManager::~GlModelResourceManager()
{
	cleanup();
}

void GlModelResourceManager::init()
{
}

void GlModelResourceManager::cleanup()
{
	for (auto it = m_renderModelCache.begin(); it != m_renderModelCache.end(); ++it)
	{
		GlRenderModelResource* resource = it->second;

		resource->disposeRenderResources();
		delete resource;
	}

	m_renderModelCache.clear();
}

GlRenderModelResource* GlModelResourceManager::fetchRenderModel(
	const std::string& renderModelName,
	const GlVertexDefinition* vertexDefinition)
{
	if (renderModelName.size() > 0)
	{
		std::string hashName = renderModelName + vertexDefinition->getVertexDefinitionDesc();

		if (m_renderModelCache.find(hashName) != m_renderModelCache.end())
		{
			return m_renderModelCache[hashName];
		}
		else
		{
			GlRenderModelResource* resource = new GlRenderModelResource(renderModelName, vertexDefinition);

			if (resource->createRenderResources())
			{
				m_renderModelCache[hashName] = resource;

				return resource;
			}
			else
			{
				delete resource;

				return nullptr;
			}
		}
	}

	return nullptr;
}