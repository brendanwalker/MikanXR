#pragma once

#include "AssetFwd.h"
#include "RendererFwd.h"

#include <filesystem>
#include <memory>
#include <string>
#include <map>

class GlTextureCache
{
public:
	GlTextureCache()= default;

	bool startup();
	void shutdown();

	bool removeTexureFromCache(GlTexturePtr texture);
	GlTexturePtr loadTextureAssetReference(TextureAssetReferencePtr textureAssetRef);
	GlTexturePtr loadTexturePath(const std::filesystem::path& texturePath);

private:
	std::map<std::string, GlTexturePtr> m_textureCache;
};