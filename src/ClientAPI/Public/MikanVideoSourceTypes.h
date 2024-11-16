#pragma once

#include "MikanAPIExport.h"
#include "MikanMathTypes.h"
#include "MikanVRDeviceTypes.h"
#include "SerializableObjectPtr.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanVideoSourceTypes.rfkh.h"
#endif

#include <assert.h>

// Constants
enum ENUM(Serialization::CodeGenModule("MikanVideoSourceTypes")) MikanVideoSourceType
{
	MikanVideoSourceType_MONO ENUMVALUE_STRING("MONO"),
	MikanVideoSourceType_STEREO ENUMVALUE_STRING("STEREO")
};

/// The list of possible camera APIs used by MikanXR
enum ENUM(Serialization::CodeGenModule("MikanVideoSourceTypes")) MikanVideoSourceApi
{
	MikanVideoSourceApi_INVALID ENUMVALUE_STRING("INVALID"),
	MikanVideoSourceApi_OPENCV_CV ENUMVALUE_STRING("OPEN_CV"),
	MikanVideoSourceApi_WINDOWS_MEDIA_FOUNDATION ENUMVALUE_STRING("WINDOWS_MEDIA_FOUNDATION")
};

enum ENUM(Serialization::CodeGenModule("MikanVideoSourceTypes")) MikanIntrinsicsType
{
	INVALID_CAMERA_INTRINSICS ENUMVALUE_STRING("INVALID"),
	MONO_CAMERA_INTRINSICS  ENUMVALUE_STRING("MONO_CAMERA_INTRINSICS"),
	STEREO_CAMERA_INTRINSICS  ENUMVALUE_STRING("STEREO_CAMERA_INTRINSICS"),
};

// Structures

/// Radial and tangential lens distortion coefficients computed during lens lens calibration
/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
struct STRUCT(Serialization::CodeGenModule("MikanVideoSourceTypes")) MikanDistortionCoefficients
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

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanDistortionCoefficients_GENERATED
	#endif
};

/// Camera intrinsic common properties
/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
struct STRUCT(Serialization::CodeGenModule("MikanVideoSourceTypes")) MikanCameraIntrinsics
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

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanCameraIntrinsics_GENERATED
	#endif
};

/// Camera intrinsic properties for a monoscopic camera
/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
struct STRUCT(Serialization::CodeGenModule("MikanVideoSourceTypes")) MikanMonoIntrinsics : public MikanCameraIntrinsics
{
	inline static const std::string k_typeName = "MikanMonoIntrinsics";

	FIELD()
	MikanDistortionCoefficients distortion_coefficients;   ///< Lens distortion coefficients
	FIELD()
	MikanMatrix3d camera_matrix;   ///< Intrinsic camera matrix containing focal lengths and principal point

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanMonoIntrinsics_GENERATED
	#endif
};

/// Camera intrinsic properties for a stereoscopic camera
/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
struct STRUCT(Serialization::CodeGenModule("MikanVideoSourceTypes")) MikanStereoIntrinsics : public MikanCameraIntrinsics
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

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanStereoIntrinsics_GENERATED
	#endif
};

/// Bundle containing all intrinsic video source properties
struct STRUCT(Serialization::CodeGenModule("MikanVideoSourceTypes")) MikanVideoSourceIntrinsics : public MikanResponse
{
	inline static const std::string k_typeName = "MikanVideoSourceIntrinsics";

	FIELD()
	Serialization::ObjectPtr<MikanCameraIntrinsics> intrinsics_ptr;

	FIELD()
	MikanIntrinsicsType intrinsics_type;

	MikanVideoSourceIntrinsics() 
		: MikanResponse(k_typeName) 
		, intrinsics_ptr()
		, intrinsics_type(INVALID_CAMERA_INTRINSICS)
	{}

	const MikanMonoIntrinsics& getMonoIntrinsics() const
	{
		assert(intrinsics_type == MONO_CAMERA_INTRINSICS);
		auto monoIntrinsicsPtr= 
			std::static_pointer_cast<MikanMonoIntrinsics>(
				intrinsics_ptr.getSharedPointer());
			
		return *monoIntrinsicsPtr.get();
	}

	const MikanStereoIntrinsics& getStereoIntrinsics() const
	{
		assert(intrinsics_type == STEREO_CAMERA_INTRINSICS);
		auto stereoIntrinsicsPtr =
			std::static_pointer_cast<MikanStereoIntrinsics>(
				intrinsics_ptr.getSharedPointer());

		return *stereoIntrinsicsPtr.get();
	}

	#if defined(MIKANAPI_REFLECTION_ENABLED) && defined(SERIALIZATION_REFLECTION_ENABLED)
	void setMonoIntrinsics(const MikanMonoIntrinsics& mono_intrinsics)
	{
		auto monoIntrinsics = std::make_shared<MikanMonoIntrinsics>(mono_intrinsics);

		intrinsics_ptr.setSharedPointer(monoIntrinsics);
		intrinsics_type = MONO_CAMERA_INTRINSICS;
	}

	void setStereoIntrinsics(const MikanStereoIntrinsics& stereo_intrinsics)
	{
		auto stereoIntrinsics = std::make_shared<MikanStereoIntrinsics>(stereo_intrinsics);

		intrinsics_ptr.setSharedPointer(stereoIntrinsics);
		intrinsics_type = STEREO_CAMERA_INTRINSICS;
	}
	#endif // MIKANAPI_REFLECTION_ENABLED && SERIALIZATION_REFLECTION_ENABLED

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceIntrinsics_GENERATED
	#endif // MIKANAPI_REFLECTION_ENABLED
};

/// Static properties 
struct STRUCT(Serialization::CodeGenModule("MikanVideoSourceTypes")) MikanVideoSourceAttachmentInfo : public MikanResponse
{
	inline static const std::string k_typeName = "MikanVideoSourceAttachmentInfo";

	FIELD()
	MikanVRDeviceID attached_vr_device_id;
	FIELD()
	MikanMatrix4f vr_device_offset_xform;

	MikanVideoSourceAttachmentInfo() : MikanResponse(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceAttachmentInfo_GENERATED
	#endif
};

/// Static properties about a video source
struct STRUCT(Serialization::CodeGenModule("MikanVideoSourceTypes")) MikanVideoSourceMode : public MikanResponse
{
	inline static const std::string k_typeName = "MikanVideoSourceMode";

	FIELD()
	MikanVideoSourceType video_source_type;
	FIELD()
	MikanVideoSourceApi video_source_api;
	FIELD()
	Serialization::String device_path;
	FIELD()
	Serialization::String video_mode_name;
	FIELD()
	int32_t resolution_x;
	FIELD()
	int32_t resolution_y;
	FIELD()
	float frame_rate;

	MikanVideoSourceMode() : MikanResponse(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceMode_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanVideoSourceTypes_GENERATED
#endif