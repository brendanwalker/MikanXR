#pragma once

//-- includes -----
#include "CommonConfigFwd.h"
#include "InterprocessMessageServerInterface.h"
#include "ScriptingFwd.h"
#include "MikanAPITypes.h"
#include "MikanClientTypes.h"
#include "MikanClientEvents.h"
#include "MikanCameraEvents.h"
#include "MikanScriptEvents.h"
#include "MikanStencilEvents.h"
#include "MikanSpatialAnchorEvents.h"
#include "MikanVideoSourceEvents.h"
#include "MikanVRDeviceEvents.h"
#include "MulticastDelegate.h"
#include "ObjectSystemConfigFwd.h"
#include "glm/ext/matrix_float4x4.hpp"
#include "stdint.h"

#include <map>
#include <memory>

class MikanClientConnectionState;
using MikanClientConnectionStatePtr= std::shared_ptr<MikanClientConnectionState>;
using MikanClientConnectionStateConstPtr= std::shared_ptr<const MikanClientConnectionState>;

//-- definitions -----
class MikanServer
{
public:
	MikanServer();
	virtual ~MikanServer();

	static MikanServer* getInstance() { return m_instance; }
	ProjectConfigPtr getProjectConfig() const;
	inline class IInterprocessMessageServer* getMessageServer() { return m_messageServer; }
	inline class CameraRequestHandler* getCameraRequestHandler() const { return m_cameraRequestHandler; }
	inline class ScriptRequestHandler* getScriptRequestHandler() const { return m_scriptRequestHandler; }
	inline class RemoteControlManager* getRemoteControlManager() const { return m_remoteControlManager; }
	inline class RenderTargetRequestHandler* getRenderTargetRequestHandler() const { return m_renderTargetRequestHandler; }
	inline class StencilRequestHandler* getStencilRequestHandler() const { return m_stencilRequestHandler; }
	inline class TextureSourceRequestHandler* getTextureSourceRequestHandler() const { return m_textureSourceRequestHandler; }
	inline class VideoSourceRequestHandler* getVideoSourceRequestHandler() const { return m_videoSourceRequestHandler; }

	bool startup(class MainWindow* mainWindow);
	void update();
	void shutdown();

	void publishMikanJsonEvent(const std::string& mikanJsonEvent);

	MikanClientConnectionStatePtr getConnectedClientState(const std::string& connectionId) const;
	void getConnectedClientStateList(std::vector<MikanClientConnectionStateConstPtr>& outClientList) const;

	MulticastDelegate<void(const std::string& clientId, const MikanClientInfo& clientInfo) > OnClientInitialized;
	MulticastDelegate<void(const std::string& clientId)> OnClientDisposed;

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

private:
	static MikanServer* m_instance;

	class IInterprocessMessageServer* m_messageServer;

	class AnchorRequestHandler* m_anchorRequestHandler;
	class CameraRequestHandler* m_cameraRequestHandler;
	class RemoteControlManager* m_remoteControlManager;
	class RenderTargetRequestHandler* m_renderTargetRequestHandler;
	class ScriptRequestHandler* m_scriptRequestHandler;
	class StencilRequestHandler* m_stencilRequestHandler;
	class TextureSourceRequestHandler* m_textureSourceRequestHandler;
	class VideoSourceRequestHandler* m_videoSourceRequestHandler;
	class VRDeviceRequestHandler* m_vrDeviceRequestHandler;

	ProjectConfigWeakPtr m_projectConfig;

	std::map<std::string, MikanClientConnectionStatePtr> m_clientConnections;
};