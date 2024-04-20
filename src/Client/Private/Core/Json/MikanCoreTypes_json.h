#pragma once

#include "MikanCoreTypes.h"

#include "nlohmann/json.hpp"

NLOHMANN_JSON_SERIALIZE_ENUM(MikanResult, {
	{MikanResult_Success, "Success"},
	{MikanResult_GeneralError, "GeneralError"},
	{MikanResult_Uninitialized, "Uninitialized"},
	{MikanResult_NullParam, "NullParam" },
	{MikanResult_BufferTooSmall, "BufferTooSmall"},
	{MikanResult_InitFailed, "InitFailed"},
	{MikanResult_ConnectionFailed, "ConnectionFailed"},
	{MikanResult_AlreadyConnected, "AlreadyConnected"},
	{MikanResult_NotConnected, "NotConnected"},
	{MikanResult_SocketError, "SocketError"},
	{MikanResult_NoData, "NoData"},
	{MikanResult_Timeout, "Timeout"},
	{MikanResult_Canceled, "Canceled"},
	{MikanResult_UnknownClient, "UnknownClient"},
	{MikanResult_UnknownFunction, "UnknownFunction"},
	{MikanResult_FailedFunctionSend, "FailedFunctionSend"},
	{MikanResult_FunctionResponseTimeout, "FunctionResponseTimeout"},
	{MikanResult_MalformedParameters, "MalformedParameters"},
	{MikanResult_MalformedResponse, "MalformedResponse"},
	{MikanResult_InvalidAPI, "InvalidAPI"},
	{MikanResult_SharedTextureError, "SharedTextureError"},
	{MikanResult_NoVideoSource, "NoVideoSource"},
	{MikanResult_NoVideoSourceAssignedTracker, "NoVideoSourceAssignedTracker"},
	{MikanResult_InvalidDeviceId, "InvalidDeviceId"},
	{MikanResult_InvalidStencilID, "InvalidStencilID"},
	{MikanResult_InvalidAnchorID, "InvalidAnchorID"},
})

NLOHMANN_JSON_SERIALIZE_ENUM(MikanColorBufferType, {
	{MikanColorBuffer_NOCOLOR, "NOCOLOR"},
	{MikanColorBuffer_RGB24, "RGB24"},
	{MikanColorBuffer_RGBA32, "RGBA32"},
	{MikanColorBuffer_BGRA32, "BGRA32"},
})

NLOHMANN_JSON_SERIALIZE_ENUM(MikanDepthBufferType, {
	{MikanDepthBuffer_NODEPTH, "NODEPTH"},
	{MikanDepthBuffer_FLOAT_DEPTH, "FLOAT_DEPTH"},
	{MikanDepthBuffer_PACK_DEPTH_RGBA, "PACK_DEPTH_RGBA"},
})

NLOHMANN_JSON_SERIALIZE_ENUM(MikanClientGraphicsApi, {
	{MikanClientGraphicsApi_UNKNOWN, nullptr},
	{MikanClientGraphicsApi_Direct3D9, "Direct3D9"},
	{MikanClientGraphicsApi_Direct3D11, "Direct3D11"},
	{MikanClientGraphicsApi_Direct3D12, "Direct3D12"},
	{MikanClientGraphicsApi_OpenGL, "OpenGL"},
	{MikanClientGraphicsApi_COUNT, nullptr},
})

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanRenderTargetDescriptor,
	color_buffer_type,
	depth_buffer_type,
	width,
	height,
	graphicsAPI
)

NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(MikanClientFrameRendered,
	frame_index
)