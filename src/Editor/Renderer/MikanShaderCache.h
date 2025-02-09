#pragma once

#include "AssetFwd.h"
#include "MikanRendererFwd.h"
#include "IMkShaderCache.h"

class MikanShaderCache 
{
public:
	MikanShaderCache()= delete;
	MikanShaderCache(IMkWindow* ownerWindow);

	inline IMkShaderCachePtr getMkShaderCache() { return m_shaderCache; }

	bool startup();
	void shutdown();

	MkMaterialPtr loadMaterialAssetReference(MaterialAssetReferencePtr materialAssetRef);

	MkMaterialPtr registerMaterial(IMkShaderCodeConstPtr code);
	MkMaterialConstPtr getMaterialByName(const std::string& name);

	IMkShaderPtr fetchCompiledIMkShader(IMkShaderCodeConstPtr code);

protected:
	IMkShaderCodeConstPtr loadShaderCodeFromConfigData(const MikanShaderConfig& config);

private:
	IMkShaderCachePtr m_shaderCache;
};