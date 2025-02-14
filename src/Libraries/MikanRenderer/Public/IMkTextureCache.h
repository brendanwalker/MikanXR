#pragma once

#include "MkRendererExport.h"
#include "MkRendererFwd.h"

#include <filesystem>
#include <memory>
#include <string>
#include <map>

class IMkTextureCache
{
public:
	virtual ~IMkTextureCache() {};

	virtual bool startup() = 0;
	virtual void shutdown() = 0;

	virtual IMkTexturePtr tryGetTextureByName(const std::string& textureName) = 0;
	virtual IMkTexturePtr loadTexturePath(
		const std::filesystem::path& texturePath, 
		const std::string& overrideName= "")  = 0;
	virtual bool removeTexureFromCache(IMkTexturePtr texture) = 0;
};

MIKAN_RENDERER_FUNC(IMkTextureCachePtr) createMkTextureCache(class IMkWindow* ownerWindow);