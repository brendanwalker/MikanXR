// This file is auto generated. DO NO EDIT.
using System;
using System.Collections.Generic;

namespace MikanXR
{
	public enum MikanIntrinsicsType
	{
		INVALID= 0,
		MONO_CAMERA_INTRINSICS= 1,
		STEREO_CAMERA_INTRINSICS= 2,
	};

	public enum MikanVideoSourceType
	{
		MONO= 0,
		STEREO= 1,
	};

	public class MikanBaseIntrinsics : PolymorphicStruct
	{
		public static new readonly long classId= -3286470658648308984;

		public double pixel_width;
		public double pixel_height;
		public double aspect_ratio;
		public double hfov;
		public double vfov;
		public double znear;
		public double zfar;
	};

	public class MikanDistortionCoefficients
	{
		public static readonly long classId= -2596555002374434624;

		public double k1;
		public double k2;
		public double k3;
		public double k4;
		public double k5;
		public double k6;
		public double p1;
		public double p2;
	};

	public class MikanMonoIntrinsics : MikanBaseIntrinsics
	{
		public static new readonly long classId= 4896055255137140914;

		public MikanDistortionCoefficients distortion_coefficients;
		public MikanMatrix3d distorted_camera_matrix;
		public MikanMatrix3d undistorted_camera_matrix;
	};

	public class MikanNetworkVideoSourceInfo
	{
		public static readonly long classId= 446555362386089173;

		public string network_source_name;
		public string url;
		public MikanVideoSourceIntrinsics intrinsics;
	};

	public class MikanStereoIntrinsics : MikanBaseIntrinsics
	{
		public static new readonly long classId= -261934067861644075;

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

	public class MikanUSBVideoSourceInfo
	{
		public static readonly long classId= -5452510762721765943;

		public string usb_source_name;
		public string device_path;
		public string video_mode;
		public MikanVideoSourceIntrinsics intrinsics;
	};

	public class MikanVariantBase : PolymorphicStruct
	{
		public static new readonly long classId= 5706978007370628991;

	};

	public class MikanVideoSourceIntrinsics
	{
		public static readonly long classId= -5073913459979558727;

		public PolymorphicObject intrinsics_ptr;
		public MikanIntrinsicsType intrinsics_type;
	};

}
