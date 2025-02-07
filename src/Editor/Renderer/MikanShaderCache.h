#pragma once

#include "AssetFwd.h"
#include "MikanRendererFwd.h"
#include "IMkShaderCache.h"

class MikanShaderCache
{
public:
	MikanShaderCache()= delete;
	MikanShaderCache(IMkWindow* ownerWindow);

	bool startup();
	void shutdown();

	GlMaterialPtr loadMaterialAssetReference(MaterialAssetReferencePtr materialAssetRef);

	GlMaterialPtr registerMaterial(const GlProgramCode& code);
	GlMaterialConstPtr getMaterialByName(const std::string& name);

	GlProgramPtr fetchCompiledGlProgram(const GlProgramCode* code);

private:
	IMkShaderCachePtr m_shaderCache;
};