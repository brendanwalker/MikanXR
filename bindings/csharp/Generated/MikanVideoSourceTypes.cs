// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
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

	public class MikanDistortionCoefficients
	{
		public double k1;
		public double k2;
		public double k3;
		public double k4;
		public double k5;
		public double k6;
		public double p1;
		public double p2;
	};

	public class MikanCameraIntrinsics
	{
		public double pixel_width;
		public double pixel_height;
		public double hfov;
		public double vfov;
		public double znear;
		public double zfar;
	};

	public class MikanMonoIntrinsics : MikanCameraIntrinsics
	{
		public MikanDistortionCoefficients distortion_coefficients;
		public MikanMatrix3d camera_matrix;
	};

	public class MikanStereoIntrinsics : MikanCameraIntrinsics
	{
		public MikanDistortionCoefficients left_distortion_coefficients;
		public MikanMatrix3d left_camera_matrix;
		public MikanDistortionCoefficients right_distortion_coefficients;
		public MikanMatrix3d right_camera_matrix;
		public MikanMatrix3d left_rectification_rotation;
		public MikanMatrix3d right_rectification_rotation;
		public MikanMatrix4x3d left_rectification_projection;
		public MikanMatrix4x3d right_rectification_projection;
		public MikanMatrix3d rotation_between_cameras;
		public MikanVector3d translation_between_cameras;
		public MikanMatrix3d essential_matrix;
		public MikanMatrix3d fundamental_matrix;
		public MikanMatrix4d reprojection_matrix;
	};

	public class MikanVideoSourceIntrinsics
	{
		public MikanCameraIntrinsics intrinsics_ptr;
		public MikanIntrinsicsType intrinsics_type;
	};

}