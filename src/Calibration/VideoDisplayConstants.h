#pragma once

enum eVideoDisplayMode
{
	mode_bgr,
	mode_undistored,
	mode_grayscale,

	MAX_VIDEO_DISPLAY_MODES
};

#define VIDEO_FRAME_HAS_BGR_UNDISTORT_FLAG		0x0001
#define VIDEO_FRAME_HAS_GRAYSCALE_FLAG			0x0002
#define VIDEO_FRAME_HAS_GL_TEXTURE_FLAG			0x0004
#define VIDEO_FRAME_HAS_ALL						0xffff