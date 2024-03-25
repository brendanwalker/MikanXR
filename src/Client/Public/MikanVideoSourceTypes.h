#pragma once

#include "MikanClientTypes.h"
#include "MikanMathTypes.h"

// Constants
enum MikanIntrinsicsType
{
	INVALID_CAMERA_INTRINSICS,
	MONO_CAMERA_INTRINSICS,
	STEREO_CAMERA_INTRINSICS,
};

// Structures

/// Radial and tangential lens distortion coefficients computed during lens lens calibration
/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
struct MikanDistortionCoefficients
{
	inline static const std::string k_typeName = "MikanMonoIntrinsics";

	double k1; ///< Radial Distortion Parameter 1 (r^2 numerator constant)
	double k2; ///< Radial Distortion Parameter 2 (r^4 numerator constant)
	double k3; ///< Radial Distortion Parameter 3 (r^6 numerator constant)
	double k4; ///< Radial Distortion Parameter 4 (r^2 divisor constant)
	double k5; ///< Radial Distortion Parameter 5 (r^4 divisor constant)
	double k6; ///< Radial Distortion Parameter 6 (r^6 divisor constant)
	double p1; ///< Tangential Distortion Parameter 1
	double p2; ///< Tangential Distortion Parameter 2
};

/// Camera intrinsic properties for a monoscopic camera
/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
struct MikanMonoIntrinsics
{
	inline static const std::string k_typeName = "MikanMonoIntrinsics";

	double pixel_width;  ///< Width of the camera buffer in pixels
	double pixel_height; ///< Height of the camera buffer in pixels
	double hfov;         ///< The horizontal field of view camera in degrees
	double vfov;         ///< The vertical field of view camera in degrees
	double znear;        ///< The distance of the near clipping plane in cm
	double zfar;         ///< The distance of the far clipping plane in cm
	MikanDistortionCoefficients distortion_coefficients;   ///< Lens distortion coefficients
	MikanMatrix3d camera_matrix;   ///< Intrinsic camera matrix containing focal lengths and principal point
};

/// Camera intrinsic properties for a stereoscopic camera
/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
struct MikanStereoIntrinsics
{
	inline static const std::string k_typeName = "MikanStereoIntrinsics";

	// Keep these in sync with PSVRMonoTrackerIntrinsics
	double pixel_width;  ///< Width of the camera buffer in pixels
	double pixel_height; ///< Height of the camera buffer in pixels
	double hfov;         ///< The horizontal field of view camera in degrees
	double vfov;         ///< The vertical field of view camera in degrees
	double znear;        ///< The distance of the near clipping plane in cm
	double zfar;         ///< The distance of the far clipping plane in cm
	MikanDistortionCoefficients left_distortion_coefficients; ///< Left lens distortion coefficients
	MikanMatrix3d left_camera_matrix; ///< Intrinsic matrix for left camera containing focal lengths and principal point

	MikanDistortionCoefficients right_distortion_coefficients; ///< Right lens distortion coefficients
	MikanMatrix3d right_camera_matrix; ///< Intrinsic matrix for rotation camera containing focal lengths and principal point
	MikanMatrix3d left_rectification_rotation; ///< Rotation applied to left camera to rectify the image
	MikanMatrix3d right_rectification_rotation; ///< Rotation applied to right camera to rectify the image
	MikanMatrix4x3d left_rectification_projection; ///< Projection applied to left camera to rectify the image
	MikanMatrix4x3d right_rectification_projection; ///< Projection applied to right camera to rectify the image
	MikanMatrix3d rotation_between_cameras; ///< Rotation between the left and right cameras
	MikanVector3d translation_between_cameras; ///< Translation between the left and right camera
	MikanMatrix3d essential_matrix; ///< Transform relating points in unit coordinate space between cameras
	MikanMatrix3d fundamental_matrix; ///< Transform relating points in pixel coordinates between cameras
	MikanMatrix4d reprojection_matrix;  ///< Transform relating pixel x,y + disparity to distance from cameras
};

/// Bundle containing all intrinsic video source properties
struct MikanVideoSourceIntrinsics : public MikanResponse
{
	inline static const std::string k_typeName = "MikanVideoSourceIntrinsics";

	union
	{
		MikanMonoIntrinsics mono;
		MikanStereoIntrinsics stereo;
	} intrinsics;

	MikanIntrinsicsType intrinsics_type;

	MikanVideoSourceIntrinsics() : MikanResponse(k_typeName) {}
};

/// Static properties 
struct MikanVideoSourceAttachmentInfo : public MikanResponse
{
	inline static const std::string k_typeName = "MikanVideoSourceAttachmentInfo";

	MikanVRDeviceID attached_vr_device_id;
	MikanMatrix4f vr_device_offset_xform;

	MikanVideoSourceAttachmentInfo() : MikanResponse(k_typeName) {}
};

/// Static properties about a video source
struct MikanVideoSourceMode : public MikanResponse
{
	inline static const std::string k_typeName = "MikanVideoSourceMode";

	MikanVideoSourceType video_source_type;
	MikanVideoSourceApi video_source_api;
	std::string device_path;
	std::string video_mode_name;
	int32_t resolution_x;
	int32_t resolution_y;
	float frame_rate;

	MikanVideoSourceMode() : MikanResponse(k_typeName) {}
};