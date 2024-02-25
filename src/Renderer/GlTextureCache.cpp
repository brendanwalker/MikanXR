#include "GlTextureCache.h"
#include "GlTexture.h"
#include "TextureAssetReference.h"
#include "Logger.h"

bool GlTextureCache::startup()
{
	return true;
}

void GlTextureCache::shutdown()
{
	m_textureCache.clear();
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

GlTexturePtr GlTextureCache::loadTextureAssetReference(TextureAssetReferencePtr textureAssetRef)
{
	return loadTexturePath(textureAssetRef->getAssetPath());
}

GlTexturePtr GlTextureCache::loadTexturePath(const std::filesystem::path& texturePath)
{
	if (!texturePath.empty() && std::filesystem::exists(texturePath))
	{
		auto it = m_textureCache.find(texturePath.string());
		if (it != m_textureCache.end())
		{
			return it->second;
		}
		else
		{
			GlTexturePtr texture = std::make_shared<GlTexture>();
			texture->setImagePath(texturePath);
			if (texture->reloadTextureFromImagePath())
			{
				m_textureCache.insert({texturePath.string(), texture});
				return texture;
			}
			else
			{
				MIKAN_LOG_ERROR("GlTextureCache::loadTexturePath()") 
					<< "Failed to load texture: " << texturePath.string();
			}
		}
	}

	return GlTexturePtr();
}
