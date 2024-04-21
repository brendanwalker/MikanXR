#pragma once

#include "AssetFwd.h"
#include "RendererFwd.h"

#include <filesystem>
#include <memory>
#include <string>
#include <map>

#define INTERNAL_TEXTURE_WHITE_RGB		"Internal_White_RGB"
#define INTERNAL_TEXTURE_BLACK_RGB		"Internal_Black_RGB"
#define INTERNAL_TEXTURE_WHITE_RGBA		"Internal_White_RGBA"
#define INTERNAL_TEXTURE_BLACK_RGBA		"Internal_Black_RGBA"

class GlTextureCache
{
public:
	GlTextureCache()= default;

	bool startup();
	void shutdown();

	GlTexturePtr tryGetTextureByName(const std::string& textureName);
	GlTexturePtr loadTextureAssetReference(TextureAssetReferencePtr textureAssetRef);
	GlTexturePtr loadTexturePath(const std::filesystem::path& texturePath, const std::string& overrideName= "");
	bool removeTexureFromCache(GlTexturePtr texture);

private:
	std::map<std::string, GlTexturePtr> m_textureCache;
};