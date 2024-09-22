#ifndef __MIKAN_CORE_TYPES_H
#define __MIKAN_CORE_TYPES_H

#include "MikanExport.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef ENABLE_REFLECTION
#include "MikanCoreTypes.rfkh.h"
#endif // ENABLE_REFLECTION

//cut_before

#define INVALID_MIKAN_ID				-1

/// Result enum in response to a client API request
enum ENUM() MikanResult
{
	MikanResult_Success,	///< General Success Result	
	MikanResult_GeneralError,	///< General Error Result
	MikanResult_Uninitialized,
	MikanResult_NullParam,
	MikanResult_BufferTooSmall,
	MikanResult_InitFailed,
	MikanResult_ConnectionFailed,
	MikanResult_AlreadyConnected,
	MikanResult_NotConnected,
	MikanResult_SocketError,
	MikanResult_NoData,
	MikanResult_Timeout,
	MikanResult_Canceled,
	MikanResult_UnknownClient,
	MikanResult_UnknownFunction,
	MikanResult_FailedFunctionSend,
	MikanResult_FunctionResponseTimeout,
	MikanResult_MalformedParameters,
	MikanResult_MalformedResponse,
	MikanResult_InvalidAPI,
	MikanResult_SharedTextureError,
	MikanResult_NoVideoSource,
	MikanResult_NoVideoSourceAssignedTracker,
	MikanResult_InvalidDeviceId,
	MikanResult_InvalidStencilID,
	MikanResult_InvalidAnchorID,
};

enum ENUM() MikanLogLevel
{
	MikanLogLevel_Trace,
	MikanLogLevel_Debug,
	MikanLogLevel_Info,
	MikanLogLevel_Warning,
	MikanLogLevel_Error,
	MikanLogLevel_Fatal
};

enum ENUM() MikanClientGraphicsApi
{
	MikanClientGraphicsApi_UNKNOWN= -1,

	MikanClientGraphicsApi_Direct3D9,
	MikanClientGraphicsApi_Direct3D11,
	MikanClientGraphicsApi_Direct3D12,
	MikanClientGraphicsApi_OpenGL,
	MikanClientGraphicsApi_Metal,
	MikanClientGraphicsApi_Vulkan,

	MikanClientGraphicsApi_COUNT,
};

enum ENUM() MikanColorBufferType
{
	MikanColorBuffer_NOCOLOR,
	MikanColorBuffer_RGB24,
	MikanColorBuffer_RGBA32, // DXGI_FORMAT_R8G8B8A8_UNORM / DXGI_FORMAT_R8G8B8A8_TYPELESS
	MikanColorBuffer_BGRA32, // DXGI_FORMAT_B8G8R8A8_UNORM / DXGI_FORMAT_B8G8R8A8_TYPELESS
};

enum ENUM() MikanDepthBufferType
{
	MikanDepthBuffer_NODEPTH,
	MikanDepthBuffer_FLOAT_DEVICE_DEPTH, // Raw float non-linear depth values from the z-buffer (in source world units)
	MikanDepthBuffer_FLOAT_SCENE_DEPTH, // Linearized float distance-from-camera values (in source world units)
	MikanDepthBuffer_PACK_DEPTH_RGBA, // DXGI_FORMAT_R8G8B8A8_UNORM / DXGI_FORMAT_R8G8B8A8_TYPELESS
};

struct STRUCT() MikanRenderTargetDescriptor
{
	MikanColorBufferType color_buffer_type;
	MikanDepthBufferType depth_buffer_type;
	uint32_t width;
	uint32_t height;
	MikanClientGraphicsApi graphicsAPI;

#ifdef ENABLE_REFLECTION
	MikanRenderTargetDescriptor_GENERATED
#endif // ENABLE_REFLECTION
};

struct STRUCT() MikanClientFrameRendered
{
	uint64_t frame_index;

#ifdef ENABLE_REFLECTION
	MikanClientFrameRendered_GENERATED
#endif // ENABLE_REFLECTION
};

// Wrapper Types
//--------------
/// The ID of a pending request send to Mikan
typedef int MikanRequestID;

/// Registered text response callback function for a Mikan request
typedef void(MIKAN_CALLBACK* MikanTextResponseCallback)(
	MikanRequestID request_id, const char* utf8_response_string, void* userdata);

/// Registered binary response callback function for a Mikan request
typedef void(MIKAN_CALLBACK* MikanBinaryResponseCallback)(
	const uint8_t* buffer, size_t buffer_size, void* userdata);


typedef void (MIKAN_CALLBACK* MikanLogCallback)(
	int /*log_level*/, const char* /*log_message*/);


#ifdef ENABLE_REFLECTION
File_MikanCoreTypes_GENERATED
#endif // ENABLE_REFLECTION

/**
@}
*/

//cut_after
#endif