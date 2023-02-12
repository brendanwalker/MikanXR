#ifndef MIKAN_CLIENT_H
#define MIKAN_CLIENT_H

//-- includes -----
#include "MikanClientTypes.h"
#include "Logger.h"

//-- definitions -----
class MikanClient
{
public:
    MikanClient();
    virtual ~MikanClient();

	// -- State Queries ----
	bool getIsConnected() const;

    // -- ClientMikanAPI System -----
    MikanResult startup(LogSeverityLevel log_level, t_logCallback log_callback);
	MikanResult connect(MikanClientInfo& ClientInfo);
	MikanResult disconnect();
	MikanResult pollNextEvent(MikanEvent& message);
	MikanResult shutdown();

	MikanResult getVideoSourceIntrinsics(MikanVideoSourceIntrinsics& out_intrinsics);
	MikanResult getVideoSourceMode(MikanVideoSourceMode& out_info);
	MikanResult getVideoSourceAttachment(MikanVideoSourceAttachmentInfo& out_info);

	MikanResult getVRDeviceList(MikanVRDeviceList& out_vr_device_list);
	MikanResult getVRDeviceInfo(MikanVRDeviceID device_id, MikanVRDeviceInfo& out_vr_device_info);
	MikanResult subscribeToVRDevicePoseUpdates(MikanVRDeviceID device_id);
	MikanResult unsubscribeFromVRDevicePoseUpdates(MikanVRDeviceID device_id);

	MikanResult allocateRenderTargetBuffers(const MikanRenderTargetDescriptor& descriptor, MikanRenderTargetMemory* out_memory_ptr);
	MikanResult publishRenderTargetTexture(void* ApiTexturePtr, uint64_t frame_index);
	MikanResult publishRenderTargetBuffers(uint64_t frame_index);
	MikanResult freeRenderTargetBuffers();

	MikanResult getStencilList(MikanStencilList& out_stencil_list);
	MikanResult getQuadStencil(MikanStencilID stencil_id, MikanStencilQuad& out_stencil);
	MikanResult getModelStencil(MikanStencilID stencil_id, MikanStencilModel& out_stencil);

	MikanResult getSpatialAnchorList(MikanSpatialAnchorList& out_anchor_list);
	MikanResult getSpatialAnchorInfo(MikanSpatialAnchorID anchor_id, MikanSpatialAnchorInfo& out_anchor_info);
	MikanResult findSpatialAnchorInfoByName(const char* anchor_name, MikanSpatialAnchorInfo& out_anchor_info);
 
private:   
	std::string m_clientName;
	class InterprocessRenderTargetWriteAccessor* m_renderTargetWriter;
	class InterprocessMessageClient* m_messageClient;
	bool m_bIsConnected;
};


#endif // MIKAN_CLIENT_H