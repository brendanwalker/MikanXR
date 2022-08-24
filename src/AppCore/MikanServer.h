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
	InterprocessRenderTargetReadAccessor* renderTargetReadAccessor;

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

	void publishNewVideoFrameEvent(const MikanVideoSourceNewFrameEvent& newFrameEvent);

	void getConnectedClientInfoList(std::vector<MikanClientConnectionInfo>& outClientList) const;
	void getAllStencilList(std::vector<const MikanStencilQuad*>& outStencilList) const;
	void getRelevantQuadStencilList(
		const glm::vec3& cameraPosition, 
		const glm::vec3& cameraForward,
		std::vector<const MikanStencilQuad*>& outStencilList) const;
	void getRelevantModelStencilList(
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

	void allocateQuadStencil(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);
	void getStencilList(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);
	void getQuadStencil(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);
	void updateQuadStencil(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);
	void freeQuadStencil(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);

	void getSpatialAnchorList(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);
	void getSpatialAnchorInfo(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);
	void findSpatialAnchorInfoByName(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult);

	// VRManager Callbacks
	void publishVRDevicePoses(uint64_t newFrameIndex);

private:
	static MikanServer* m_instance;

	std::map<std::string, class ClientConnectionState*> m_clientConnections;
	class InterprocessMessageServer* m_messageServer;
};

#endif // MIKAN_SERVER_H