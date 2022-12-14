/**
\file
*/

#ifndef __MIKAN_CLIENT_TYPES_H
#define __MIKAN_CLIENT_TYPES_H
#include "MikanMathTypes.h"
#include <stdint.h>
//cut_before

/**
\brief Client Interface for MikanXR
\defgroup MikanClient_CAPI Client Interface
\addtogroup MikanClient_CAPI
@{
*/

// Wrapper Types
//--------------

/// The ID of a VR Device
typedef int32_t MikanVRDeviceID;

/// The ID of a stencil
typedef int32_t MikanStencilID;

/// The ID of a spatial anchor
typedef int32_t MikanSpatialAnchorID;

#define INVALID_MIKAN_ID			-1
#define MAX_MIKAN_VR_DEVICES		64
#define MAX_MIKAN_STENCILS			16
#define MAX_MIKAN_SPATIAL_ANCHORS	64
#define MAX_MIKAN_ANCHOR_NAME_LEN	128

// Shared Constants
//-----------------

/// Result enum in response to a client API request
typedef enum
{
	MikanResult_Success = 0,	///< General Success Result	
	MikanResult_GeneralError = -1,	///< General Error Result
	MikanResult_Uninitialized = -2,
	MikanResult_NullParam = -2,
	MikanResult_InitFailed = -3,
	MikanResult_ConnectionFailed = -4,
	MikanResult_AlreadyConnected = -5,
	MikanResult_NoData = -6,
	MikanResult_NotConnected = -7,
	MikanResult_SharedMemoryError = -8,
	MikanResult_UnknownClient = -9,
	MikanResult_UnknownFunction = -10,
	MikanResult_FailedFunctionSend= -11,
	MikanResult_FunctionResponseTimeout= -12,
	MikanResult_MalformedParameters = -13,
	MikanResult_MalformedResponse = -14,
	MikanResult_NoSelectedCamera = -15,
	MikanResult_NoCameraAssignedTracker = -16,
	MikanResult_InvalidDeviceID = -17,
	MikanResult_InvalidStencilID = -18,
	MikanResult_TooManyStencils = -19,
	MikanResult_InvalidAPI = -20,
	MikanResult_SharedTextureError = -21,
} MikanResult;

typedef enum
{
	MikanLogLevel_Trace,
	MikanLogLevel_Debug,
	MikanLogLevel_Info,
	MikanLogLevel_Warning,
	MikanLogLevel_Error,
	MikanLogLevel_Fatal
} MikanLogLevel;

typedef void (*MikanLogCallback)(int /*log_level*/, const char* /*log_message*/);

typedef enum
{
	MikanVideoSourceType_MONO,
	MikanVideoSourceType_STEREO
} MikanVideoSourceType;

/// The list of possible camera APIs used by MikanXR
typedef enum
{
	MikanVideoSourceApi_INVALID,
	MikanVideoSourceApi_OPENCV_CV,
	MikanVideoSourceApi_WINDOWS_MEDIA_FOUNDATION
} MikanVideoSourceApi;

/// The list of possible vr device drivers used by MikanXR Client API
typedef enum
{
	MikanVRDeviceApi_INVALID,
	MikanVRDeviceApi_STEAM_VR,
} MikanVRDeviceApi;

/// The list of possible vr device types used by MikanXR Client API
typedef enum
{
	MikanVRDeviceType_INVALID,
	MikanVRDeviceType_HMD,
	MikanVRDeviceType_CONTROLLER,
	MikanVRDeviceType_TRACKER
} MikanVRDeviceType;

typedef enum
{
	MikanRenderTarget_COLOR,
	MikanRenderTarget_DEPTH,
} MikanBufferType;

typedef enum
{
	MikanColorBuffer_NONE,
	MikanColorBuffer_RGB24,
	MikanColorBuffer_RGBA32,
} MikanColorBufferType;

typedef enum
{
	MikanDepthBuffer_NONE,
	MikanDepthBuffer_DEPTH16,
	MikanDepthBuffer_DEPTH32,
} MikanDepthBufferType;

typedef enum
{
	MikanFeature_NONE = 0L,

	// Render target options
	MikanFeature_RenderTarget_RGB24 = 1L << 0,
	MikanFeature_RenderTarget_RGBA32 = 1L << 1,
	MikanFeature_RenderTarget_DEPTH16 = 1L << 2,
	MikanFeature_RenderTarget_DEPTH32 = 1L << 3,
} MikanClientFeatures;

typedef enum
{
	MikanClientGraphicsAPI_UNKNOWN,

	MikanClientGraphicsAPI_Direct3D9,
	MikanClientGraphicsAPI_Direct3D11,
	MikanClientGraphicsAPI_Direct3D12,
	MikanClientGraphicsAPI_OpenGL,

	MikanClientGraphicsAPI_COUNT,
} MikanClientGraphicsAPI;

typedef struct
{
	uint64_t supportedFeatures;
	char engineName[64];
	char engineVersion[32];
	char applicationName[128];
	char applicationVersion[32];
	char xrDeviceName[32];
	char mikanSdkVersion[32];
	MikanClientGraphicsAPI graphicsAPI;
} MikanClientInfo;

/// A float RGB color with [0,1] components.
typedef struct
{
	float r, g, b;
} MikanColorRGB;

typedef struct
{
	MikanColorBufferType color_buffer_type;
	MikanColorRGB color_key;
	MikanDepthBufferType depth_buffer_type;
	uint32_t width;
	uint32_t height;
	MikanClientGraphicsAPI graphicsAPI;
} MikanRenderTargetDescriptor;

typedef struct
{
	uint8_t* color_buffer;
	uint8_t* depth_buffer;
	size_t color_buffer_size;
	size_t depth_buffer_size;
	uint32_t width;
	uint32_t height;
} MikanRenderTargetMemory;

typedef struct  
{
	MikanStencilID stencil_id; // filled in on allocation
	MikanSpatialAnchorID parent_anchor_id; // if invalid, stencil is in world space
	MikanVector3f quad_center;
	MikanVector3f quad_x_axis;
	MikanVector3f quad_y_axis;
	MikanVector3f quad_normal;
	float quad_width;
	float quad_height;
	bool is_double_sided;
	bool is_disabled;
} MikanStencilQuad;

typedef struct
{
	MikanStencilID stencil_id; // filled in on allocation
	MikanSpatialAnchorID parent_anchor_id; // if invalid, stencil is in world space
	MikanVector3f box_center;
	MikanVector3f box_x_axis;
	MikanVector3f box_y_axis;
	MikanVector3f box_z_axis;
	float box_x_size;
	float box_y_size;
	float box_z_size;
	bool is_disabled;
} MikanStencilBox;

typedef struct
{
	MikanStencilID stencil_id; // filled in on allocation
	MikanSpatialAnchorID parent_anchor_id; // if invalid, stencil is in world space
	MikanVector3f model_position;
	MikanRotator3f model_rotator;
	MikanVector3f model_scale;
	bool is_disabled;
} MikanStencilModel;

typedef enum
{
	MikanStencilType_NONE,
	MikanStencilType_QUAD,
	MikanStencilType_MODEL,
} MikanStencilType;

typedef struct
{
	MikanStencilID stencil_id_list[MAX_MIKAN_STENCILS];
	int32_t stencil_count;
} MikanStencilList;


// Tracker State
//--------------

/// Radial and tangential lens distortion coefficients computed during lens lens calibration
/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
typedef struct
{
	double k1; ///< Radial Distortion Parameter 1 (r^2 numerator constant)
	double k2; ///< Radial Distortion Parameter 2 (r^4 numerator constant)
	double k3; ///< Radial Distortion Parameter 3 (r^6 numerator constant)
	double k4; ///< Radial Distortion Parameter 4 (r^2 divisor constant)
	double k5; ///< Radial Distortion Parameter 5 (r^4 divisor constant)
	double k6; ///< Radial Distortion Parameter 6 (r^6 divisor constant)
	double p1; ///< Tangential Distortion Parameter 1
	double p2; ///< Tangential Distortion Parameter 2
} MikanDistortionCoefficients;

/// Camera intrinsic properties for a monoscopic camera
/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
typedef struct
{
	double pixel_width;  ///< Width of the camera buffer in pixels
	double pixel_height; ///< Height of the camera buffer in pixels
	double hfov;         ///< The horizontal field of view camera in degrees
	double vfov;         ///< The vertical field of view camera in degrees
	double znear;        ///< The distance of the near clipping plane in cm
	double zfar;         ///< The distance of the far clipping plane in cm
	MikanDistortionCoefficients distortion_coefficients;   ///< Lens distortion coefficients
	MikanMatrix3d camera_matrix;   ///< Intrinsic camera matrix containing focal lengths and principal point
} MikanMonoIntrinsics;

/// Camera intrinsic properties for a stereoscopic camera
/// See the [OpenCV Docs](http://docs.opencv.org/2.4/modules/calib3d/doc/camera_calibration_and_3d_reconstruction.html) for details
typedef struct
{
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
} MikanStereoIntrinsics;

typedef enum
{
	INVALID_CAMERA_INTRINSICS,
	MONO_CAMERA_INTRINSICS,
	STEREO_CAMERA_INTRINSICS,
} MikanIntrinsicsType;

/// Bundle containing all intrinsic video source properties
typedef struct
{
	union {
		MikanMonoIntrinsics mono;
		MikanStereoIntrinsics stereo;
	} intrinsics;

	MikanIntrinsicsType intrinsics_type;
} MikanVideoSourceIntrinsics;

/// Static properties 
typedef struct
{
	MikanSpatialAnchorID parent_anchor_id;
	MikanVRDeviceID attached_vr_device_id;
	MikanMatrix4f vr_device_offset_xform;
	float camera_scale;
} MikanVideoSourceAttachmentInfo;

/// Static properties about a video source
typedef struct
{
	MikanVideoSourceType video_source_type;
	MikanVideoSourceApi video_source_api;
	char device_path[128];
	char video_mode_name[64];
	int32_t resolution_x;
	int32_t resolution_y;
	float frame_rate;
} MikanVideoSourceMode;

typedef struct
{
	MikanVRDeviceID vr_device_id_list[MAX_MIKAN_VR_DEVICES];
	int32_t vr_device_count;
} MikanVRDeviceList;

typedef struct
{
	MikanVRDeviceApi vr_device_api;
	MikanVRDeviceType vr_device_type;
	char device_path[128];
} MikanVRDeviceInfo;

typedef struct
{
	MikanSpatialAnchorID spatial_anchor_id_list[MAX_MIKAN_SPATIAL_ANCHORS];
	int32_t spatial_anchor_count;
} MikanSpatialAnchorList;

typedef struct
{
	MikanSpatialAnchorID anchor_id;
	MikanMatrix4f anchor_xform;
	char anchor_name[MAX_MIKAN_ANCHOR_NAME_LEN];
} MikanSpatialAnchorInfo;

// Message Container
//------------------
typedef enum
{
	// App Connection Events
	MikanEvent_connected,
	MikanEvent_disconnected,

	// Video Source Events
	MikanEvent_videoSourceOpened,
	MikanEvent_videoSourceClosed,
	MikanEvent_videoSourceNewFrame,
	MikanEvent_videoSourceAttachmentChanged,
	MikanEvent_videoSourceIntrinsicsChanged,
	MikanEvent_videoSourceModeChanged,

	// VR Device Events
	MikanEvent_vrDevicePoseUpdated,
	MikanEvent_vrDeviceListUpdated,

	// Spatial Anchor Events
	MikanEvent_anchorPoseUpdated,
	MikanEvent_anchorListUpdated
} MikanEventType;

typedef struct
{
	MikanVector3f cameraForward;
	MikanVector3f cameraUp;
	MikanVector3f cameraPosition;
	uint64_t frame;
} MikanVideoSourceNewFrameEvent;

typedef struct
{
	MikanMatrix4f transform;
	MikanVRDeviceID device_id;
	uint64_t frame;
} MikanVRDevicePoseUpdateEvent;

typedef struct
{
	MikanMatrix4f transform;
	MikanSpatialAnchorID anchor_id;
} MikanAnchorPoseUpdateEvent;

/// A container for all MikanXR Client API events
typedef struct
{
	union {
		MikanVideoSourceNewFrameEvent video_source_new_frame;
		MikanVRDevicePoseUpdateEvent vr_device_pose_updated;
		MikanAnchorPoseUpdateEvent anchor_pose_updated;
	} event_payload;
	MikanEventType event_type;
} MikanEvent;


/**
@}
*/

//cut_after
#endif
