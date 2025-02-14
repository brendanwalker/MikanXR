#pragma once

#include "AssetFwd.h"
#include "MikanRendererFwd.h"
#include "IMkShaderCache.h"

class MikanShaderCache : public IMkShaderCache
{
public:
	MikanShaderCache()= delete;
	MikanShaderCache(IMkWindow* ownerWindow);

	virtual bool startup() override;
	virtual void shutdown() override;
	virtual MkMaterialPtr registerMaterial(IMkShaderCodeConstPtr code) override;
	virtual MkMaterialConstPtr getMaterialByName(const std::string& name) override;
	virtual IMkShaderPtr fetchCompiledIMkShader(IMkShaderCodeConstPtr code) override;

	MkMaterialPtr loadMaterialAssetReference(MaterialAssetReferencePtr materialAssetRef);

protected:
	IMkShaderCodeConstPtr loadShaderCodeFromConfigData(const MikanShaderConfig& config);

private:
	IMkShaderCachePtr m_shaderCache;
};