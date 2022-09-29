//-- includes -----
#include "InterprocessRenderTargetWriter.h"
#include "InterprocessMessages.h"
#include "MikanClient.h"
#include "MikanClient_CAPI.h"
#include "RandomUtils.h"

#include <assert.h>

// -- methods -----
MikanClient::MikanClient()
	: m_clientName(RandomUtils::RandomHexString(16))
	, m_renderTargetWriter(new InterprocessRenderTargetWriteAccessor(m_clientName))
	, m_messageClient(new InterprocessMessageClient())
{
}

MikanClient::~MikanClient()
{
    freeRenderTargetBuffers();
    delete m_renderTargetWriter;
	delete m_messageClient;
}

// -- ClientPSMoveAPI System -----
MikanResult MikanClient::startup(LogSeverityLevel log_level, const char* log_filename)
{
	// Reset status flags
	m_bIsConnected= false;

	log_init(log_level, log_filename != nullptr ? log_filename : "");

    return MikanResult_Success;
}

MikanResult MikanClient::connect(MikanClientInfo& clientInfo)
{
	return m_messageClient->connect(m_clientName, &clientInfo);
}

bool MikanClient::getIsConnected() const
{
	return m_messageClient->getIsConnected();
}

MikanResult MikanClient::disconnect()
{
	MikanResult resultCode= MikanResult_NotConnected;

	if (m_messageClient->getIsConnected())
	{
		m_messageClient->disconnect();
		resultCode= MikanResult_Success;
	}

	return resultCode;
}

MikanResult MikanClient::pollNextEvent(MikanEvent& message)
{
	MikanResult resultCode = MikanResult_NotConnected;

	if (m_messageClient->getIsConnected())
	{
		resultCode = m_messageClient->tryFetchNextServerEvent(&message) ? MikanResult_Success : MikanResult_NoData;
	}

	return resultCode;
}

MikanResult MikanClient::shutdown()
{
	log_dispose();
	m_messageClient->disconnect();
	freeRenderTargetBuffers();

	return MikanResult_Success;
}

MikanResult callRPC(
	InterprocessMessageClient* mesgClient,
	const char* functionName,
	uint8_t* parameter_buffer,
	size_t parameter_size)
{
	MikanRemoteFunctionResult functionResponse;
	MikanResult resultCode =
		mesgClient->callRemoteFunction(
			functionName, parameter_buffer, parameter_size, &functionResponse);

	if (resultCode == MikanResult_Success)
	{
		resultCode = functionResponse.getResultCode();
	}

	return resultCode;
}

template <typename t_result_type>
MikanResult callRPC(
	InterprocessMessageClient* mesgClient,
	const char* functionName, 
	uint8_t* parameter_buffer, 
	size_t parameter_size,
	t_result_type& out_response)
{
	MikanRemoteFunctionResult functionResponse;
	MikanResult resultCode= 
		mesgClient->callRemoteFunction(
			functionName, parameter_buffer, parameter_size, &functionResponse);

	if (resultCode == MikanResult_Success)
	{
		resultCode= functionResponse.getResultCode();

		if (resultCode == MikanResult_Success)
		{
			if (!functionResponse.extractResult(out_response))
			{
				resultCode= MikanResult_MalformedResponse;
			}
		}
	}

	return resultCode;
}

MikanResult MikanClient::getVideoSourceIntrinsics(MikanVideoSourceIntrinsics& out_intrinsics)
{
	return callRPC(m_messageClient, "getVideoSourceIntrinsics", nullptr, 0, out_intrinsics);
}

MikanResult MikanClient::getVideoSourceMode(MikanVideoSourceMode& out_info)
{
	return callRPC(m_messageClient, "getVideoSourceMode", nullptr, 0, out_info);
}

MikanResult MikanClient::getVideoSourceAttachment(MikanVideoSourceAttachmentInfo& out_info)
{
	return callRPC(m_messageClient, "getVideoSourceAttachment", nullptr, 0, out_info);
}

MikanResult MikanClient::getVRDeviceList(MikanVRDeviceList& out_vr_device_list)
{
	return callRPC(m_messageClient, "getVRDeviceList", nullptr, 0, out_vr_device_list);
}

MikanResult MikanClient::getVRDeviceInfo(MikanVRDeviceID device_id, MikanVRDeviceInfo& out_vr_device_info)
{
	return callRPC(m_messageClient, "getVRDeviceInfo", (uint8_t*)&device_id, sizeof(MikanVRDeviceID), out_vr_device_info);
}

MikanResult MikanClient::subscribeToVRDevicePoseUpdates(MikanVRDeviceID device_id)
{
	return callRPC(m_messageClient, "subscribeToVRDevicePoseUpdates", (uint8_t*)&device_id, sizeof(MikanVRDeviceID));
}

MikanResult MikanClient::unsubscribeFromVRDevicePoseUpdates(MikanVRDeviceID device_id)
{
	return callRPC(m_messageClient, "unsubscribeFromVRDevicePoseUpdates", (uint8_t*)&device_id, sizeof(MikanVRDeviceID));
}

MikanResult MikanClient::allocateRenderTargetBuffers(
	const MikanRenderTargetDescriptor& descriptor, 
	MikanRenderTargetMemory* out_memory_ptr)
{
	MikanResult resultCode;

	// Free any existing buffer if we called allocate already
	freeRenderTargetBuffers();

	// Fetch the cached graphics API interface, if any
	void* apiInterface = nullptr;
	if (descriptor.graphicsAPI != MikanClientGraphicsAPI_UNKNOWN)
	{
		Mikan_GetGraphicsDeviceInterface(descriptor.graphicsAPI, &apiInterface);
	}

	// Create the shared memory buffer
	bool bSuccess= false;
	const MikanClientInfo& clientInfo= m_messageClient->getClientInfo();	
	if (m_renderTargetWriter->initialize(&descriptor, apiInterface))
	{
		// Copy the buffer pointers allocated by the render target write
		assert(out_memory_ptr != nullptr);
		*out_memory_ptr = m_renderTargetWriter->getLocalMemory();

		resultCode= callRPC(
			m_messageClient, 
			"allocateRenderTargetBuffers", (uint8_t*)&descriptor, sizeof(MikanRenderTargetDescriptor));
	}
	else
	{
		resultCode= MikanResult_SharedMemoryError;
	}

	return resultCode;
}

MikanResult MikanClient::publishRenderTargetTexture(void* apiTexturePtr, uint64_t frame_index)
{
	return m_renderTargetWriter->writeRenderTargetTexture(apiTexturePtr, frame_index) ? MikanResult_Success : MikanResult_SharedTextureError;
}

MikanResult MikanClient::publishRenderTargetBuffers(uint64_t frame_index)
{
	// Copy the render target buffers in local memory to shared memory
	return m_renderTargetWriter->writeRenderTargetMemory(frame_index) ? MikanResult_Success : MikanResult_SharedMemoryError;
}

MikanResult MikanClient::freeRenderTargetBuffers()
{
	MikanResult resultCode= MikanResult_Success;

	// Tell the server to free it's existing render target (ignored if there isn't a render target allocated)
	if (m_messageClient->getIsConnected())
	{
		resultCode= callRPC(m_messageClient, "freeRenderTargetBuffers", nullptr, 0);
	}

	// Free shared and local memory buffers
	m_renderTargetWriter->dispose();

	return resultCode;
}

MikanResult MikanClient::getStencilList(MikanStencilList& out_stencil_list)
{
	return callRPC(m_messageClient, "getStencilList", nullptr, 0, out_stencil_list);
}

MikanResult MikanClient::getQuadStencil(MikanStencilID stencil_id, MikanStencilQuad& out_stencil)
{
	return callRPC(m_messageClient, "getQuadStencil", (uint8_t*)&stencil_id, sizeof(MikanStencilID), out_stencil);
}

MikanResult MikanClient::getModelStencil(MikanStencilID stencil_id, MikanStencilModel& out_stencil)
{
	return callRPC(m_messageClient, "getModelStencil", (uint8_t*)&stencil_id, sizeof(MikanStencilID), out_stencil);
}

MikanResult MikanClient::getSpatialAnchorList(MikanSpatialAnchorList& out_anchor_list)
{
	return callRPC(m_messageClient, "getSpatialAnchorList", nullptr, 0, out_anchor_list);
}

MikanResult MikanClient::getSpatialAnchorInfo(MikanSpatialAnchorID anchor_id, MikanSpatialAnchorInfo& out_anchor_info)
{
	return callRPC(m_messageClient, "getSpatialAnchorInfo", (uint8_t*)&anchor_id, sizeof(MikanVRDeviceID), out_anchor_info);
}

MikanResult MikanClient::findSpatialAnchorInfoByName(const char* anchor_name, MikanSpatialAnchorInfo& out_anchor_info)
{
	char nameBuffer[MAX_MIKAN_ANCHOR_NAME_LEN];
	memset(&nameBuffer, 0, sizeof(nameBuffer));
	strncpy(nameBuffer, anchor_name, sizeof(nameBuffer)-1);

	return callRPC(
		m_messageClient, 
		"findSpatialAnchorInfoByName", 
		(uint8_t*)&nameBuffer, sizeof(nameBuffer), 
		out_anchor_info);
}