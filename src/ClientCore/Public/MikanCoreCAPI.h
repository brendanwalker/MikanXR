#pragma once

#include "MikanCoreExport.h"
#include "MikanCoreTypes.h"

typedef void* MikanContext;

/** \brief Initializes the MikanXR Client API.
 This function must be called before calling any other client functions. 
 Calling this function after the api is already initialized will return MikanResult_Success.

 \returns MikanResult_Success on success or MikanResult_Error on a general connection error.
 */
MIKAN_CORE_CAPI(MikanResult) Mikan_Initialize(
	MikanLogLevel min_log_level, 
	MikanLogCallback log_callback, 
	MikanContext* outContext);

MIKAN_CORE_CAPI(MikanResult) Mikan_SetClientInfo(
	MikanContext context, 
	const char* clientInfo);

MIKAN_CORE_CAPI(int) Mikan_GetCoreSDKVersion();

MIKAN_CORE_CAPI(const char *) Mikan_GetClientUniqueID(MikanContext context);

MIKAN_CORE_CAPI(bool) Mikan_GetIsInitialized(MikanContext context);

MIKAN_CORE_CAPI(MikanResult) Mikan_SetGraphicsDeviceInterface(
	MikanContext context,
	MikanClientGraphicsApi api, 
	void* graphicsDeviceInterface);

MIKAN_CORE_CAPI(MikanResult) Mikan_GetGraphicsDeviceInterface(
	MikanContext context,
	MikanClientGraphicsApi api, 
	void** outGraphicsDeviceInterface);

MIKAN_CORE_CAPI(MikanResult) Mikan_GetRenderTargetDescriptor(
	MikanContext context,
	MikanRenderTargetDescriptor* out_descriptor);

MIKAN_CORE_CAPI(MikanResult) Mikan_AllocateRenderTargetTextures(
	MikanContext context, 
	const MikanRenderTargetDescriptor* descriptor);

MIKAN_CORE_CAPI(MikanResult) Mikan_FreeRenderTargetTextures(MikanContext context);

MIKAN_CORE_CAPI(MikanResult) Mikan_WriteColorRenderTargetTexture(
	MikanContext context,
	void* color_texture);

MIKAN_CORE_CAPI(MikanResult) Mikan_WriteDepthRenderTargetTexture(
	MikanContext context, 
	void* depth_texture, 
	float z_near, 
	float z_far);

MIKAN_CORE_CAPI(void*) Mikan_GetPackDepthTextureResourcePtr(MikanContext context);

/** \brief Initializes a connection to MikanXR.
 Starts connection process to MikanXR at the given address and port. 
 Calling this function again after a connection has already been requested will return MikanResult_RequestSent.
	.   
 \param host The address that MikanXR is running at, usually MIKANXR_DEFAULT_ADDRESS
 \param port The port that MikanXR is running at, usually MIKANXR_DEFAULT_PORT
 \returns MikanResult_RequestSent on success, MikanResult_Timeout, or PSMResult_Error on a general connection error.
 */
MIKAN_CORE_CAPI(MikanResult) Mikan_Connect(
	MikanContext context, 
	const char* host, 
	const char* port);

MIKAN_CORE_CAPI(MikanResult) Mikan_Disconnect(MikanContext context);

/** \brief Get the client connection status
    \return true if the client is connected to MikanXR Client API
 */
MIKAN_CORE_CAPI(bool) Mikan_GetIsConnected(MikanContext context);

// Copies the buffer into the provided buffer and returns the number of bytes written
// If no buffer is provided, the function will return the size of the buffer needed to store the event
MIKAN_CORE_CAPI(MikanResult) Mikan_FetchNextEvent(
	MikanContext context,
	size_t utf8_buffer_size,
	char* out_utf8_buffer,
	size_t* out_utf8_bytes_written);

// Sends a MikanRequest as a UTF8 encoded JSON string
MIKAN_CORE_CAPI(MikanResult) Mikan_SendRequestJSON(
	MikanContext context,
	const char* utf8_request_json);

MIKAN_CORE_CAPI(MikanResult) Mikan_SetTextResponseCallback(
	MikanContext context,
	MikanTextResponseCallback callback, 
	void* callback_userdata);

MIKAN_CORE_CAPI(MikanResult) Mikan_SetBinaryResponseCallback(
	MikanContext context,
	MikanBinaryResponseCallback callback,
	void* callback_userdata);

/** \brief Cleans up the MikanXR Client API
 Free the resources allocated by the MikanXR Client API.
 Calling this function again after the api already cleaned up will return MikanResult_Error.

  \returns MikanResult_Success on success or MikanResult_Error if there was no valid connection.
 */
MIKAN_CORE_CAPI(MikanResult) Mikan_Shutdown(MikanContext* context);