#pragma once

#include <string>

#define DEFAULT_COMPOSITOR_CONFIG_NAME "Alpha Channel"
#define MAX_CLIENT_SOURCES 8
#define EMPTY_SOURCE_NAME "empty"

enum class eSupportedCodec : int
{
	INVALID= -1,

	MP4V,
	MJPG,
	RGBA,

	COUNT
};
extern const std::string* k_supportedCodecName;
extern const std::string* k_supportedCodecFileSuffix;
extern const int* k_supportedCodecFourCC;

enum class eCompositorLayerAlphaMode : int
{
	INVALID= -1,

	NoAlpha,
	ColorKey,
	AlphaChannel,
	MagicPortal,

	COUNT
};
extern const std::string* k_compositorLayerAlphaStrings;

enum class eCompositorStencilMode : int
{
	INVALID= -1,

	noStencil,
	insideStencil,
	outsideStencil,

	COUNT
};
extern const std::string* k_compositorStencilModeStrings;

enum class eCompositorBlendMode : int
{
	INVALID= -1,

	blendOff= 0,
	blendNormal= 1,
	blendMultiply= 2,

	COUNT
};
extern const std::string* k_compositorBlendModeStrings;

enum class eStencilCullMode : int
{
	INVALID= -1,

	none,
	zAxis,
	yAxis,
	xAxis,

	COUNT
};
extern const std::string* k_stencilCullModeStrings;

enum class eTextureSourceColorType : int
{
	INVALID= -1,
	// Base source color texture, intended to be composited using "Over"/"Normal" blending
	// i.e. result = source*a + video*(1-a)
	colorRGB,
	colorRGBA,
	// Shadow (light survival) factor buffer, intended to be composited multiplicatively; identity is white.
	// i.e. result = video*shadow
	shadowRGB,
	shadowRGBA,

	COUNT
};
extern const std::string* k_textureSourceColorTypeStrings;

enum class eTextureSourceDepthType : int
{
	INVALID= -1,

	depthPackRGBA,

	COUNT
};
extern const std::string* k_textureSourceDepthTypeStrings;

enum class eCompositorEvaluatorWindow : int
{
	INVALID= -1,

	mainWindow,
	editorWindow,

	COUNT
};
extern const std::string* k_compositorEvaluatorWindow;