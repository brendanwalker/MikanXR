// -- includes -----
#include "MikanCoreCAPI.h"
#include "MikanClient.h"
#include "MikanConstants.h"
#include "Logger.h"

#include <map>
#include <assert.h>

#ifdef _MSC_VER
	#pragma warning(disable:4996)  // ignore strncpy warning
#endif

// -- macros -----
#define IS_VALID_VR_DEVICE_INDEX(x) ((x) >= 0 && (x) < MAX_MIKAN_VR_DEVICES)

// -- constants ----
#define MIKAN_CORE_SDK_VERSION		0

// -- private data ---
MikanClient *g_mikanClient= nullptr;
void* g_graphicsDeviceInterfaces[MikanClientGraphicsApi_COUNT] = {
	nullptr, // DirectX9
	nullptr, // DirectX11
	nullptr, // DirectX12
	nullptr, // OpenGL
	nullptr, // Metal
	nullptr  // Vulkan
};


// -- public interface -----

MikanResult Mikan_Initialize(MikanLogLevel log_level, MikanLogCallback log_callback)
{
    if (g_mikanClient != nullptr)
        return MikanResult_Success;

    g_mikanClient = new MikanClient();

	MikanResult resultCode= g_mikanClient->startup((LogSeverityLevel)log_level, log_callback);
    if (resultCode != MikanResult_Success)
    {
        delete g_mikanClient;
    }

    return resultCode;
}

int Mikan_GetCoreSDKVersion()
{
	return MIKAN_CORE_SDK_VERSION;
}

const char* Mikan_GetClientUniqueID()
{
	return (g_mikanClient != nullptr) ? g_mikanClient->getClientUniqueID().c_str() : nullptr;
}

bool Mikan_GetIsInitialized()
{
	return g_mikanClient != nullptr;
}

MikanResult Mikan_SetClientProperty(const char* key, const char* value)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (key == nullptr)
		return MikanResult_NullParam;
	if (value == nullptr)
		return MikanResult_NullParam;

	return g_mikanClient->setClientProperty(key, value);
}

MikanResult Mikan_Connect(const char* host, const char* port)
{
    if (g_mikanClient == nullptr)
        return MikanResult_Uninitialized;
	if (host == nullptr)
		return MikanResult_NullParam;
	if (port == nullptr)
		return MikanResult_NullParam;

    if (g_mikanClient->getIsConnected())
        return MikanResult_Success;
        
    return g_mikanClient->connect(host, port);
}

bool Mikan_GetIsConnected()
{
	return g_mikanClient != nullptr && g_mikanClient->getIsConnected();
}

MikanResult Mikan_FetchNextEvent(
	size_t utf8_buffer_size,
	char* out_utf8_buffer,
	size_t* out_utf8_bytes_written)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;

	// Poll events queued up by the call to g_mikanClient->update()
	return g_mikanClient->fetchNextEvent(utf8_buffer_size, out_utf8_buffer, out_utf8_bytes_written);
}

// Sends a utf8 string to the server and registers a callback to be called when the response is received
// Callback will be called on the web socket thread
MikanResult Mikan_SendRequest(
	const char* utf8_request_name, 
	const char* utf8_payload,
	int request_version,
	MikanRequestID* out_request_id)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;

	return g_mikanClient->sendRequest(utf8_request_name, utf8_payload, request_version, out_request_id);
}

MikanResult Mikan_SetResponseCallback(
	MikanResponseCallback callback,
	void* callback_userdata)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;

	return g_mikanClient->setResponseCallback(callback, callback_userdata);
}

MikanResult Mikan_SetGraphicsDeviceInterface(MikanClientGraphicsApi api, void* graphicsDeviceInterface)
{
	if (api < 0 || api >= MikanClientGraphicsApi_COUNT)
		return MikanResult_InvalidAPI;

	g_graphicsDeviceInterfaces[api]= graphicsDeviceInterface;

	return MikanResult_Success;
}

MikanResult Mikan_GetGraphicsDeviceInterface(MikanClientGraphicsApi api, void** outGraphicsDeviceInterface)
{
	if (api < 0 || api >= MikanClientGraphicsApi_COUNT)
		return MikanResult_InvalidAPI;
	if (outGraphicsDeviceInterface == nullptr)
		return MikanResult_NullParam;

	*outGraphicsDeviceInterface= g_graphicsDeviceInterfaces[api];
	return MikanResult_Success;
}

MikanResult Mikan_AllocateRenderTargetTextures(
	const MikanRenderTargetDescriptor* descriptor, 
	MikanRequestID* out_request_id)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (!g_mikanClient->getIsConnected())
		return MikanResult_NotConnected;
	if (descriptor == nullptr)
		return MikanResult_NullParam;
	if (out_request_id == nullptr)
		return MikanResult_NullParam;

	return g_mikanClient->allocateRenderTargetTextures(*descriptor, out_request_id);
}

MikanResult Mikan_PublishRenderTargetTextures(
	void* ApiColorTexturePtr,
	void* ApiDepthTexturePtr,
	MikanClientFrameRendered* frame_info)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (!g_mikanClient->getIsConnected())
		return MikanResult_NotConnected;
	if (frame_info == nullptr)
		return MikanResult_NullParam;

	return g_mikanClient->publishRenderTargetTextures(ApiColorTexturePtr, ApiDepthTexturePtr, *frame_info);
}

MikanResult Mikan_FreeRenderTargetTextures(MikanRequestID* out_request_id)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (out_request_id == nullptr)
		return MikanResult_NullParam;

	return g_mikanClient->freeRenderTargetTextures(out_request_id);
}

void* Mikan_GetPackDepthTextureResourcePtr()
{
	return g_mikanClient != nullptr ? g_mikanClient->getPackDepthTextureResourcePtr() : nullptr;
}

MikanResult Mikan_Disconnect()
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;

	return g_mikanClient->disconnect();
}

MikanResult Mikan_Shutdown()
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;

	MikanResult resultCode= g_mikanClient->shutdown();

	delete g_mikanClient;
	g_mikanClient= nullptr;

	return resultCode;
}