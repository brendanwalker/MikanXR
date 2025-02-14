#include "IMkTextureCache.h"
#include "IMkTexture.h"
#include "Logger.h"

class GlTextureCache : public IMkTextureCache
{
public:
	GlTextureCache() = delete;
	GlTextureCache(IMkWindow* ownerWindow) : m_ownerWindow(ownerWindow) {}
	virtual ~GlTextureCache() 
	{
		shutdown();	
	}

	virtual bool startup() override
	{
		return true;
	}

	virtual void shutdown() override
	{
		m_textureCache.clear();
	}

	virtual IMkTexturePtr tryGetTextureByName(const std::string& textureName) override
	{
		auto it = m_textureCache.find(textureName);
		if (it != m_textureCache.end())
		{
			return it->second;
		}

		return IMkTexturePtr();
	}

	virtual IMkTexturePtr loadTexturePath(
		const std::filesystem::path& texturePath,
		const std::string& overrideName) override
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

	virtual bool removeTexureFromCache(IMkTexturePtr texture) override
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

private:
	IMkWindow* m_ownerWindow;
	std::map<std::string, IMkTexturePtr> m_textureCache;
};

IMkTextureCachePtr createMkTextureCache(class IMkWindow* ownerWindow)
{
	return std::make_shared<GlTextureCache>(ownerWindow);
}