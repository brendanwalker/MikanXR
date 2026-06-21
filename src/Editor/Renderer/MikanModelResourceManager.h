#pragma once

#include "MikanRendererFwd.h"
#include "IModelImporter.h"
#include "IModelExporter.h"

#include <filesystem>
#include <map>
#include <memory>
#include <string>

class MikanModelResourceManager
{
public:
	MikanModelResourceManager(class IMkGraphicsContext* ownerGraphicsContext);
	virtual ~MikanModelResourceManager();

	bool startup();
	void shutdown();

	inline IMkGraphicsContext* getGraphicsContext() const { return m_ownerGraphicsContext; }
	inline class MikanShaderCache* getShaderCache() const { return m_shaderCache.get(); }

	MikanRenderModelResourcePtr fetchRenderModel(const std::filesystem::path& modelFilePath,
												 MkMaterialConstPtr overrideMaterial= MkMaterialConstPtr());
	bool flushModelByFilePathFromCache(const std::filesystem::path& modelFilePath);

	bool exportModelToFile(MikanRenderModelResourcePtr modelResource, const std::filesystem::path& modelPath);

private:
	class IMkGraphicsContext* m_ownerGraphicsContext= nullptr;

	std::unique_ptr<class MikanShaderCache> m_shaderCache;
	std::map<std::string, MikanRenderModelResourcePtr> m_renderModelCache;
	std::map<std::string, IModelImporterPtr> m_modelImporters;
	std::map<std::string, IModelExporterPtr> m_modelExporters;
};
