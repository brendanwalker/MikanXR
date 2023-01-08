#pragma once

#include <string>

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