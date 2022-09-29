// -- includes -----
#include "MikanClient_CAPI.h"
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

// -- private data ---
MikanClient *g_mikanClient= nullptr;
void* g_graphicsDeviceInterfaces[MikanClientGraphicsAPI_COUNT] = {
	nullptr, // Unknown
	nullptr, // DirectX9
	nullptr, // DirectX11
	nullptr, // DirectX12
	nullptr  // OpenGL
};


// -- public interface -----

MikanResult Mikan_Initialize(MikanLogLevel log_level, const char* log_filename)
{
    if (g_mikanClient != nullptr)
        return MikanResult_Success;

    g_mikanClient = new MikanClient();

	MikanResult resultCode= g_mikanClient->startup((LogSeverityLevel)log_level, log_filename);
    if (resultCode != MikanResult_Success)
    {
        delete g_mikanClient;
    }

    return resultCode;
}

bool Mikan_GetIsInitialized()
{
	return g_mikanClient != nullptr;
}

const char* Mikan_GetVersionString()
{
	const char* version_string = MIKAN_CLIENT_VERSION_STRING;

	return version_string;
}

MikanResult Mikan_Connect(MikanClientInfo* client_info)
{
    if (g_mikanClient == nullptr)
        return MikanResult_Uninitialized;
	if (client_info == nullptr)
		return MikanResult_NullParam;

    if (g_mikanClient->getIsConnected())
        return MikanResult_Success;
        
    return g_mikanClient->connect(*client_info);
}

bool Mikan_GetIsConnected()
{
	return g_mikanClient != nullptr && g_mikanClient->getIsConnected();
}

MikanResult Mikan_PollNextEvent(MikanEvent* out_event)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (out_event == nullptr)
		return MikanResult_NullParam;

	// Poll events queued up by the call to g_mikanClient->update()
	return g_mikanClient->pollNextEvent(*out_event);
}

MikanResult Mikan_GetVideoSourceIntrinsics(MikanVideoSourceIntrinsics* out_intrinsics)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (out_intrinsics == nullptr)
		return MikanResult_NullParam;

	return g_mikanClient->getVideoSourceIntrinsics(*out_intrinsics);
}

MikanResult Mikan_GetVideoSourceMode(MikanVideoSourceMode* out_info)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (out_info == nullptr)
		return MikanResult_NullParam;

	return g_mikanClient->getVideoSourceMode(*out_info);
}

MikanResult Mikan_GetVideoSourceAttachment(MikanVideoSourceAttachmentInfo* out_info)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (out_info == nullptr)
		return MikanResult_NullParam;

	return g_mikanClient->getVideoSourceAttachment(*out_info);
}

MikanResult Mikan_GetVRDeviceList(MikanVRDeviceList* out_vr_device_list)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (out_vr_device_list == nullptr)
		return MikanResult_NullParam;

	return g_mikanClient->getVRDeviceList(*out_vr_device_list);
}

MikanResult Mikan_GetVRDeviceInfo(MikanVRDeviceID device_id, MikanVRDeviceInfo* out_vr_device_info)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (out_vr_device_info == nullptr)
		return MikanResult_NullParam;

	return g_mikanClient->getVRDeviceInfo(device_id, *out_vr_device_info);
}

MikanResult Mikan_SubscribeToVRDevicePoseUpdates(MikanVRDeviceID device_id)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;

	return g_mikanClient->subscribeToVRDevicePoseUpdates(device_id);
}

MikanResult Mikan_UnsubscribeFromVRDevicePoseUpdates(MikanVRDeviceID device_id)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;

	return g_mikanClient->unsubscribeFromVRDevicePoseUpdates(device_id) ;
}

MikanResult Mikan_SetGraphicsDeviceInterface(MikanClientGraphicsAPI api, void* graphicsDeviceInterface)
{
	if (api < 0 || api >= MikanClientGraphicsAPI_COUNT)
		return MikanResult_InvalidAPI;

	g_graphicsDeviceInterfaces[api]= graphicsDeviceInterface;

	return MikanResult_Success;
}

MikanResult Mikan_GetGraphicsDeviceInterface(MikanClientGraphicsAPI api, void** outGraphicsDeviceInterface)
{
	if (api < 0 || api >= MikanClientGraphicsAPI_COUNT)
		return MikanResult_InvalidAPI;
	if (outGraphicsDeviceInterface == nullptr)
		return MikanResult_NullParam;

	*outGraphicsDeviceInterface= g_graphicsDeviceInterfaces[api];
	return MikanResult_Success;
}

MikanResult Mikan_AllocateRenderTargetBuffers(
	const MikanRenderTargetDescriptor* descriptor,
	MikanRenderTargetMemory* out_memory_ptr)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (!g_mikanClient->getIsConnected())
		return MikanResult_NotConnected;
	if (descriptor == nullptr)
		return MikanResult_NullParam; 
	if (out_memory_ptr == nullptr)
		return MikanResult_NullParam;

	return g_mikanClient->allocateRenderTargetBuffers(*descriptor, out_memory_ptr);
}

MikanResult Mikan_PublishRenderTargetTexture(void* ApiTexturePtr, uint64_t frame_index)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (!g_mikanClient->getIsConnected())
		return MikanResult_NotConnected;

	return g_mikanClient->publishRenderTargetTexture(ApiTexturePtr, frame_index);
}

MikanResult Mikan_PublishRenderTargetBuffers(uint64_t frame_index)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;

	return g_mikanClient->publishRenderTargetBuffers(frame_index);
}

MikanResult Mikan_FreeRenderTargetBuffers()
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;

	return g_mikanClient->freeRenderTargetBuffers();
}

MikanResult Mikan_GetStencilList(MikanStencilList* out_stencil_list)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (out_stencil_list == nullptr)
		return MikanResult_NullParam;

	return g_mikanClient->getStencilList(*out_stencil_list);
}

MikanResult Mikan_GetQuadStencil(MikanStencilID stencil_id, MikanStencilQuad* out_stencil)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (out_stencil == nullptr)
		return MikanResult_NullParam;

	return g_mikanClient->getQuadStencil(stencil_id, *out_stencil);
}

MikanResult Mikan_GetModelStencil(MikanStencilID stencil_id, MikanStencilModel* out_stencil)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (out_stencil == nullptr)
		return MikanResult_NullParam;

	return g_mikanClient->getModelStencil(stencil_id, *out_stencil);
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

MikanResult Mikan_GetSpatialAnchorList(MikanSpatialAnchorList* out_anchor_list)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (out_anchor_list == nullptr)
		return MikanResult_NullParam;

	return g_mikanClient->getSpatialAnchorList(*out_anchor_list);
}

MikanResult Mikan_GetSpatialAnchorInfo(
	MikanSpatialAnchorID anchor_id,
	MikanSpatialAnchorInfo* out_anchor_info)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (out_anchor_info == nullptr)
		return MikanResult_NullParam;

	return g_mikanClient->getSpatialAnchorInfo(anchor_id, *out_anchor_info);
}

MikanResult Mikan_FindSpatialAnchorInfoByName(
	const char* anchor_name,
	MikanSpatialAnchorInfo* out_anchor_info)
{
	if (g_mikanClient == nullptr)
		return MikanResult_Uninitialized;
	if (anchor_name == nullptr)
		return MikanResult_NullParam;
	if (out_anchor_info == nullptr)
		return MikanResult_NullParam;

	return g_mikanClient->findSpatialAnchorInfoByName(anchor_name, *out_anchor_info);
}