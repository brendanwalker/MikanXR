#pragma once

#include "AssetFwd.h"
#include "MkRendererFwd.h"

#include <memory>
#include <string>
#include <map>

#define INTERNAL_MATERIAL_PT_FULLSCREEN_RGB_TEXTURE		"Internal_PT_FullscreenRGBTexture"
#define INTERNAL_MATERIAL_PT_FULLSCREEN_RGBA_TEXTURE	"Internal_PT_FullscreenRGBATexture"
#define INTERNAL_MATERIAL_TEXT							"Internal_Text"
#define INTERNAL_MATERIAL_UNPACK_RGBA_DEPTH_TEXTURE		"Internal_UnpackRGBADepthTexture"
#define INTERNAL_MATERIAL_P_WIREFRAME					"Internal_P_Wireframe"
#define INTERNAL_MATERIAL_P_SOLID_COLOR					"Internal_P_SolidColor"
#define INTERNAL_MATERIAL_PT_TEXTURED					"Internal_PT_Textured"
#define INTERNAL_MATERIAL_PNT_TEXTURED_LIT_COLORED		"Internal_PNT_TexturedLitColored"
#define INTERNAL_MATERIAL_P_LINEAR_DEPTH				"Internal_P_LinearDepth"

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