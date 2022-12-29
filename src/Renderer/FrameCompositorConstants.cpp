#include "FrameCompositorConstants.h"

const std::string g_compositorLayerAlphaStrings[(int)eCompositorLayerAlphaMode::COUNT] = {
	"NoAlpha",
	"ColorKey",
	"AlphaChannel",
	"MagicPortal"
};
const std::string* k_compositorLayerAlphaStrings = g_compositorLayerAlphaStrings;