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

class MikanTextureCache
{
public:
	MikanTextureCache()= delete;
	MikanTextureCache(IMkWindow* ownerWindow);

	inline IMkTextureCachePtr getMkTextureCache() { return m_textureCache; }

	bool startup();
	void shutdown();

	IMkTexturePtr tryGetTextureByName(const std::string& textureName);
	IMkTexturePtr loadTextureAssetReference(TextureAssetReferencePtr textureAssetRef);
	IMkTexturePtr loadTexturePath(const std::filesystem::path& texturePath, const std::string& overrideName= "");
	bool removeTexureFromCache(IMkTexturePtr texture);

private:
	IMkTextureCachePtr m_textureCache;
};