#pragma once

#include <string>

enum class eVideoFrameProcessorMode : int
{
	CALIBRATION, // CPU-based OpenCV undistortion with grayscale support
	COMPOSITOR,  // GPU-based shader undistortion
};

#define LEFT_PROJECTION_INDEX 0
#define RIGHT_PROJECTION_INDEX 1

#define MONO_PROJECTION_COUNT 1
#define STEREO_PROJECTION_COUNT 2

#define MAX_PROJECTION_COUNT 2
#define PRIMARY_PROJECTION_INDEX LEFT_PROJECTION_INDEX

/// The list of possible sub sections to extract from a video frame
enum class VideoFrameSection : int
{
	Left= 0,   ///< The left frame from a stereo camera
	Right= 1,  ///< The right frame from a stereo camera
	Primary= 0 ///< The only frame from a stereo camera
};

enum class eVideoDisplayMode : int
{
	INVALID= -1,

	mode_bgr,
	mode_undistored,
	mode_grayscale,

	COUNT
};
extern const std::string* k_videoDisplayModeStrings;

enum class eVideoTextureSource : int
{
	INVALID= -1,

	video_texture,
	distortion_texture,

	COUNT
};
extern const std::string* k_videoTextureStrings;