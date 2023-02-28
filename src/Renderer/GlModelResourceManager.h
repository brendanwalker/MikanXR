#pragma once

#include <filesystem>
#include <map>
#include <string>

class GlModelResourceManager
{
public:
	GlModelResourceManager();
	virtual ~GlModelResourceManager();

	void init();
	void cleanup();

	class GlRenderModelResource* fetchRenderModel(
		const std::filesystem::path& modelFilePath,
		const struct GlVertexDefinition* vertexDefinition);

private:
	std::map<std::string, class GlRenderModelResource*> m_renderModelCache;
};
