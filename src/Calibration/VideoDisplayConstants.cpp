#include "VideoDisplayConstants.h"

const std::string g_videoDisplayModeStrings[(int)eVideoDisplayMode::COUNT] = {
	"BGR",
	"Undistorted",
	"Grayscale"
};
const std::string* k_videoDisplayModeStrings = g_videoDisplayModeStrings;

const std::string g_videoTextureStrings[(int)eVideoTextureSource::COUNT] = {
	"Video Texture",
	"Distortion Texture",
	"Float Depth Texture",
	"RGB Depth Texture",
};
const std::string* k_videoTextureStrings = g_videoTextureStrings;