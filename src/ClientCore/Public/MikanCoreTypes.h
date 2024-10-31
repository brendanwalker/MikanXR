#pragma once

#include "MikanCoreExport.h"
#include "SerializationProperty.h"

#include <stdint.h>
#include <stdbool.h>

#ifdef MIKANCORE_REFLECTION_ENABLED
#include "MikanCoreTypes.rfkh.h"
#endif // MIKANAPI_REFLECTION_ENABLED

#define INVALID_MIKAN_ID				-1

/// Result enum in response to a client API request
enum ENUM(Serialization::CodeGenModule("MikanCoreConstants")) MikanResult
{
	MikanResult_Success ENUMVALUE_STRING("Success"),	///< General Success Result	
	MikanResult_GeneralError ENUMVALUE_STRING("GeneralError"),	///< General Error Result
	MikanResult_Uninitialized ENUMVALUE_STRING("Uninitialized"),
	MikanResult_NullParam ENUMVALUE_STRING("NullParam"),
	MikanResult_BufferTooSmall ENUMVALUE_STRING("BufferTooSmall"),
	MikanResult_InitFailed ENUMVALUE_STRING("InitFailed"),
	MikanResult_ConnectionFailed ENUMVALUE_STRING("ConnectionFailed"),
	MikanResult_AlreadyConnected ENUMVALUE_STRING("AlreadyConnected"),
	MikanResult_NotConnected ENUMVALUE_STRING("NotConnected"),
	MikanResult_SocketError ENUMVALUE_STRING("SocketError"),
	MikanResult_NoData ENUMVALUE_STRING("NoData"),
	MikanResult_Timeout ENUMVALUE_STRING("Timeout"),
	MikanResult_Canceled ENUMVALUE_STRING("Canceled"),
	MikanResult_UnknownClient ENUMVALUE_STRING("UnknownClient"),
	MikanResult_UnknownFunction ENUMVALUE_STRING("UnknownFunction"),
	MikanResult_FailedFunctionSend ENUMVALUE_STRING("FailedFunctionSend"),
	MikanResult_FunctionResponseTimeout ENUMVALUE_STRING("FunctionResponseTimeout"),
	MikanResult_MalformedParameters ENUMVALUE_STRING("MalformedParameters"),
	MikanResult_MalformedResponse ENUMVALUE_STRING("MalformedResponse"),
	MikanResult_InvalidAPI ENUMVALUE_STRING("InvalidAPI"),
	MikanResult_SharedTextureError ENUMVALUE_STRING("SharedTextureError"),
	MikanResult_NoVideoSource ENUMVALUE_STRING("NoVideoSource"),
	MikanResult_NoVideoSourceAssignedTracker ENUMVALUE_STRING("NoVideoSourceAssignedTracker"),
	MikanResult_InvalidDeviceId ENUMVALUE_STRING("InvalidDeviceId"),
	MikanResult_InvalidStencilID ENUMVALUE_STRING("InvalidStencilID"),
	MikanResult_InvalidAnchorID ENUMVALUE_STRING("InvalidAnchorID"),
};

enum ENUM(Serialization::CodeGenModule("MikanCoreConstants")) MikanLogLevel
{
	MikanLogLevel_Trace ENUMVALUE_STRING("Trace"),
	MikanLogLevel_Debug ENUMVALUE_STRING("Debug"),
	MikanLogLevel_Info ENUMVALUE_STRING("Info"),
	MikanLogLevel_Warning ENUMVALUE_STRING("Warning"),
	MikanLogLevel_Error ENUMVALUE_STRING("Error"),
	MikanLogLevel_Fatal ENUMVALUE_STRING("Fatal")
};

enum ENUM(Serialization::CodeGenModule("MikanCoreConstants")) MikanClientGraphicsApi
{
	MikanClientGraphicsApi_UNKNOWN ENUMVALUE_STRING("UNKNOWN") = -1,

	MikanClientGraphicsApi_Direct3D9 ENUMVALUE_STRING("Direct3D9"),
	MikanClientGraphicsApi_Direct3D11 ENUMVALUE_STRING("Direct3D11"),
	MikanClientGraphicsApi_Direct3D12 ENUMVALUE_STRING("Direct3D12"),
	MikanClientGraphicsApi_OpenGL ENUMVALUE_STRING("OpenGL"),
	MikanClientGraphicsApi_Metal ENUMVALUE_STRING("Metal"),
	MikanClientGraphicsApi_Vulkan ENUMVALUE_STRING("Vulkan"),

	MikanClientGraphicsApi_COUNT,
};

enum ENUM(Serialization::CodeGenModule("MikanCoreConstants")) MikanColorBufferType
{
	MikanColorBuffer_NOCOLOR ENUMVALUE_STRING("NOCOLOR"),
	MikanColorBuffer_RGB24 ENUMVALUE_STRING("RGB24"),
	// DXGI_FORMAT_R8G8B8A8_UNORM / DXGI_FORMAT_R8G8B8A8_TYPELESS
	MikanColorBuffer_RGBA32 ENUMVALUE_STRING("RGBA32"), 
	// DXGI_FORMAT_B8G8R8A8_UNORM / DXGI_FORMAT_B8G8R8A8_TYPELESS
	MikanColorBuffer_BGRA32 ENUMVALUE_STRING("BGRA32"), 
};

enum ENUM(Serialization::CodeGenModule("MikanCoreConstants")) MikanDepthBufferType
{
	MikanDepthBuffer_NODEPTH ENUMVALUE_STRING("NOCOLOR"),
	// Raw float non-linear depth values from the z-buffer (in source world units)
	MikanDepthBuffer_FLOAT_DEVICE_DEPTH ENUMVALUE_STRING("FLOAT_DEVICE_DEPTH"),
	// Linearized float distance-from-camera values (in source world units)
	MikanDepthBuffer_FLOAT_SCENE_DEPTH ENUMVALUE_STRING("FLOAT_SCENE_DEPTH"), 
	// DXGI_FORMAT_R8G8B8A8_UNORM / DXGI_FORMAT_R8G8B8A8_TYPELESS
	MikanDepthBuffer_PACK_DEPTH_RGBA ENUMVALUE_STRING("PACK_DEPTH_RGBA"), 
};

struct STRUCT(Serialization::CodeGenModule("MikanCoreTypes")) MikanRenderTargetDescriptor
{
	MikanColorBufferType color_buffer_type;
	MikanDepthBufferType depth_buffer_type;
	uint32_t width;
	uint32_t height;
	MikanClientGraphicsApi graphicsAPI;

#ifdef MIKANCORE_REFLECTION_ENABLED
	MikanRenderTargetDescriptor_GENERATED
#endif // MIKANCORE_REFLECTION_ENABLED
};

struct STRUCT(Serialization::CodeGenModule("MikanCoreTypes")) MikanClientFrameRendered
{
	uint64_t frame_index;

#ifdef MIKANCORE_REFLECTION_ENABLED
	MikanClientFrameRendered_GENERATED
#endif // MIKANCORE_REFLECTION_ENABLED
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


#ifdef MIKANCORE_REFLECTION_ENABLED
File_MikanCoreTypes_GENERATED
#endif // MIKANCORE_REFLECTION_ENABLED