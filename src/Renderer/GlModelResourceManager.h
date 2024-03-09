#pragma once

#include "RendererFwd.h"
#include "IModelImporter.h"

#include <filesystem>
#include <map>
#include <string>

class GlModelResourceManager
{
public:
	GlModelResourceManager(class IGlWindow* ownerWindow);
	virtual ~GlModelResourceManager();

	bool startup();
	void shutdown();

	inline IGlWindow* getOwnerWindow() const { return m_ownerWindow; }

	GlRenderModelResourcePtr fetchRenderModel(
		const std::filesystem::path& modelFilePath,
		GlMaterialConstPtr overrideMaterial= GlMaterialConstPtr());
	bool removeModelResourceFromCache(GlRenderModelResourcePtr modelResource);

private:
	class IGlWindow* m_ownerWindow= nullptr;

	std::map<std::string, GlRenderModelResourcePtr> m_renderModelCache;
	std::map<std::string, IModelImporterPtr> m_modelImporters;
};
