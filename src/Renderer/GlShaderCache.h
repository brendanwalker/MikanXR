#pragma once

#include "AssetFwd.h"
#include "RendererFwd.h"

#include <memory>
#include <string>
#include <map>

#define INTERNAL_MATERIAL_PT_FULLSCREEN_TEXTURE		"Internal_PT_FullscreenTexture"
#define INTERNAL_MATERIAL_PNT_BLINN_PHONG			"Internal_PNT_BlingPhong"
#define INTERNAL_MATERIAL_P_WIREFRAME				"Internal_P_Wireframe"
#define INTERNAL_MATERIAL_P_SOLID_COLOR				"Internal_P_SolidColor"
#define INTERNAL_MATERIAL_PT_TEXTURED				"Internal_PT_Textured"
#define INTERNAL_MATERIAL_PNT_TEXTURED_COLORED		"Internal_PNT_TexturedColored"

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
	std::map<std::string, GlProgramPtr> m_programCache;
	std::map<std::string, GlMaterialPtr> m_materialCache;
};