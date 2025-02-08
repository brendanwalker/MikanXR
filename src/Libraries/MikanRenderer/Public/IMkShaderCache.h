#pragma once

#include "MkRendererExport.h"
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

class IMkShaderCache
{
public:
	virtual	~IMkShaderCache() {}

	virtual bool startup() = 0;
	virtual void shutdown() = 0;

	virtual GlMaterialPtr registerMaterial(IMkShaderCodeConstPtr code) = 0;
	virtual GlMaterialConstPtr getMaterialByName(const std::string& name) = 0;

	virtual IMkShaderPtr fetchCompiledIMkShader(IMkShaderCodeConstPtr code) = 0;
};

MIKAN_RENDERER_FUNC(IMkShaderCachePtr) CreateMkShaderCache(class IMkWindow* ownerWindow);