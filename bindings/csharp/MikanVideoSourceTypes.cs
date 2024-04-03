using System.Collections;

namespace MikanXR
{
	// Constants
	public enum MikanVideoSourceType
	{
		MONO,
		STEREO
	};

	/// The list of possible camera APIs used by MikanXR
	public enum MikanVideoSourceApi
	{
		INVALID,
		OPENCV_CV,
		WINDOWS_MEDIA_FOUNDATION
	};

	public enum MikanIntrinsicsType
	{
		INVALID_CAMERA_INTRINSICS,
		MONO_CAMERA_INTRINSICS,
		STEREO_CAMERA_INTRINSICS,
	};

	// Structures

	/// Radial and tangential lens distortion coefficients computed during lens lens calibration
	/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
	public struct MikanDistortionCoefficients
	{
		public double k1 { get; set; } ///< Radial Distortion Parameter 1 (r^2 numerator constant)
		public double k2 { get; set; } ///< Radial Distortion Parameter 2 (r^4 numerator constant)
		public double k3 { get; set; } ///< Radial Distortion Parameter 3 (r^6 numerator constant)
		public double k4 { get; set; } ///< Radial Distortion Parameter 4 (r^2 divisor constant)
		public double k5 { get; set; } ///< Radial Distortion Parameter 5 (r^4 divisor constant)
		public double k6 { get; set; } ///< Radial Distortion Parameter 6 (r^6 divisor constant)
		public double p1 { get; set; } ///< Tangential Distortion Parameter 1
		public double p2 { get; set; } ///< Tangential Distortion Parameter 2
	};

	/// Camera intrinsic properties for a monoscopic camera
	/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
	public struct MikanMonoIntrinsics
	{
		public double pixel_width { get; set; }  ///< Width of the camera buffer in pixels
		public double pixel_height { get; set; } ///< Height of the camera buffer in pixels
		public double hfov { get; set; }         ///< The horizontal field of view camera in degrees
		public double vfov { get; set; }         ///< The vertical field of view camera in degrees
		public double znear { get; set; }        ///< The distance of the near clipping plane in cm
		public double zfar { get; set; }         ///< The distance of the far clipping plane in cm
		public MikanDistortionCoefficients distortion_coefficients { get; set; }   ///< Lens distortion coefficients
		public MikanMatrix3d camera_matrix { get; set; }   ///< Intrinsic camera matrix containing focal lengths and principal point
	};

	/// Camera intrinsic properties for a stereoscopic camera
	/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
	public struct MikanStereoIntrinsics
	{
		// Keep these in sync with PSVRMonoTrackerIntrinsics
		public double pixel_width { get; set; }  ///< Width of the camera buffer in pixels
		public double pixel_height { get; set; } ///< Height of the camera buffer in pixels
		public double hfov { get; set; }         ///< The horizontal field of view camera in degrees
		public double vfov { get; set; }         ///< The vertical field of view camera in degrees
		public double znear { get; set; }        ///< The distance of the near clipping plane in cm
		public double zfar { get; set; }         ///< The distance of the far clipping plane in cm
		public MikanDistortionCoefficients left_distortion_coefficients { get; set; } ///< Left lens distortion coefficients
		public MikanMatrix3d left_camera_matrix { get; set; } ///< Intrinsic matrix for left camera containing focal lengths and principal point

		public MikanDistortionCoefficients right_distortion_coefficients { get; set; } ///< Right lens distortion coefficients
		public MikanMatrix3d right_camera_matrix { get; set; } ///< Intrinsic matrix for rotation camera containing focal lengths and principal point
		public MikanMatrix3d left_rectification_rotation { get; set; } ///< Rotation applied to left camera to rectify the image
		public MikanMatrix3d right_rectification_rotation { get; set; } ///< Rotation applied to right camera to rectify the image
		public MikanMatrix4x3d left_rectification_projection { get; set; } ///< Projection applied to left camera to rectify the image
		public MikanMatrix4x3d right_rectification_projection { get; set; } ///< Projection applied to right camera to rectify the image
		public MikanMatrix3d rotation_between_cameras { get; set; } ///< Rotation between the left and right cameras
		public MikanVector3d translation_between_cameras { get; set; } ///< Translation between the left and right camera
		public MikanMatrix3d essential_matrix { get; set; } ///< Transform relating points in unit coordinate space between cameras
		public MikanMatrix3d fundamental_matrix { get; set; } ///< Transform relating points in pixel coordinates between cameras
		public MikanMatrix4d reprojection_matrix { get; set; }  ///< Transform relating pixel x,y + disparity to distance from cameras
	};

	/// Bundle containing all intrinsic video source properties
	public struct MikanVideoSourceIntrinsics : public MikanResponse
	{
		public MikanMonoIntrinsics mono { get; set; }
		public MikanStereoIntrinsics stereo { get; set; }
		public MikanIntrinsicsType intrinsics_type { get; set; }

		public MikanVideoSourceIntrinsics() : MikanResponse(typeof(MikanVideoSourceIntrinsics).Name) {}
	};

	/// Static properties 
	public struct MikanVideoSourceAttachmentInfo : public MikanResponse
	{
		public MikanVRDeviceID attached_vr_device_id { get; set; }
		public MikanMatrix4f vr_device_offset_xform { get; set; }

		MikanVideoSourceAttachmentInfo() : MikanResponse(typeof(MikanVideoSourceAttachmentInfo).Name) {}
	};

	/// Static properties about a video source
	public struct MikanVideoSourceMode : public MikanResponse
	{
		public MikanVideoSourceType video_source_type { get; set; }
		public MikanVideoSourceApi video_source_api { get; set; }
		public string device_path { get; set; }
		public string video_mode_name { get; set; }
		public int resolution_x { get; set; }
		public int resolution_y { get; set; }
		public float frame_rate { get; set; }

		MikanVideoSourceMode() : MikanResponse(typeof(MikanVideoSourceMode).Name) {}
	};	
}