#ifndef MIKAN_SERVER_H
#define MIKAN_SERVER_H

//-- includes -----
#include "MikanClientTypes.h"
#include "MulticastDelegate.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "stdint.h"
#include <map>

//-- definitions -----
struct MikanClientConnectionInfo
{
	std::string clientId;
	MikanClientInfo clientInfo;
	class InterprocessRenderTargetReadAccessor* renderTargetReadAccessor;

	bool hasAllocatedRenderTarget() const;
};

class MikanServer
{
public:
	MikanServer();
	virtual ~MikanServer();

	static MikanServer* getInstance() { return m_instance; }

	bool startup();
	void update();
	void shutdown();

	// Video Source Events
	void publishVideoSourceOpenedEvent();
	void publishVideoSourceClosedEvent();
	void publishVideoSourceNewFrameEvent(const MikanVideoSourceNewFrameEvent& newFrameEvent);
	void publishVideoSourceAttachmentChangedEvent();
	void publishVideoSourceIntrinsicsChangedEvent();
	void publishVideoSourceModeChangedEvent();

	// Spatial Anchor Events
	void publishAnchorPoseUpdatedEvent(const MikanAnchorPoseUpdateEvent& newPoseEvent);
	void publishAnchorListChangedEvent();

	void getConnectedClientInfoList(std::vector<MikanClientConnectionInfo>& outClientList) const;
	void getRelevantQuadStencilList(
		const std::vector<MikanStencilID>* allowedStencilIds,
		const glm::vec3& cameraPosition, 
		const glm::vec3& cameraForward,
		std::vector<const MikanStencilQuad*>& outStencilList) const;
	void getRelevantBoxStencilList(
		const std::vector<MikanStencilID>* allowedStencilIds,
		const glm::vec3& cameraPosition,
		const glm::vec3& cameraForward,
		std::vector<const MikanStencilBox*>& outStencilList) const;
	void getRelevantModelStencilList(
		const std::vector<MikanStencilID>* allowedStencilIds,
		std::vector<const struct MikanStencilModelConfig*>& outStencilList) const;


	MulticastDelegate<void(const std::string& clientId, const MikanClientInfo& clientInfo) > OnClientConnected;
	MulticastDelegate<void(const std::string& clientId)> OnClientDisconnected;

	MulticastDelegate<void(const std::string& clientId, const MikanClientInfo& clientInfo, class InterprocessRenderTargetReadAccessor* readAccessor) > OnClientRenderTargetAllocated;
	MulticastDelegate<void(const std::string& clientId, class InterprocessRenderTargetReadAccessor* readAccessor)> OnClientRenderTargetReleased;
	MulticastDelegate<void(const std::string& clientId, uint64_t frameIndex)> OnClientRenderTargetUpdated;

protected:
	// RPC Callbacks
	void connect(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);
	void disconnect(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);
	void getVideoSourceIntrinsics(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);
	void getVideoSourceMode(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);
	void getVideoSourceAttachment(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);

	void getVRDeviceList(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);
	void getVRDeviceInfo(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);
	void subscribeToVRDevicePoseUpdates(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);
	void unsubscribeFromVRDevicePoseUpdates(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);

	void allocateRenderTargetBuffers(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);
	void freeRenderTargetBuffers(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);

	void getStencilList(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);
	void getQuadStencil(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);
	void getBoxStencil(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);
	void getModelStencil(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);

	void getSpatialAnchorList(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);
	void getSpatialAnchorInfo(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);
	void findSpatialAnchorInfoByName(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);

	// VRManager Callbacks
	void publishVRDeviceListChanged();
	void publishVRDevicePoses(uint64_t newFrameIndex);

	// Publish helpers
	void publishSimpleEvent(MikanEventType eventType);

private:
	static MikanServer* m_instance;

	std::map<std::string, class ClientConnectionState*> m_clientConnections;
	class InterprocessMessageServer* m_messageServer;
};

#endif // MIKAN_SERVER_H