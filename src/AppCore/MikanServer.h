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
class MikanClientConnectionInfo
{
public:
	MikanClientConnectionInfo();
	virtual ~MikanClientConnectionInfo();

	void clearMikanClientInfo();
	void setClientInfo(const MikanClientInfo& clientInfo);
	const MikanClientInfo& getClientInfo() const;

	const std::string& getClientId() const;
	bool isClientInfoValid() const;

	bool hasAllocatedRenderTarget() const;
	inline class InterprocessRenderTargetReadAccessor* getRenderTargetReadAccessor() const 
	{ return m_renderTargetReadAccessor; }
	bool allocateRenderTargetTextures(const MikanRenderTargetDescriptor& desc);
	void freeRenderTargetTexturesHandler();

protected:
	void allocateRenderTargetAccessor();
	void disposeRenderTargetAccessor();

private:
	MikanClientInfo m_clientInfo;
	class InterprocessRenderTargetReadAccessor* m_renderTargetReadAccessor= nullptr;
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

	void getConnectedClientInfoList(std::vector<const MikanClientConnectionInfo*>& outClientList) const;

	MulticastDelegate<void(const std::string& clientId, const MikanClientInfo& clientInfo) > OnClientInitialized;
	MulticastDelegate<void(const std::string& clientId)> OnClientDisposed;

	MulticastDelegate<void(const std::string& clientId, const MikanClientInfo& clientInfo, class InterprocessRenderTargetReadAccessor* readAccessor) > OnClientRenderTargetAllocated;
	MulticastDelegate<void(const std::string& clientId, class InterprocessRenderTargetReadAccessor* readAccessor)> OnClientRenderTargetReleased;
	MulticastDelegate<void(const std::string& clientId, int64_t frameIndex)> OnClientRenderTargetUpdated;

protected:
	// Connection State Management
	MikanClientConnectionStatePtr allocateClientConnectionState(const std::string& connectionId);
	void disposeClientConnectionState(const std::string& connectionId);
	void initClientInfo(MikanClientConnectionStatePtr connectionState, const MikanClientInfo& clientInfo);
	bool disposeClientInfo(MikanClientConnectionStatePtr connectionState);

	// Websocket Event Handlers
	void onClientConnectedHandler(const ClientSocketEvent& event);
	void onClientDisconnectedHandler(const ClientSocketEvent& event);
	void onClientErrorHandler(const ClientSocketEvent& event);

	// Request Callbacks
	void initClientHandler(const ClientRequest& request, ClientResponse& response);
	void disposeClientHandler(const ClientRequest& request, ClientResponse& response);

	void invokeScriptMessageHandler(const ClientRequest& request, ClientResponse& response);
	
	void getVideoSourceIntrinsicsHandler(const ClientRequest& request, ClientResponse& response);
	void getVideoSourceModeHandler(const ClientRequest& request, ClientResponse& response);
	void getVideoSourceAttachmentHandler(const ClientRequest& request, ClientResponse& response);

	void getVRDeviceListHandler(const ClientRequest& request, ClientResponse& response);
	void getVRDeviceInfoHandler(const ClientRequest& request, ClientResponse& response);
	void subscribeToVRDevicePoseUpdatesHandler(const ClientRequest& request, ClientResponse& response);
	void unsubscribeFromVRDevicePoseUpdatesHandler(const ClientRequest& request, ClientResponse& response);

	void allocateRenderTargetTexturesHandler(const ClientRequest& request, ClientResponse& response);
	void freeRenderTargetTexturesHandler(const ClientRequest& request, ClientResponse& response);
	void frameRenderedHandler(const ClientRequest& request, ClientResponse& response);

	void getQuadStencilListHandler(const ClientRequest& request, ClientResponse& response);
	void getQuadStencilHandler(const ClientRequest& request, ClientResponse& response);
	void getBoxStencilListHandler(const ClientRequest& request, ClientResponse& response);
	void getBoxStencilHandler(const ClientRequest& request, ClientResponse& response);
	void getModelStencilListHandler(const ClientRequest& request, ClientResponse& response);
	void getModelStencilHandler(const ClientRequest& request, ClientResponse& response);
	void getModelStencilRenderGeometryHandler(const ClientRequest& request, ClientResponse& response);

	void getSpatialAnchorListHandler(const ClientRequest& request, ClientResponse& response);
	void getSpatialAnchorInfoHandler(const ClientRequest& request, ClientResponse& response);
	void findSpatialAnchorInfoByNameHandler(const ClientRequest& request, ClientResponse& response);

	// VRManager Callbacks
	void publishVRDeviceListChanged();
	void publishVRDevicePoses(int64_t newFrameIndex);

private:
	static MikanServer* m_instance;

	std::vector<CommonScriptContextWeakPtr> m_scriptContexts;
	std::map<std::string, MikanClientConnectionStatePtr> m_clientConnections;
	class IInterprocessMessageServer* m_messageServer;
};

#endif // MIKAN_SERVER_H