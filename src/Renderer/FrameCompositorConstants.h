#pragma once

#include <string>

#define DEFAULT_COMPOSITOR_CONFIG_NAME			"Alpha Channel"
#define DEFAULT_COMPOSITOR_VIDEO_MATERIAL_NAME	"rgbUndistortionFrame"
#define DEFAULT_COMPOSITOR_CLIENT_MATERIAL_NAME	"rgbaFrame"
#define STENCIL_MVP_UNIFORM_NAME				"mvpMatrix"
#define MAX_CLIENT_SOURCES						8
#define EMPTY_SOURCE_NAME						"empty"

enum class eSupportedCodec : int
{
	INVALID = -1,

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
	INVALID = -1,

	NoAlpha,
	ColorKey,
	AlphaChannel,
	MagicPortal,

	COUNT
};
extern const std::string* k_compositorLayerAlphaStrings;

enum class eCompositorStencilMode
{
	INVALID = -1,

	noStencil,
	insideStencil,
	outsideStencil,

	COUNT
};
extern const std::string* k_compositorStencilModeStrings;

enum class eCompositorBlendMode
{
	INVALID = -1,

	blendOff,
	blendOn,

	COUNT
};
extern const std::string* k_compositorBlendModeStrings;