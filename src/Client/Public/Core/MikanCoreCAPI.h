/**
\file
*/ 

#ifndef __MIKAN_CLIENT_CAPI_H
#define __MIKAN_CLIENT_CAPI_H
#include "MikanExport.h"
#include "MikanCoreTypes.h"

//cut_before

/** 
\brief Core C-API Interface for MikanXR
\defgroup MikanCore_CAPI Client Interface
\addtogroup MikanCore_CAPI 
@{ 
*/

/** \brief Initializes the MikanXR Client API.
 This function must be called before calling any other client functions. 
 Calling this function after the api is already initialized will return MikanResult_Success.

 \returns MikanResult_Success on success or MikanResult_Error on a general connection error.
 */
MIKAN_PUBLIC_FUNCTION(MikanResult) Mikan_Initialize(MikanLogLevel min_log_level, MikanLogCallback log_callback);

MIKAN_PUBLIC_FUNCTION(int) Mikan_GetCoreSDKVersion();

MIKAN_PUBLIC_FUNCTION(bool) Mikan_GetIsInitialized();

MIKAN_PUBLIC_FUNCTION(MikanResult) Mikan_SetGraphicsDeviceInterface(
	MikanClientGraphicsApi api, 
	void* graphicsDeviceInterface);

MIKAN_PUBLIC_FUNCTION(MikanResult) Mikan_GetGraphicsDeviceInterface(
	MikanClientGraphicsApi api, 
	void** outGraphicsDeviceInterface);

MIKAN_PUBLIC_FUNCTION(MikanResult) Mikan_AllocateRenderTargetTextures(
	const MikanRenderTargetDescriptor* descriptor, MikanRequestID* out_request_id);

MIKAN_PUBLIC_FUNCTION(MikanResult) Mikan_PublishRenderTargetTextures(
	void* ApiColorTexturePtr,
	void* ApiDepthTexturePtr,
	MikanClientFrameRendered* frame_info);

MIKAN_PUBLIC_FUNCTION(MikanResult) Mikan_FreeRenderTargetTextures(MikanRequestID* out_request_id);

MIKAN_PUBLIC_FUNCTION(MikanResult) Mikan_SetClientProperty(const char* key, const char* value);

/** \brief Initializes a connection to MikanXR.
 Starts connection process to MikanXR at the given address and port. 
 Calling this function again after a connection has already been requested will return MikanResult_RequestSent.
	.   
 \param host The address that MikanXR is running at, usually MIKANXR_DEFAULT_ADDRESS
 \param port The port that MikanXR is running at, usually MIKANXR_DEFAULT_PORT
 \returns MikanResult_RequestSent on success, MikanResult_Timeout, or PSMResult_Error on a general connection error.
 */
MIKAN_PUBLIC_FUNCTION(MikanResult) Mikan_Connect(const char* host, const char* port);

/** \brief Get the client connection status
    \return true if the client is connected to MikanXR Client API
 */
MIKAN_PUBLIC_FUNCTION(bool) Mikan_GetIsConnected();

// Copies the buffer into the provided buffer and returns the number of bytes written
// If no buffer is provided, the function will return the size of the buffer needed to store the event
MIKAN_PUBLIC_FUNCTION(MikanResult) Mikan_FetchNextEvent(
	size_t utf8_buffer_size,
	char* out_utf8_buffer,
	size_t* out_utf8_bytes_written);

// Sends a utf8 string to the server and registers a callback to be called when the response is received
// Callback will be called on the web socket thread
MIKAN_PUBLIC_FUNCTION(MikanResult) Mikan_SendRequest(
	const char* utf8_request_name, 
	const char* utf8_payload,
	int request_version,
	MikanRequestID* out_request_id);

MIKAN_PUBLIC_FUNCTION(MikanResult) Mikan_SetResponseCallback(
	MikanResponseCallback callback, 
	void* callback_userdata);


MIKAN_PUBLIC_FUNCTION(MikanResult) Mikan_Disconnect();

/** \brief Cleans up the MikanXR Client API
 Free the resources allocated by the MikanXR Client API.
 Calling this function again after the api already cleaned up will return MikanResult_Error.

  \returns MikanResult_Success on success or MikanResult_Error if there was no valid connection.
 */
MIKAN_PUBLIC_FUNCTION(MikanResult) Mikan_Shutdown();


/** 
@} 
*/ 

//cut_after
#endif
