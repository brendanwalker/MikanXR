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
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVideoSourceTypes")) MikanDistortionCoefficients
{
	FIELD()
	double k1 = 0.0; ///< Radial Distortion Parameter 1 (r^2 numerator constant)
	FIELD()
	double k2 = 0.0; ///< Radial Distortion Parameter 2 (r^4 numerator constant)
	FIELD()
	double k3 = 0.0; ///< Radial Distortion Parameter 3 (r^6 numerator constant)
	FIELD()
	double k4 = 0.0; ///< Radial Distortion Parameter 4 (r^2 divisor constant)
	FIELD()
	double k5 = 0.0; ///< Radial Distortion Parameter 5 (r^4 divisor constant)
	FIELD()
	double k6 = 0.0; ///< Radial Distortion Parameter 6 (r^6 divisor constant)
	FIELD()
	double p1 = 0.0; ///< Tangential Distortion Parameter 1
	FIELD()
	double p2 = 0.0; ///< Tangential Distortion Parameter 2

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanDistortionCoefficients_GENERATED
	#endif
};

/// Camera intrinsic common properties
/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVideoSourceTypes")) MikanCameraIntrinsics
	: public Serialization::PolymorphicStruct
{
	FIELD()
	double pixel_width = 0.0;  ///< Width of the camera buffer in pixels
	FIELD()
	double pixel_height = 0.0; ///< Height of the camera buffer in pixels
	FIELD()
	double aspect_ratio = 0.0; ///< The aspect ratio of each pixel (y focal length / x focal length)
	FIELD()
	double hfov = 0.0;         ///< The horizontal field of view camera in degrees
	FIELD()
	double vfov = 0.0;         ///< The vertical field of view camera in degrees
	FIELD()
	double znear = 0.0;        ///< The distance of the near clipping plane in meters
	FIELD()
	double zfar = 0.0;         ///< The distance of the far clipping plane in meters

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanCameraIntrinsics_GENERATED
	#endif
};

/// Camera intrinsic properties for a monoscopic camera
/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVideoSourceTypes")) MikanMonoIntrinsics 
	: public MikanCameraIntrinsics
{
	// Distortion coefficients computed for the physical camera lens
	FIELD()
	MikanDistortionCoefficients distortion_coefficients;
	// Intrinsic camera matrix containing focal lengths and principal point for the raw distorted image
	FIELD()
	MikanMatrix3d distorted_camera_matrix;
	// Intrinsic camera matrix containing focal lengths and principal point for the undistorted image
	// NOTE: The hfov and vfov are computed from this matrix
	FIELD()
	MikanMatrix3d undistorted_camera_matrix;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanMonoIntrinsics_GENERATED
	#endif
};

/// Camera intrinsic properties for a stereoscopic camera
/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVideoSourceTypes")) MikanStereoIntrinsics 
	: public MikanCameraIntrinsics
{
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
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanVideoSourceTypes")) MikanVideoSourceIntrinsics
{
	// MikanCameraIntrinsics derived type
	FIELD()
	Serialization::PolymorphicObjectPtr intrinsics_ptr;

	FIELD()
	MikanIntrinsicsType intrinsics_type;

	MikanVideoSourceIntrinsics() 
		: intrinsics_ptr()
		, intrinsics_type(INVALID_CAMERA_INTRINSICS)
	{}

	const MikanMonoIntrinsics& getMonoIntrinsics() const
	{
		assert(intrinsics_type == MONO_CAMERA_INTRINSICS);
		auto* monoIntrinsicsPtr= intrinsics_ptr.getTypedPointer<MikanMonoIntrinsics>();
			
		return *monoIntrinsicsPtr;
	}

	MikanMonoIntrinsics& getMonoIntrinsicsMutable()
	{
		return const_cast<MikanMonoIntrinsics&>(getMonoIntrinsics());
	}

	const MikanStereoIntrinsics& getStereoIntrinsics() const
	{
		assert(intrinsics_type == STEREO_CAMERA_INTRINSICS);
		auto stereoIntrinsicsPtr = intrinsics_ptr.getTypedPointer<MikanStereoIntrinsics>();

		return *stereoIntrinsicsPtr;
	}

	MikanStereoIntrinsics& getStereoIntrinsicsMutable()
	{
		return const_cast<MikanStereoIntrinsics&>(getStereoIntrinsics());
	}

	#if defined(MIKANAPI_REFLECTION_ENABLED) && defined(SERIALIZATION_REFLECTION_ENABLED)
	MikanMonoIntrinsics& makeMonoIntrinsics()
	{
		auto* monoIntrinsics= intrinsics_ptr.allocatedByType<MikanMonoIntrinsics>();
		intrinsics_type = MONO_CAMERA_INTRINSICS;

		return *monoIntrinsics;
	}

	MikanStereoIntrinsics& makeStereoIntrinsics()
	{
		auto* stereoIntrinsics= intrinsics_ptr.allocatedByType<MikanStereoIntrinsics>();
		intrinsics_type = STEREO_CAMERA_INTRINSICS;

		return *stereoIntrinsics;
	}
	#endif // MIKANAPI_REFLECTION_ENABLED && SERIALIZATION_REFLECTION_ENABLED

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVideoSourceIntrinsics_GENERATED
	#endif // MIKANAPI_REFLECTION_ENABLED
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanVideoSourceTypes_GENERATED
#endif