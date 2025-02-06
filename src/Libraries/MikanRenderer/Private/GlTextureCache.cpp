#include "GlTextureCache.h"
#include "IMkTexture.h"
#include "TextureAssetReference.h"
#include "Logger.h"
#include "PathUtils.h"

bool GlTextureCache::startup()
{
	std::filesystem::path texturePath= PathUtils::getResourceDirectory() / "textures";

	bool bSuccess= true;
	bSuccess&= loadTexturePath(texturePath / "whiteRGB.png", INTERNAL_TEXTURE_WHITE_RGB) != nullptr;
	bSuccess&= loadTexturePath(texturePath / "blackRGB.png", INTERNAL_TEXTURE_BLACK_RGB) != nullptr;
	bSuccess&= loadTexturePath(texturePath / "whiteRGBA.png", INTERNAL_TEXTURE_WHITE_RGBA) != nullptr;
	bSuccess&= loadTexturePath(texturePath / "blackRGBA.png", INTERNAL_TEXTURE_BLACK_RGBA) != nullptr;

	return bSuccess;
}

void GlTextureCache::shutdown()
{
	m_textureCache.clear();
}

IMkTexturePtr GlTextureCache::tryGetTextureByName(const std::string& textureName)
{
	auto it = m_textureCache.find(textureName);
	if (it != m_textureCache.end())
	{
		return it->second;
	}

	return IMkTexturePtr();
}

IMkTexturePtr GlTextureCache::loadTextureAssetReference(TextureAssetReferencePtr textureAssetRef)
{
	return loadTexturePath(textureAssetRef->getAssetPath());
}

IMkTexturePtr GlTextureCache::loadTexturePath(
	const std::filesystem::path& texturePath,
	const std::string& overrideName)
{
	IMkTexturePtr texture;

	if (!texturePath.empty() && std::filesystem::exists(texturePath))
	{
		texture = tryGetTextureByName(texturePath.string());

		if (texture == nullptr)
		{
			texture = CreateMkTexture();
			texture->setImagePath(texturePath);
			if (texture->reloadTextureFromImagePath())
			{
				std::string textureName = !overrideName.empty() ? overrideName : texturePath.string();

				m_textureCache.insert({textureName, texture});
			}
			else
			{
				MIKAN_LOG_ERROR("GlTextureCache::loadTexturePath()") 
					<< "Failed to load texture: " << texturePath.string();
			}
		}
	}

	return texture;
}

bool GlTextureCache::removeTexureFromCache(IMkTexturePtr texture)
{
	if (texture)
	{
		std::string texturePath = texture->getImagePath().string();
		auto it = m_textureCache.find(texturePath);
		if (it != m_textureCache.end())
		{
			m_textureCache.erase(it);
			return true;
		}
	}
	return false;
}