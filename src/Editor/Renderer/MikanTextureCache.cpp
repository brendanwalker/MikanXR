#include "MikanTextureCache.h"
#include "IMkTexture.h"
#include "IMkTextureCache.h"
#include "TextureAssetReference.h"
#include "Logger.h"
#include "PathUtils.h"

MikanTextureCache::MikanTextureCache(IMkWindow* ownerWindow)
	: m_textureCache(createMkTextureCache(ownerWindow))
{
}

bool MikanTextureCache::startup()
{
	std::filesystem::path texturePath= PathUtils::getResourceDirectory() / "textures";

	bool bSuccess= true;
	bSuccess&= loadTexturePath(texturePath / "whiteRGB.png", INTERNAL_TEXTURE_WHITE_RGB) != nullptr;
	bSuccess&= loadTexturePath(texturePath / "blackRGB.png", INTERNAL_TEXTURE_BLACK_RGB) != nullptr;
	bSuccess&= loadTexturePath(texturePath / "whiteRGBA.png", INTERNAL_TEXTURE_WHITE_RGBA) != nullptr;
	bSuccess&= loadTexturePath(texturePath / "blackRGBA.png", INTERNAL_TEXTURE_BLACK_RGBA) != nullptr;

	return bSuccess;
}

void MikanTextureCache::shutdown()
{
	m_textureCache->shutdown();
}

IMkTexturePtr MikanTextureCache::tryGetTextureByName(const std::string& textureName)
{
	return m_textureCache->tryGetTextureByName(textureName);
}

IMkTexturePtr MikanTextureCache::loadTextureAssetReference(TextureAssetReferencePtr textureAssetRef)
{
	return loadTexturePath(textureAssetRef->getAssetPath());
}

IMkTexturePtr MikanTextureCache::loadTexturePath(
	const std::filesystem::path& texturePath,
	const std::string& overrideName)
{
	return m_textureCache->loadTexturePath(texturePath, overrideName);
}

bool MikanTextureCache::removeTexureFromCache(IMkTexturePtr texture)
{
	return m_textureCache->removeTexureFromCache(texture);
}