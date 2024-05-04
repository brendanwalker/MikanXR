#ifndef __MIKAN_CORE_TYPES_H
#define __MIKAN_CORE_TYPES_H

#include "MikanExport.h"
#include <stdint.h>
#include <stdbool.h>

//cut_before

#define INVALID_MIKAN_ID				-1

/// Result enum in response to a client API request
enum MikanResult
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

enum MikanLogLevel
{
	MikanLogLevel_Trace,
	MikanLogLevel_Debug,
	MikanLogLevel_Info,
	MikanLogLevel_Warning,
	MikanLogLevel_Error,
	MikanLogLevel_Fatal
};

enum MikanClientGraphicsApi
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

enum MikanColorBufferType
{
	MikanColorBuffer_NOCOLOR,
	MikanColorBuffer_RGB24,
	MikanColorBuffer_RGBA32, // DXGI_FORMAT_R8G8B8A8_UNORM / DXGI_FORMAT_R8G8B8A8_TYPELESS
	MikanColorBuffer_BGRA32, // DXGI_FORMAT_B8G8R8A8_UNORM / DXGI_FORMAT_B8G8R8A8_TYPELESS
};

enum MikanDepthBufferType
{
	MikanDepthBuffer_NODEPTH,
	MikanDepthBuffer_FLOAT_DEPTH,
	MikanDepthBuffer_PACK_DEPTH_RGBA,
};

struct MikanRenderTargetDescriptor
{
	MikanColorBufferType color_buffer_type;
	MikanDepthBufferType depth_buffer_type;
	uint32_t width;
	uint32_t height;
	MikanClientGraphicsApi graphicsAPI;
};

struct MikanClientFrameRendered
{
	uint64_t frame_index;
	float zNear;
	float zFar;
};

// Wrapper Types
//--------------
/// The ID of a pending request send to Mikan
typedef int MikanRequestID;

/// Registered response callback function for a Mikan request
typedef void(MIKAN_CALLBACK* MikanResponseCallback)(
	MikanRequestID request_id, const char* utf8_response_string, void* userdata);

typedef void (MIKAN_CALLBACK* MikanLogCallback)(
	int /*log_level*/, const char* /*log_message*/);

/**
@}
*/

//cut_after
#endif