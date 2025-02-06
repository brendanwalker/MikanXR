#include "GlMaterial.h"
#include "GlModelResourceManager.h"
#include "GlRenderModelResource.h"
#include "GlProgram.h"
#include "GlShaderCache.h"
#include "GlVertexDefinition.h"
#include "IMkWindow.h"
#include "Logger.h"
#include "ObjModelImporter.h"
#include "ObjModelExporter.h"

GlModelResourceManager::GlModelResourceManager(IMkWindow* ownerWindow)
	: m_ownerWindow(ownerWindow)
{
	// Register model importers
	m_modelImporters.insert({".obj", std::make_shared<ObjModelImporter>(this)});

	// Register model exporters
	m_modelExporters.insert({".obj", std::make_shared<ObjModelExporter>(this)});
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
		std::string materialName= overrideMaterial ? overrideMaterial->getName() : "/default_material";
		std::string cacheKey = modelPathString + materialName;

		auto it = m_renderModelCache.find(cacheKey);
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
					GlRenderModelResourcePtr resource= 
						importer->importModelFromFile(modelFilePath, overrideMaterial);
					if (resource)
					{
						std::string modelName = resource->getModelFilePath().string();
						m_renderModelCache.insert({cacheKey, resource});
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

bool GlModelResourceManager::flushModelByFilePathFromCache(const std::filesystem::path& modelFilePath)
{
	std::string modelPathString = modelFilePath.string();
	auto it = m_renderModelCache.find(modelPathString);
	if (it != m_renderModelCache.end())
	{
		m_renderModelCache.erase(it);
		return true;
	}

	return false;
}

bool GlModelResourceManager::exportModelToFile(
	GlRenderModelResourcePtr modelResource,
	const std::filesystem::path& modelPath)
{
	std::string modelPathString = modelPath.string();
	std::string extension = modelPath.extension().string();
	auto exporterIt = m_modelExporters.find(extension);
	if (exporterIt != m_modelExporters.end())
	{
		IModelExporterPtr exporter = exporterIt->second;
		if (exporter)
		{
			return exporter->exportModelToFile(modelResource, modelPath);
		}
	}
	else
	{
		MIKAN_LOG_ERROR("GlModelResourceManager::exportModelToFile") <<
			"No model exporter found for extension: " << extension <<
			", for file: " << modelPathString;
	}

	return false;
}