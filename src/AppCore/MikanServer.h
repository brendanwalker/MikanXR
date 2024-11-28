#ifndef MIKAN_SERVER_H
#define MIKAN_SERVER_H

//-- includes -----
#include "CommonConfigFwd.h"
#include "ScriptingFwd.h"
#include "MikanAPITypes.h"
#include "MikanClientTypes.h"
#include "MikanClientEvents.h"
#include "MikanScriptEvents.h"
#include "MikanStencilEvents.h"
#include "MikanSpatialAnchorEvents.h"
#include "MikanVideoSourceEvents.h"
#include "MikanVRDeviceEvents.h"
#include "MulticastDelegate.h"
#include "InterprocessMessages.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "stdint.h"

#include <map>
#include <memory>

class MikanClientConnectionState;
using MikanClientConnectionStatePtr= std::shared_ptr<MikanClientConnectionState>;

//-- definitions -----
struct MikanClientConnectionInfo
{
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

	// Scripting
	void bindScriptContect(CommonScriptContextPtr scriptContext);
	void unbindScriptContect(CommonScriptContextPtr scriptContext);
	void publishScriptMessageEvent(const std::string& message);

	// Video Source Events
	void publishVideoSourceOpenedEvent();
	void publishVideoSourceClosedEvent();
	void publishVideoSourceNewFrameEvent(const MikanVideoSourceNewFrameEvent& newFrameEvent);
	void publishVideoSourceAttachmentChangedEvent();
	void publishVideoSourceIntrinsicsChangedEvent();
	void publishVideoSourceModeChangedEvent();


	// Spatial Anchor Events
	void publishAnchorNameUpdatedEvent(const MikanAnchorNameUpdateEvent& newPoseEvent);
	void publishAnchorPoseUpdatedEvent(const MikanAnchorPoseUpdateEvent& newPoseEvent);
	void handleAnchorSystemConfigChange(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);

	// Stencil Events
	void publishStencilNameUpdatedEvent(const MikanStencilNameUpdateEvent& newPoseEvent);
	void publishStencilPoseUpdatedEvent(const MikanStencilPoseUpdateEvent& newPoseEvent);
	void handleStencilSystemConfigChange(CommonConfigPtr configPtr, const class ConfigPropertyChangeSet& changedPropertySet);

	void getConnectedClientInfoList(std::vector<MikanClientConnectionInfo>& outClientList) const;

	MulticastDelegate<void(const std::string& clientId, const MikanClientInfo& clientInfo) > OnClientInitialized;
	MulticastDelegate<void(const std::string& clientId)> OnClientDisposed;

	MulticastDelegate<void(const std::string& clientId, const MikanClientInfo& clientInfo, class InterprocessRenderTargetReadAccessor* readAccessor) > OnClientRenderTargetAllocated;
	MulticastDelegate<void(const std::string& clientId, class InterprocessRenderTargetReadAccessor* readAccessor)> OnClientRenderTargetReleased;
	MulticastDelegate<void(const std::string& clientId, uint64_t frameIndex)> OnClientRenderTargetUpdated;

protected:
	// Connection State Management
	void allocateClientConnectionState(const std::string& connectionId, const MikanClientInfo& clientInfo);
	void disposeClientConnectionState(const std::string& connectionId);

	// Websocket Event Handlers
	void onClientDisconnected(const std::string& connectionId);	

	// Request Callbacks
	void initClientHandler(const ClientRequest& request, ClientResponse& response);
	void disposeClientHandler(const ClientRequest& request, ClientResponse& response);

	void invokeScriptMessageHandler(const ClientRequest& request, ClientResponse& response);
	
	void getVideoSourceIntrinsics(const ClientRequest& request, ClientResponse& response);
	void getVideoSourceMode(const ClientRequest& request, ClientResponse& response);
	void getVideoSourceAttachment(const ClientRequest& request, ClientResponse& response);

	void getVRDeviceList(const ClientRequest& request, ClientResponse& response);
	void getVRDeviceInfo(const ClientRequest& request, ClientResponse& response);
	void subscribeToVRDevicePoseUpdates(const ClientRequest& request, ClientResponse& response);
	void unsubscribeFromVRDevicePoseUpdates(const ClientRequest& request, ClientResponse& response);

	void allocateRenderTargetTextures(const ClientRequest& request, ClientResponse& response);
	void freeRenderTargetTextures(const ClientRequest& request, ClientResponse& response);
	void frameRendered(const ClientRequest& request, ClientResponse& response);

	void getQuadStencilList(const ClientRequest& request, ClientResponse& response);
	void getQuadStencil(const ClientRequest& request, ClientResponse& response);
	void getBoxStencilList(const ClientRequest& request, ClientResponse& response);
	void getBoxStencil(const ClientRequest& request, ClientResponse& response);
	void getModelStencilList(const ClientRequest& request, ClientResponse& response);
	void getModelStencil(const ClientRequest& request, ClientResponse& response);
	void getModelStencilRenderGeometry(const ClientRequest& request, ClientResponse& response);

	void getSpatialAnchorList(const ClientRequest& request, ClientResponse& response);
	void getSpatialAnchorInfo(const ClientRequest& request, ClientResponse& response);
	void findSpatialAnchorInfoByName(const ClientRequest& request, ClientResponse& response);

	// VRManager Callbacks
	void publishVRDeviceListChanged();
	void publishVRDevicePoses(uint64_t newFrameIndex);

private:
	static MikanServer* m_instance;

	std::vector<CommonScriptContextWeakPtr> m_scriptContexts;
	std::map<std::string, MikanClientConnectionStatePtr> m_clientConnections;
	class IInterprocessMessageServer* m_messageServer;
};

#endif // MIKAN_SERVER_H