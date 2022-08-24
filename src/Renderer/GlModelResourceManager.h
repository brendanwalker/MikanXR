#pragma once

#include <string>
#include <map>

class GlModelResourceManager
{
public:
	GlModelResourceManager();
	virtual ~GlModelResourceManager();

	void init();
	void cleanup();

	class GlRenderModelResource* fetchRenderModel(
		const std::string& modelFilePath,
		const struct GlVertexDefinition* vertexDefinition);

private:
	std::map<std::string, class GlRenderModelResource*> m_renderModelCache;
};
