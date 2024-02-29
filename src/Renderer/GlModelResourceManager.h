#pragma once

#include "RendererFwd.h"

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

	GlRenderModelResourcePtr fetchRenderModel(
		const std::filesystem::path& modelFilePath,
		GlMaterialConstPtr overrideMaterial= GlMaterialConstPtr());

private:
	class IGlWindow* m_ownerWindow= nullptr;

	std::map<std::string, GlRenderModelResourcePtr> m_renderModelCache;
};
