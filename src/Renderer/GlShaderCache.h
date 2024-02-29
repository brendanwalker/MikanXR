#pragma once

#include "AssetFwd.h"
#include "RendererFwd.h"

#include <memory>
#include <string>
#include <map>

#define INTERNAL_MATERIAL_BLINN_PHONG	"Internal_BlingPhong"
#define INTERNAL_MATERIAL_WIREFRAME		"Internal_Wireframe"

class GlShaderCache
{
public:
	GlShaderCache()= default;

	bool startup();
	void shutdown();

	GlMaterialPtr loadMaterialAssetReference(MaterialAssetReferencePtr materialAssetRef);

	GlMaterialPtr registerMaterial(const GlProgramCode& code);
	GlMaterialConstPtr getMaterialByName(const std::string& name);

	GlProgramPtr fetchCompiledGlProgram(const GlProgramCode* code);

private:
	static const GlProgramCode& getPhongShaderCode();
	static const GlProgramCode& getWireframeShaderCode();

	std::map<std::string, GlProgramPtr> m_programCache;
	std::map<std::string, GlMaterialPtr> m_materialCache;
};