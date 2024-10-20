#pragma once

#include "MikanMathTypes.h"
#include "MikanVRDeviceTypes.h"

#ifdef REFLECTION_CODE_BUILT
#include "MikanVideoSourceTypes.rfkh.h"
#endif

// Constants
enum ENUM() MikanVideoSourceType
{
	MikanVideoSourceType_MONO,
	MikanVideoSourceType_STEREO
};

/// The list of possible camera APIs used by MikanXR
enum ENUM() MikanVideoSourceApi
{
	MikanVideoSourceApi_INVALID,
	MikanVideoSourceApi_OPENCV_CV,
	MikanVideoSourceApi_WINDOWS_MEDIA_FOUNDATION
};

enum ENUM() MikanIntrinsicsType
{
	INVALID_CAMERA_INTRINSICS,
	MONO_CAMERA_INTRINSICS,
	STEREO_CAMERA_INTRINSICS,
};

// Structures

/// Radial and tangential lens distortion coefficients computed during lens lens calibration
/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
struct STRUCT() MikanDistortionCoefficients
{
	inline static const std::string k_typeName = "MikanMonoIntrinsics";

	FIELD()
	double k1; ///< Radial Distortion Parameter 1 (r^2 numerator constant)
	FIELD()
	double k2; ///< Radial Distortion Parameter 2 (r^4 numerator constant)
	FIELD()
	double k3; ///< Radial Distortion Parameter 3 (r^6 numerator constant)
	FIELD()
	double k4; ///< Radial Distortion Parameter 4 (r^2 divisor constant)
	FIELD()
	double k5; ///< Radial Distortion Parameter 5 (r^4 divisor constant)
	FIELD()
	double k6; ///< Radial Distortion Parameter 6 (r^6 divisor constant)
	FIELD()
	double p1; ///< Tangential Distortion Parameter 1
	FIELD()
	double p2; ///< Tangential Distortion Parameter 2

	#ifdef REFLECTION_CODE_BUILT
	MikanDistortionCoefficients_GENERATED
	#endif
};

/// Camera intrinsic common properties
/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
struct STRUCT() MikanCameraIntrinsics
{
	inline static const std::string k_typeName = "MikanCameraIntrinsics";

	FIELD()
	double pixel_width;  ///< Width of the camera buffer in pixels
	FIELD()
	double pixel_height; ///< Height of the camera buffer in pixels
	FIELD()
	double hfov;         ///< The horizontal field of view camera in degrees
	FIELD()
	double vfov;         ///< The vertical field of view camera in degrees
	FIELD()
	double znear;        ///< The distance of the near clipping plane in cm
	FIELD()
	double zfar;         ///< The distance of the far clipping plane in cm

	#ifdef REFLECTION_CODE_BUILT
	MikanCameraIntrinsics_GENERATED
	#endif
};

/// Camera intrinsic properties for a monoscopic camera
/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
struct STRUCT() MikanMonoIntrinsics : public MikanCameraIntrinsics
{
	inline static const std::string k_typeName = "MikanMonoIntrinsics";

	FIELD()
	MikanDistortionCoefficients distortion_coefficients;   ///< Lens distortion coefficients
	FIELD()
	MikanMatrix3d camera_matrix;   ///< Intrinsic camera matrix containing focal lengths and principal point

	#ifdef REFLECTION_CODE_BUILT
	MikanMonoIntrinsics_GENERATED
	#endif
};

/// Camera intrinsic properties for a stereoscopic camera
/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
struct STRUCT() MikanStereoIntrinsics : public MikanCameraIntrinsics
{
	inline static const std::string k_typeName = "MikanStereoIntrinsics";

	FIELD()
	MikanDistortionCoefficients left_distortion_coefficients; ///< Left lens distortion coefficients
	FIELD()
	MikanMatrix3d left_camera_matrix; ///< Intrinsic matrix for left camera containing focal lengths and principal point

	FIELD()
	MikanDistortionCoefficients right_distortion_coefficients; ///< Right lens distortion coefficients
	FIELD()
	MikanMatrix3d right_camera_matrix; ///< Intrinsic matrix for rotation camera containing focal lengths and principal point
	FIELD()
	MikanMatrix3d left_rectification_rotation; ///< Rotation applied to left camera to rectify the image
	FIELD()
	MikanMatrix3d right_rectification_rotation; ///< Rotation applied to right camera to rectify the image
	FIELD()
	MikanMatrix4x3d left_rectification_projection; ///< Projection applied to left camera to rectify the image
	FIELD()
	MikanMatrix4x3d right_rectification_projection; ///< Projection applied to right camera to rectify the image
	FIELD()
	MikanMatrix3d rotation_between_cameras; ///< Rotation between the left and right cameras
	FIELD()
	MikanVector3d translation_between_cameras; ///< Translation between the left and right camera
	FIELD()
	MikanMatrix3d essential_matrix; ///< Transform relating points in unit coordinate space between cameras
	FIELD()
	MikanMatrix3d fundamental_matrix; ///< Transform relating points in pixel coordinates between cameras
	FIELD()
	MikanMatrix4d reprojection_matrix;  ///< Transform relating pixel x,y + disparity to distance from cameras

	#ifdef REFLECTION_CODE_BUILT
	MikanStereoIntrinsics_GENERATED
	#endif
};

/// Bundle containing all intrinsic video source properties
struct STRUCT() MikanVideoSourceIntrinsics : public MikanResponse
{
	inline static const std::string k_typeName = "MikanVideoSourceIntrinsics";

	//TODO: DELETE THIS
	union
	{
		MikanMonoIntrinsics mono;
		MikanStereoIntrinsics stereo;
	} intrinsics;
	//TODO: DELETE THIS

	FIELD()
	std::shared_ptr<MikanCameraIntrinsics> intrinsics_ptr;

	FIELD()
	MikanIntrinsicsType intrinsics_type;

	MikanVideoSourceIntrinsics() : MikanResponse(k_typeName) {}

	#ifdef REFLECTION_CODE_BUILT
	MikanVideoSourceIntrinsics_GENERATED
	#endif
};

/// Static properties 
struct STRUCT() MikanVideoSourceAttachmentInfo : public MikanResponse
{
	inline static const std::string k_typeName = "MikanVideoSourceAttachmentInfo";

	FIELD()
	MikanVRDeviceID attached_vr_device_id;
	FIELD()
	MikanMatrix4f vr_device_offset_xform;

	MikanVideoSourceAttachmentInfo() : MikanResponse(k_typeName) {}

	#ifdef REFLECTION_CODE_BUILT
	MikanVideoSourceAttachmentInfo_GENERATED
	#endif
};

/// Static properties about a video source
struct STRUCT() MikanVideoSourceMode : public MikanResponse
{
	inline static const std::string k_typeName = "MikanVideoSourceMode";

	FIELD()
	MikanVideoSourceType video_source_type;
	FIELD()
	MikanVideoSourceApi video_source_api;
	FIELD()
	std::string device_path;
	FIELD()
	std::string video_mode_name;
	FIELD()
	int32_t resolution_x;
	FIELD()
	int32_t resolution_y;
	FIELD()
	float frame_rate;

	MikanVideoSourceMode() : MikanResponse(k_typeName) {}

	#ifdef REFLECTION_CODE_BUILT
	MikanVideoSourceMode_GENERATED
	#endif
};

#ifdef REFLECTION_CODE_BUILT
File_MikanVideoSourceTypes_GENERATED
#endif