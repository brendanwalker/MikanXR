#pragma once

#include "MikanRendererFwd.h"
#include "IModelImporter.h"
#include "IModelExporter.h"

#include <filesystem>
#include <map>
#include <string>

class MikanModelResourceManager
{
public:
	MikanModelResourceManager(class IMkWindow* ownerWindow);
	virtual ~MikanModelResourceManager();

	bool startup();
	void shutdown();

	inline IMkWindow* getOwnerWindow() const { return m_ownerWindow; }

	MikanRenderModelResourcePtr fetchRenderModel(
		const std::filesystem::path& modelFilePath,
		GlMaterialConstPtr overrideMaterial= GlMaterialConstPtr());
	bool flushModelByFilePathFromCache(const std::filesystem::path& modelFilePath);

	bool exportModelToFile(
		MikanRenderModelResourcePtr modelResource,
		const std::filesystem::path& modelPath);

private:
	class IMkWindow* m_ownerWindow= nullptr;

	std::map<std::string, MikanRenderModelResourcePtr> m_renderModelCache;
	std::map<std::string, IModelImporterPtr> m_modelImporters;
	std::map<std::string, IModelExporterPtr> m_modelExporters;
};
