// -- includes -----
#include "MikanCoreCAPI.h"
#include "MikanClient.h"
#include "MikanConstants.h"
#include "Logger.h"

#include <map>
#include <assert.h>

#include "MikanCoreTypes.rfks.h"

#ifdef _MSC_VER
	#pragma warning(disable:4996)  // ignore strncpy warning
#endif

// -- macros -----
#define IS_VALID_VR_DEVICE_INDEX(x) ((x) >= 0 && (x) < MAX_MIKAN_VR_DEVICES)

// -- constants ----
#define MIKAN_CORE_SDK_VERSION		0

// -- public interface -----

MikanResult Mikan_Initialize(
	MikanLogLevel log_level, 
	MikanLogCallback log_callback,
	MikanContext* outContext)
{
	assert(outContext != nullptr);
    if (*outContext != nullptr)
        return MikanResult_Success;

    MikanClient* context = new MikanClient();

	MikanResult resultCode= context->startup((LogSeverityLevel)log_level, log_callback);
    if (resultCode != MikanResult_Success)
    {
        delete context;
    }

	*outContext= context;

    return resultCode;
}

int Mikan_GetCoreSDKVersion()
{
	return MIKAN_CORE_SDK_VERSION;
}

const char* Mikan_GetClientUniqueID(MikanContext context)
{
	auto* mikanClient= reinterpret_cast<MikanClient*>(context);

	return (mikanClient != nullptr) ? mikanClient->getClientUniqueID().c_str() : nullptr;
}

bool Mikan_GetIsInitialized(MikanContext context)
{
	return context != nullptr;
}

MikanResult Mikan_SetClientInfo(MikanContext context, const char* clientInfo)
{
	auto* mikanClient = reinterpret_cast<MikanClient*>(context);
	if (mikanClient == nullptr)
		return MikanResult_Uninitialized;

	return mikanClient->setClientInfo(clientInfo);
}

MikanResult Mikan_GetRenderTargetDescriptor(
	MikanContext context,
	MikanRenderTargetDescriptor* out_descriptor)
{
	auto* mikanClient = reinterpret_cast<MikanClient*>(context);
	if (mikanClient == nullptr)
		return MikanResult_Uninitialized;

	if (!mikanClient->getIsConnected())
		return MikanResult_NotConnected;
	if (out_descriptor == nullptr)
		return MikanResult_NullParam;

	return mikanClient->getRenderTargetDescriptor(*out_descriptor);
}

MikanResult Mikan_AllocateRenderTargetTextures(
	MikanContext context,
	const MikanRenderTargetDescriptor* descriptor)
{
	auto* mikanClient = reinterpret_cast<MikanClient*>(context);
	if (mikanClient == nullptr)
		return MikanResult_Uninitialized;

	if (!mikanClient->getIsConnected())
		return MikanResult_NotConnected;
	if (descriptor == nullptr)
		return MikanResult_NullParam;

	return mikanClient->allocateRenderTargetTextures(*descriptor);
}

MikanResult Mikan_FreeRenderTargetTextures(MikanContext context)
{
	auto* mikanClient = reinterpret_cast<MikanClient*>(context);
	if (mikanClient == nullptr)
		return MikanResult_Uninitialized;

	return mikanClient->freeRenderTargetTextures();
}

MikanResult Mikan_WriteColorRenderTargetTexture(MikanContext context, void* color_texture)
{
	auto* mikanClient = reinterpret_cast<MikanClient*>(context);
	if (mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (!mikanClient->getIsConnected())
		return MikanResult_NotConnected;
	if (color_texture == nullptr)
		return MikanResult_NullParam;

	return mikanClient->writeColorRenderTargetTexture(color_texture);
}

MikanResult Mikan_WriteDepthRenderTargetTexture(
	MikanContext context,
	void* depth_texture,
	float z_near,
	float z_far)
{
	auto* mikanClient = reinterpret_cast<MikanClient*>(context);
	if (mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (!mikanClient->getIsConnected())
		return MikanResult_NotConnected;
	if (depth_texture == nullptr)
		return MikanResult_NullParam;

	return mikanClient->writeDepthRenderTargetTexture(depth_texture, z_near, z_far);
}

void* Mikan_GetPackDepthTextureResourcePtr(MikanContext context)
{
	auto* mikanClient = reinterpret_cast<MikanClient*>(context);

	return mikanClient != nullptr ? mikanClient->getPackDepthTextureResourcePtr() : nullptr;
}

MikanResult Mikan_Connect(MikanContext context, const char* host, const char* port)
{
	auto* mikanClient = reinterpret_cast<MikanClient*>(context);
	if (mikanClient == nullptr)
		return MikanResult_Uninitialized;

	if (mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (host == nullptr)
		return MikanResult_NullParam;
	if (port == nullptr)
		return MikanResult_NullParam;

	if (mikanClient->getIsConnected())
		return MikanResult_Success;

	return mikanClient->connect(host, port);
}

bool Mikan_GetIsConnected(MikanContext context)
{
	auto* mikanClient= reinterpret_cast<MikanClient*>(context);

	return mikanClient != nullptr && mikanClient->getIsConnected();
}

MikanResult Mikan_FetchNextEvent(
	MikanContext context,
	size_t utf8_buffer_size,
	char* out_utf8_buffer,
	size_t* out_utf8_bytes_written)
{
	auto* mikanClient= reinterpret_cast<MikanClient*>(context);
	if (mikanClient == nullptr)
		return MikanResult_Uninitialized;

	// Poll events queued up by the call to g_mikanClient->update()
	return mikanClient->fetchNextEvent(utf8_buffer_size, out_utf8_buffer, out_utf8_bytes_written);
}

MikanResult Mikan_SendRequestJSON(
	MikanContext context,
	const char* utf8_request_json)
{
	auto* mikanClient = reinterpret_cast<MikanClient*>(context);
	if (mikanClient == nullptr)
		return MikanResult_Uninitialized;

	return mikanClient->sendRequestJSON(utf8_request_json);
}


MikanResult Mikan_SetTextResponseCallback(
	MikanContext context,
	MikanTextResponseCallback callback,
	void* callback_userdata)
{
	auto* mikanClient= reinterpret_cast<MikanClient*>(context);

	if (mikanClient == nullptr)
		return MikanResult_Uninitialized;

	return mikanClient->setTextResponseCallback(callback, callback_userdata);
}

MikanResult Mikan_SetBinaryResponseCallback(
	MikanContext context,
	MikanBinaryResponseCallback callback,
	void* callback_userdata)
{
	auto* mikanClient= reinterpret_cast<MikanClient*>(context);

	if (mikanClient == nullptr)
		return MikanResult_Uninitialized;

	return mikanClient->setBinaryResponseCallback(callback, callback_userdata);
}

MikanResult Mikan_SetGraphicsDeviceInterface(
	MikanContext context,
	MikanClientGraphicsApi api, 
	void* graphicsDeviceInterface)
{
	auto* mikanClient= reinterpret_cast<MikanClient*>(context);

	if (mikanClient == nullptr)
		return MikanResult_Uninitialized;

	return mikanClient->setGraphicsDeviceInterface(api, graphicsDeviceInterface);
}

MikanResult Mikan_GetGraphicsDeviceInterface(
	MikanContext context,
	MikanClientGraphicsApi api, 
	void** outGraphicsDeviceInterface)
{
	auto* mikanClient= reinterpret_cast<MikanClient*>(context);

	if (mikanClient == nullptr)
		return MikanResult_Uninitialized;

	return mikanClient->getGraphicsDeviceInterface(api, outGraphicsDeviceInterface);
}

MikanResult Mikan_Disconnect(MikanContext context)
{
	auto* mikanClient = reinterpret_cast<MikanClient*>(context);

	if (mikanClient == nullptr)
		return MikanResult_Uninitialized;

	return mikanClient->disconnect();
}

MikanResult Mikan_Shutdown(MikanContext* context)
{
	assert(context != nullptr);
	if (*context == nullptr)
		return MikanResult_Uninitialized;

	auto* mikanClient= reinterpret_cast<MikanClient*>(context);
	MikanResult resultCode= mikanClient->shutdown();

	delete mikanClient;
	*context= nullptr;

	return resultCode;
}