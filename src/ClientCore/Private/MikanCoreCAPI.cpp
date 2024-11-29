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

// -- constants ----
#define MIKAN_CLIENT_API_VERSION		0

// -- public interface -----

MikanCoreResult Mikan_Initialize(
	MikanLogLevel log_level, 
	MikanLogCallback log_callback,
	MikanContext* outContext)
{
	assert(outContext != nullptr);
    if (*outContext != nullptr)
        return MikanCoreResult_Success;

    MikanClient* context = new MikanClient();

	MikanCoreResult resultCode= context->startup((LogSeverityLevel)log_level, log_callback);
    if (resultCode != MikanCoreResult_Success)
    {
        delete context;
    }

	*outContext= context;

    return resultCode;
}

int Mikan_GetClientAPIVersion()
{
	return MIKAN_CLIENT_API_VERSION;
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

MikanCoreResult Mikan_GetRenderTargetDescriptor(
	MikanContext context,
	MikanRenderTargetDescriptor* out_descriptor)
{
	auto* mikanClient = reinterpret_cast<MikanClient*>(context);
	if (mikanClient == nullptr)
		return MikanCoreResult_Uninitialized;

	if (!mikanClient->getIsConnected())
		return MikanCoreResult_NotConnected;
	if (out_descriptor == nullptr)
		return MikanCoreResult_NullParam;

	return mikanClient->getRenderTargetDescriptor(*out_descriptor);
}

MikanCoreResult Mikan_AllocateRenderTargetTextures(
	MikanContext context,
	const MikanRenderTargetDescriptor* descriptor)
{
	auto* mikanClient = reinterpret_cast<MikanClient*>(context);
	if (mikanClient == nullptr)
		return MikanCoreResult_Uninitialized;

	if (!mikanClient->getIsConnected())
		return MikanCoreResult_NotConnected;
	if (descriptor == nullptr)
		return MikanCoreResult_NullParam;

	return mikanClient->allocateRenderTargetTextures(*descriptor);
}

MikanCoreResult Mikan_FreeRenderTargetTextures(MikanContext context)
{
	auto* mikanClient = reinterpret_cast<MikanClient*>(context);
	if (mikanClient == nullptr)
		return MikanCoreResult_Uninitialized;

	return mikanClient->freeRenderTargetTextures();
}

MikanCoreResult Mikan_WriteColorRenderTargetTexture(MikanContext context, void* color_texture)
{
	auto* mikanClient = reinterpret_cast<MikanClient*>(context);
	if (mikanClient == nullptr)
		return MikanCoreResult_Uninitialized;
	if (!mikanClient->getIsConnected())
		return MikanCoreResult_NotConnected;
	if (color_texture == nullptr)
		return MikanCoreResult_NullParam;

	return mikanClient->writeColorRenderTargetTexture(color_texture);
}

MikanCoreResult Mikan_WriteDepthRenderTargetTexture(
	MikanContext context,
	void* depth_texture,
	float z_near,
	float z_far)
{
	auto* mikanClient = reinterpret_cast<MikanClient*>(context);
	if (mikanClient == nullptr)
		return MikanCoreResult_Uninitialized;
	if (!mikanClient->getIsConnected())
		return MikanCoreResult_NotConnected;
	if (depth_texture == nullptr)
		return MikanCoreResult_NullParam;

	return mikanClient->writeDepthRenderTargetTexture(depth_texture, z_near, z_far);
}

void* Mikan_GetPackDepthTextureResourcePtr(MikanContext context)
{
	auto* mikanClient = reinterpret_cast<MikanClient*>(context);

	return mikanClient != nullptr ? mikanClient->getPackDepthTextureResourcePtr() : nullptr;
}

MikanCoreResult Mikan_Connect(
	MikanContext context, 
	const char* host, 
	const char* port)
{
	auto* mikanClient = reinterpret_cast<MikanClient*>(context);
	if (mikanClient == nullptr)
		return MikanCoreResult_Uninitialized;

	if (mikanClient == nullptr)
		return MikanCoreResult_Uninitialized;
	if (host == nullptr)
		return MikanCoreResult_NullParam;
	if (port == nullptr)
		return MikanCoreResult_NullParam;

	if (mikanClient->getIsConnected())
		return MikanCoreResult_Success;

	return mikanClient->connect(host, port);
}

bool Mikan_GetIsConnected(MikanContext context)
{
	auto* mikanClient= reinterpret_cast<MikanClient*>(context);

	return mikanClient != nullptr && mikanClient->getIsConnected();
}

MikanCoreResult Mikan_FetchNextEvent(
	MikanContext context,
	size_t utf8_buffer_size,
	char* out_utf8_buffer,
	size_t* out_utf8_bytes_written)
{
	auto* mikanClient= reinterpret_cast<MikanClient*>(context);
	if (mikanClient == nullptr)
		return MikanCoreResult_Uninitialized;

	// Poll events queued up by the call to g_mikanClient->update()
	return mikanClient->fetchNextEvent(utf8_buffer_size, out_utf8_buffer, out_utf8_bytes_written);
}

MikanCoreResult Mikan_SendRequestJSON(
	MikanContext context,
	const char* utf8_request_json)
{
	auto* mikanClient = reinterpret_cast<MikanClient*>(context);
	if (mikanClient == nullptr)
		return MikanCoreResult_Uninitialized;

	return mikanClient->sendRequestJSON(utf8_request_json);
}


MikanCoreResult Mikan_SetTextResponseCallback(
	MikanContext context,
	MikanTextResponseCallback callback,
	void* callback_userdata)
{
	auto* mikanClient= reinterpret_cast<MikanClient*>(context);

	if (mikanClient == nullptr)
		return MikanCoreResult_Uninitialized;

	return mikanClient->setTextResponseCallback(callback, callback_userdata);
}

MikanCoreResult Mikan_SetBinaryResponseCallback(
	MikanContext context,
	MikanBinaryResponseCallback callback,
	void* callback_userdata)
{
	auto* mikanClient= reinterpret_cast<MikanClient*>(context);

	if (mikanClient == nullptr)
		return MikanCoreResult_Uninitialized;

	return mikanClient->setBinaryResponseCallback(callback, callback_userdata);
}

MikanCoreResult Mikan_SetGraphicsDeviceInterface(
	MikanContext context,
	MikanClientGraphicsApi api, 
	void* graphicsDeviceInterface)
{
	auto* mikanClient= reinterpret_cast<MikanClient*>(context);

	if (mikanClient == nullptr)
		return MikanCoreResult_Uninitialized;

	return mikanClient->setGraphicsDeviceInterface(api, graphicsDeviceInterface);
}

MikanCoreResult Mikan_GetGraphicsDeviceInterface(
	MikanContext context,
	MikanClientGraphicsApi api, 
	void** outGraphicsDeviceInterface)
{
	auto* mikanClient= reinterpret_cast<MikanClient*>(context);

	if (mikanClient == nullptr)
		return MikanCoreResult_Uninitialized;

	return mikanClient->getGraphicsDeviceInterface(api, outGraphicsDeviceInterface);
}

MikanCoreResult Mikan_Disconnect(MikanContext context)
{
	auto* mikanClient = reinterpret_cast<MikanClient*>(context);

	if (mikanClient == nullptr)
		return MikanCoreResult_Uninitialized;

	return mikanClient->disconnect();
}

MikanCoreResult Mikan_Shutdown(MikanContext* context)
{
	assert(context != nullptr);
	if (*context == nullptr)
		return MikanCoreResult_Uninitialized;

	auto* mikanClient= reinterpret_cast<MikanClient*>(context);
	MikanCoreResult resultCode= mikanClient->shutdown();

	delete mikanClient;
	*context= nullptr;

	return resultCode;
}