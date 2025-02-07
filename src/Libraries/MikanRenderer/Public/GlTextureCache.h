#pragma once

#include "AssetFwd.h"
#include "MikanRendererFwd.h"

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

	IMkTexturePtr tryGetTextureByName(const std::string& textureName);
	IMkTexturePtr loadTextureAssetReference(TextureAssetReferencePtr textureAssetRef);
	IMkTexturePtr loadTexturePath(const std::filesystem::path& texturePath, const std::string& overrideName= "");
	bool removeTexureFromCache(IMkTexturePtr texture);

private:
	std::map<std::string, IMkTexturePtr> m_textureCache;
};