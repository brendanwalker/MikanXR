#include "GlMaterial.h"
#include "GlModelResourceManager.h"
#include "GlRenderModelResource.h"
#include "GlProgram.h"
#include "GlShaderCache.h"
#include "GlVertexDefinition.h"
#include "IGlWindow.h"
#include "Logger.h"

GlModelResourceManager::GlModelResourceManager(IGlWindow* ownerWindow)
	: m_ownerWindow(ownerWindow)
{
}

GlModelResourceManager::~GlModelResourceManager()
{
	shutdown();
}

bool GlModelResourceManager::startup()
{
	return true;
}

void GlModelResourceManager::shutdown()
{
	m_renderModelCache.clear();
}

GlRenderModelResourcePtr GlModelResourceManager::fetchRenderModel(
	const std::filesystem::path& modelFilePath,
	GlMaterialConstPtr overrideMaterial)
{
	if (!modelFilePath.empty())
	{
		std::string modelPathString = modelFilePath.string();

		auto it = m_renderModelCache.find(modelPathString);
		if (it != m_renderModelCache.end())
		{
			return it->second;
		}
		else
		{
			GlRenderModelResourcePtr resource = std::make_shared<GlRenderModelResource>(m_ownerWindow);
			resource->setModelFilePath(modelFilePath);

			if (resource->loadFromRenderModelFilePath(overrideMaterial))
			{
				m_renderModelCache.insert({modelPathString, resource});

				return resource;
			}
		}
	}

	return nullptr;
}

bool GlModelResourceManager::removeModelResourceFromCache(GlRenderModelResourcePtr resource)
{
	if (resource)
	{
		std::string modelPathString = resource->getModelFilePath().string();
		auto it = m_renderModelCache.find(modelPathString);
		if (it != m_renderModelCache.end())
		{
			m_renderModelCache.erase(it);
			return true;
		}
	}

	return false;
}

