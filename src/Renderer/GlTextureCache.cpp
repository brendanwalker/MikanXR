#include "GlTextureCache.h"
#include "GlTexture.h"
#include "TextureAssetReference.h"
#include "Logger.h"
#include "PathUtils.h"

bool GlTextureCache::startup()
{
	std::filesystem::path texturePath= PathUtils::getResourceDirectory() / "textures";

	bool bSuccess= loadTexturePath(texturePath / "white.png", INTERNAL_TEXTURE_WHITE) != nullptr;
	bSuccess&= loadTexturePath(texturePath / "black.png", INTERNAL_TEXTURE_BLACK) != nullptr;

	return bSuccess;
}

void GlTextureCache::shutdown()
{
	m_textureCache.clear();
}

GlTexturePtr GlTextureCache::tryGetTextureByName(const std::string& textureName)
{
	auto it = m_textureCache.find(textureName);
	if (it != m_textureCache.end())
	{
		return it->second;
	}

	return GlTexturePtr();
}

GlTexturePtr GlTextureCache::loadTextureAssetReference(TextureAssetReferencePtr textureAssetRef)
{
	return loadTexturePath(textureAssetRef->getAssetPath());
}

GlTexturePtr GlTextureCache::loadTexturePath(
	const std::filesystem::path& texturePath,
	const std::string& overrideName)
{
	GlTexturePtr texture;

	if (!texturePath.empty() && std::filesystem::exists(texturePath))
	{
		texture = tryGetTextureByName(texturePath.string());

		if (texture == nullptr)
		{
			texture = std::make_shared<GlTexture>();
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

bool GlTextureCache::removeTexureFromCache(GlTexturePtr texture)
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