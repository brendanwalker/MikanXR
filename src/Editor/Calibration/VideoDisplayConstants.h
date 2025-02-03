#pragma once

#include <string>

#define VIDEO_FRAME_HAS_BGR_UNDISTORT_FLAG		0x0001
#define VIDEO_FRAME_HAS_GRAYSCALE_FLAG			0x0002
#define VIDEO_FRAME_HAS_GL_TEXTURE_FLAG			0x0004
#define VIDEO_FRAME_HAS_ALL						0xffff

enum class eVideoDisplayMode : int
{
	INVALID = -1,

	mode_bgr,
	mode_undistored,
	mode_grayscale,

	COUNT
};
extern const std::string* k_videoDisplayModeStrings;

enum class eVideoTextureSource : int
{
	INVALID = -1,

	video_texture,
	distortion_texture,
	float_depth_texture,
	color_mapped_depth_texture,

	COUNT
};
extern const std::string* k_videoTextureStrings;