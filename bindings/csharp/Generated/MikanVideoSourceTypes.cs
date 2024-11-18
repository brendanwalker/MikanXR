// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	// Enums
	public enum MikanVideoSourceType
	{
		MONO= 0,
		STEREO= 1,
	};

	public enum MikanVideoSourceApi
	{
		INVALID= 0,
		OPEN_CV= 1,
		WINDOWS_MEDIA_FOUNDATION= 2,
	};

	public enum MikanIntrinsicsType
	{
		INVALID= 0,
		MONO_CAMERA_INTRINSICS= 1,
		STEREO_CAMERA_INTRINSICS= 2,
	};

	// Structs
	public struct MikanDistortionCoefficients
	{
		public double k1 { get; set; }
		public double k2 { get; set; }
		public double k3 { get; set; }
		public double k4 { get; set; }
		public double k5 { get; set; }
		public double k6 { get; set; }
		public double p1 { get; set; }
		public double p2 { get; set; }
	};

	public struct MikanCameraIntrinsics
	{
		public double pixel_width { get; set; }
		public double pixel_height { get; set; }
		public double hfov { get; set; }
		public double vfov { get; set; }
		public double znear { get; set; }
		public double zfar { get; set; }
	};

	public struct MikanMonoIntrinsics : MikanCameraIntrinsics
	{
		public MikanDistortionCoefficients distortion_coefficients { get; set; }
		public MikanMatrix3d camera_matrix { get; set; }
	};

	public struct MikanStereoIntrinsics : MikanCameraIntrinsics
	{
		public MikanDistortionCoefficients left_distortion_coefficients { get; set; }
		public MikanMatrix3d left_camera_matrix { get; set; }
		public MikanDistortionCoefficients right_distortion_coefficients { get; set; }
		public MikanMatrix3d right_camera_matrix { get; set; }
		public MikanMatrix3d left_rectification_rotation { get; set; }
		public MikanMatrix3d right_rectification_rotation { get; set; }
		public MikanMatrix4x3d left_rectification_projection { get; set; }
		public MikanMatrix4x3d right_rectification_projection { get; set; }
		public MikanMatrix3d rotation_between_cameras { get; set; }
		public MikanVector3d translation_between_cameras { get; set; }
		public MikanMatrix3d essential_matrix { get; set; }
		public MikanMatrix3d fundamental_matrix { get; set; }
		public MikanMatrix4d reprojection_matrix { get; set; }
	};

	public struct MikanVideoSourceIntrinsics
	{
		public MikanCameraIntrinsics intrinsics_ptr { get; set; }
		public MikanIntrinsicsType intrinsics_type { get; set; }
	};

}
