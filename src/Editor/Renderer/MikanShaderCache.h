#pragma once

#include "AssetFwd.h"
#include "MikanRendererFwd.h"
#include "IMkShaderCache.h"
#include "MulticastDelegate.h"

#include <filesystem>
#include <string>

class MikanShaderCache : public IMkShaderCache
{
public:
	MikanShaderCache()= delete;
	MikanShaderCache(IMkGraphicsContext* graphicsContext);

	virtual bool startup() override;
	virtual void shutdown() override;

	// The texture cache that resolves a material's default texture paths. Set by
	// the owning window once both caches exist; without it texture defaults are skipped.
	void setTextureCache(class MikanTextureCache* textureCache) { m_textureCache= textureCache; }
	virtual MkMaterialPtr registerMaterial(IMkShaderCodeConstPtr code) override;
	virtual MkMaterialConstPtr getMaterialByName(const std::string& name) override;
	virtual IMkShaderPtr fetchCompiledIMkShader(IMkShaderCodeConstPtr code) override;

	// Optionally hands back the .mat config the material was loaded from, so a
	// caller can read the material's domain without a second config load
	static std::string makeProgramName(const std::filesystem::path& materialPath);
	MkMaterialPtr loadMaterialAssetReference(MaterialAssetReferencePtr materialAssetRef,
											 MikanShaderConfig* outConfig= nullptr);

	// Reload a .mat whose files changed on disk into the MkMaterial this cache
	// already handed out for it, so every consumer holding that material sees
	// the recompiled program. Returns false when this cache never loaded it.
	bool reloadMaterialByPath(const std::filesystem::path& materialPath);
	MulticastDelegate<void(MkMaterialPtr material)> OnMaterialReloaded;

	// Build shader code from in-memory sources, registering the config's vertex
	// attributes and uniform semantics on it. Null when the config names an
	// attribute type or uniform semantic that does not exist.
	IMkShaderCodePtr createShaderCode(const MikanShaderConfig& config, const std::string& programName,
									  const std::string& vertexSource, const std::string& fragmentSource);

protected:
	IMkShaderCodeConstPtr loadShaderCodeFromConfigData(const MikanShaderConfig& config);
	// Write the config's uniformDefaults into the material's default value tables
	void applyMaterialDefaults(MkMaterialPtr material, const MikanShaderConfig& config);

private:
	IMkShaderCachePtr m_shaderCache;
	class MikanTextureCache* m_textureCache= nullptr;
};
