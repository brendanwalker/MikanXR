#pragma once

#include "AssetFwd.h"
#include "MikanRendererFwd.h"
#include "IMkTextureCache.h"

#include <filesystem>
#include <memory>
#include <string>
#include <map>

#define INTERNAL_TEXTURE_WHITE_RGB		"Internal_White_RGB"
#define INTERNAL_TEXTURE_BLACK_RGB		"Internal_Black_RGB"
#define INTERNAL_TEXTURE_WHITE_RGBA		"Internal_White_RGBA"
#define INTERNAL_TEXTURE_BLACK_RGBA		"Internal_Black_RGBA"

class MikanTextureCache : public IMkTextureCache
{
public:
	MikanTextureCache()= delete;
	MikanTextureCache(IMkWindow* ownerWindow);

	IMkTexturePtr loadTextureAssetReference(TextureAssetReferencePtr textureAssetRef);

	virtual bool startup() override;
	virtual void shutdown() override;
	virtual IMkTexturePtr tryGetTextureByName(const std::string& textureName) override;
	virtual IMkTexturePtr loadTexturePath(
		const std::filesystem::path& texturePath, 
		const std::string& overrideName= "") override;
	virtual bool removeTexureFromCache(IMkTexturePtr texture) override;

private:
	IMkTextureCachePtr m_textureCache;
};