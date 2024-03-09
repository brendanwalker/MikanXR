#include "GlMaterial.h"
#include "GlModelResourceManager.h"
#include "GlRenderModelResource.h"
#include "GlProgram.h"
#include "GlShaderCache.h"
#include "GlVertexDefinition.h"
#include "IGlWindow.h"
#include "Logger.h"
#include "ObjModelImporter.h"

GlModelResourceManager::GlModelResourceManager(IGlWindow* ownerWindow)
	: m_ownerWindow(ownerWindow)
{
	// Register model importers
	m_modelImporters.insert({".obj", std::make_shared<ObjModelImporter>(this)});
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
			std::string extension= modelFilePath.extension().string();
			auto importerIt = m_modelImporters.find(extension);
			if (importerIt != m_modelImporters.end())
			{
				IModelImporterPtr importer= importerIt->second;
				if (importer)
				{
					GlRenderModelResourcePtr resource= importer->importModelFromFile(modelFilePath, overrideMaterial);
					if (resource)
					{
						m_renderModelCache.insert({modelPathString, resource});
						return resource;
					}
				}
			}
			else
			{
				MIKAN_LOG_ERROR("GlModelResourceManager::fetchRenderModel") << 
					"No model importer found for extension: " << extension <<
					", for file: " << modelPathString;
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

