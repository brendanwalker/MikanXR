#pragma once

#include <string>

enum eSupportedCodec
{
	SUPPORTED_CODEC_MP4V,
	SUPPORTED_CODEC_MJPG,
	SUPPORTED_CODEC_SELECT,

	SUPPORTED_CODEC_COUNT
};

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