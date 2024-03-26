#pragma once

#include "Logger.h"
#include "MikanCoreCAPI.h"
#include "MikanJsonUtils.h"
#include "MikanEventTypes.h"
#include "MikanEventManager.h"
#include "MikanRequestManager.h"
#include "MikanVideoSourceTypes.h"

#include <memory>
#include <string>

// -- Video Source API -----
class MikanVideoSourceAPI
{
public:
	MikanVideoSourceAPI()= delete;
	MikanVideoSourceAPI(MikanRequestManager *requestManager) 
		: m_requestManager(requestManager) 
	{
		m_requestManager->addResponseFactory<MikanVideoSourceIntrinsics>();
		m_requestManager->addResponseFactory<MikanVideoSourceMode>();
		m_requestManager->addResponseFactory<MikanVideoSourceAttachmentInfo>();
	}

	inline static const std::string k_getVideoSourceIntrinsics = "getVideoSourceIntrinsics";
	MikanResponseFuture getVideoSourceIntrinsics() // returns MikanVideoSourceIntrinsics
	{
		return m_requestManager->sendRequest(k_getVideoSourceIntrinsics);
	}
	
	inline static const std::string k_getVideoSourceMode = "getVideoSourceMode";
	MikanResponseFuture getVideoSourceMode() // returns MikanVideoSourceMode
	{
		return m_requestManager->sendRequest(k_getVideoSourceMode);
	}

	inline static const std::string k_getVideoSourceAttachment = "getVideoSourceAttachment";
	MikanResponseFuture getVideoSourceAttachment() // returns MikanVideoSourceAttachmentInfo
	{
		return m_requestManager->sendRequest(k_getVideoSourceAttachment);
	}

private:
	MikanRequestManager* m_requestManager= nullptr;
};

// -- VR Device API -----
class MikanVRDeviceAPI
{
public:
	MikanVRDeviceAPI() = delete;
	MikanVRDeviceAPI(MikanRequestManager* requestManager)
		: m_requestManager(requestManager)
	{
		m_requestManager->addResponseFactory<MikanVRDeviceList>();
		m_requestManager->addResponseFactory<MikanVRDeviceInfo>();
	}

	inline static const std::string k_getVRDeviceList = "getVRDeviceList";
	MikanResponseFuture getVRDeviceList() // returns MikanVRDeviceList
	{
		return m_requestManager->sendRequest(k_getVRDeviceList);
	}

	inline static const std::string k_getVRDeviceInfo = "getVRDeviceInfo";
	MikanResponseFuture getVRDeviceInfo(MikanVRDeviceID deviceId) // returns MikanVRDeviceInfo
	{
		return m_requestManager->sendRequestWithPayload<int>(k_getVRDeviceInfo, deviceId);
	}

	inline static const std::string k_subscribeToVRDevicePoseUpdates = "subscribeToVRDevicePoseUpdates";
	MikanResponseFuture subscribeToVRDevicePoseUpdates(MikanVRDeviceID deviceId) // returns MikanResponse
	{
		return m_requestManager->sendRequestWithPayload<int>(k_subscribeToVRDevicePoseUpdates, deviceId);
	}

	inline static const std::string k_unsubscribeFromVRDevicePoseUpdates = "unsubscribeFromVRDevicePoseUpdates";
	MikanResponseFuture unsubscribeFromVRDevicePoseUpdates(MikanVRDeviceID deviceId) // returns MikanResponse
	{
		return m_requestManager->sendRequestWithPayload<int>(k_unsubscribeFromVRDevicePoseUpdates, deviceId);
	}

private:
	MikanRequestManager* m_requestManager= nullptr;
};

// -- Script API -----
class MikanScriptAPI
{
public:
	MikanScriptAPI() = delete;
	MikanScriptAPI(MikanRequestManager* requestManager)
		: m_requestManager(requestManager)
	{
	}

	inline static const std::string k_sendScriptMessage = "invokeScriptMessageHandler";
	MikanResponseFuture sendScriptMessage(const std::string& mesg) // returns MikanResult
	{
		return m_requestManager->sendRequestWithPayload<std::string>(k_sendScriptMessage, mesg);
	}

private:
	MikanRequestManager* m_requestManager= nullptr;
};

class MikanStencilAPI
{
public:
	MikanStencilAPI() = delete;
	MikanStencilAPI(MikanRequestManager* requestManager)
		: m_requestManager(requestManager)
	{
		m_requestManager->addResponseFactory<MikanStencilList>();
		m_requestManager->addResponseFactory<MikanStencilQuad>();
		m_requestManager->addResponseFactory<MikanStencilBox>();
		m_requestManager->addResponseFactory<MikanStencilModel>();
	}

	inline static const std::string k_getStencilList = "getStencilList";
	MikanResponseFuture getStencilList() // returns MikanStencilList
	{
		return m_requestManager->sendRequest(k_getStencilList);
	}

	inline static const std::string k_getQuadStencil = "getQuadStencil";
	MikanResponseFuture getQuadStencil(MikanStencilID stencilId) // returns MikanStencilQuad
	{
		return m_requestManager->sendRequestWithPayload<int>(k_getQuadStencil, stencilId);
	}

	inline static const std::string k_getBoxStencil = "getBoxStencil";
	MikanResponseFuture getBoxStencil(MikanStencilID stencilId) // returns MikanStencilBox
	{
		return m_requestManager->sendRequestWithPayload<int>(k_getBoxStencil, stencilId);
	}

	inline static const std::string k_getModelStencil = "getModelStencil";
	MikanResponseFuture getModelStencil(MikanStencilID stencilId) // returns MikanStencilModel
	{
		return m_requestManager->sendRequestWithPayload<int>(k_getModelStencil, stencilId);
	}

private:
	MikanRequestManager* m_requestManager= nullptr;
};

// -- Spatial Anchor API -----
class MikanSpatialAnchorAPI
{
public:
	MikanSpatialAnchorAPI() = delete;
	MikanSpatialAnchorAPI(MikanRequestManager* requestManager)
		: m_requestManager(requestManager)
	{
		m_requestManager->addResponseFactory<MikanSpatialAnchorList>();
		m_requestManager->addResponseFactory<MikanSpatialAnchorInfo>();
	}

	inline static const std::string k_getSpatialAnchorList = "getSpatialAnchorList";
	MikanResponseFuture getSpatialAnchorList() // returns MikanSpatialAnchorList
	{
		return m_requestManager->sendRequest(k_getSpatialAnchorList);
	}

	inline static const std::string k_getSpatialAnchorInfo = "getSpatialAnchorInfo";
	MikanResponseFuture getSpatialAnchorInfo(MikanSpatialAnchorID anchorId) // returns MikanSpatialAnchorInfo
	{
		return m_requestManager->sendRequestWithPayload<int>(k_getSpatialAnchorInfo, anchorId);
	}

	inline static const std::string k_findSpatialAnchorInfoByName = "findSpatialAnchorInfoByName";
	MikanResponseFuture findSpatialAnchorInfoByName(const std::string& anchorName) // returns MikanSpatialAnchorInfo
	{
		return m_requestManager->sendRequestWithPayload<std::string>(k_findSpatialAnchorInfoByName, anchorName);
	}

private:
	MikanRequestManager* m_requestManager= nullptr;
};

// -- Mikan API -----
class MikanAPI
{
public:
	MikanAPI()
		: m_requestManager(std::make_unique<MikanRequestManager>())
		, m_eventManager(std::make_unique<MikanEventManager>())
		, m_videoSourceAPI(std::make_unique<MikanVideoSourceAPI>(m_requestManager.get()))
		, m_vrDeviceAPI(std::make_unique<MikanVRDeviceAPI>(m_requestManager.get()))
		, m_scriptAPI(std::make_unique<MikanScriptAPI>(m_requestManager.get()))
		, m_stencilAPI(std::make_unique<MikanStencilAPI>(m_requestManager.get()))
		, m_spatialAnchorAPI(std::make_unique<MikanSpatialAnchorAPI>(m_requestManager.get()))
	{
		// Register base response types (child API classes will register their own types)
		m_requestManager->addResponseFactory<MikanResponse>();

		// Register all event types
		m_eventManager->addEventFactory<MikanConnectedEvent>();
		m_eventManager->addEventFactory<MikanDisconnectedEvent>();
		m_eventManager->addEventFactory<MikanVideoSourceOpenedEvent>();
		m_eventManager->addEventFactory<MikanVideoSourceClosedEvent>();
		m_eventManager->addEventFactory<MikanVideoSourceNewFrameEvent>();
		m_eventManager->addEventFactory<MikanVideoSourceAttachmentChangedEvent>();
		m_eventManager->addEventFactory<MikanVideoSourceIntrinsicsChangedEvent>();
		m_eventManager->addEventFactory<MikanVideoSourceModeChangedEvent>();
		m_eventManager->addEventFactory<MikanVRDevicePoseUpdateEvent>();
		m_eventManager->addEventFactory<MikanVRDeviceListUpdateEvent>();
		m_eventManager->addEventFactory<MikanAnchorPoseUpdateEvent>();
		m_eventManager->addEventFactory<MikanAnchorListUpdateEvent>();
		m_eventManager->addEventFactory<MikanScriptMessagePostedEvent>();
	}
	virtual ~MikanAPI() 
	{
		shutdown();
	}

	// Sub API accessors
	inline MikanVideoSourceAPI* getVideoSourceAPI() const { return m_videoSourceAPI.get(); }
	inline MikanVRDeviceAPI* getVRDeviceAPI() const { return m_vrDeviceAPI.get(); }
	inline MikanScriptAPI* getScriptAPI() const { return m_scriptAPI.get(); }
	inline MikanStencilAPI* getStencilAPI() const { return m_stencilAPI.get(); }
	inline MikanSpatialAnchorAPI* getSpatialAnchorAPI() const { return m_spatialAnchorAPI.get(); }

	// Initialize the Mikan API
	MikanResult init(MikanLogLevel min_log_level, MikanLogCallback log_callback)
	{
		MikanResult result= Mikan_Initialize(min_log_level, log_callback);
		if (result != MikanResult_Success)
		{
			return result;
		}

		result= m_requestManager->init();
		if (result != MikanResult_Success)
		{			
			return result;
		}

		return MikanResult_Success;
	}

	bool getIsInitialized()
	{
		return Mikan_GetIsInitialized();
	}

	// Set client properties before calling connect
	MikanResult setClientInfo(const MikanClientInfo& clientInfo)
	{
		json clientInfoJson = clientInfo;
		const std::string clientInfoString= clientInfoJson.dump();

		return Mikan_SetClientProperty("clientInfo", clientInfoString.c_str());
	}

	MikanResult setGraphicsDeviceInterface(MikanClientGraphicsApi api, void* graphicsDeviceInterface)
	{
		return Mikan_SetGraphicsDeviceInterface(api, graphicsDeviceInterface);
	}

	MikanResult getGraphicsDeviceInterface(MikanClientGraphicsApi api, void** outGraphicsDeviceInterface)
	{
		return Mikan_GetGraphicsDeviceInterface(api, outGraphicsDeviceInterface);
	}

	MikanResponseFuture allocateRenderTargetBuffers(const MikanRenderTargetDescriptor& descriptor)
	{
		MikanRequestID requestId = INVALID_MIKAN_ID;
		MikanResult result= Mikan_AllocateRenderTargetBuffers(&descriptor, &requestId);

		return m_requestManager->addResponseHandler(requestId, result);
	}

	MikanResult publishRenderTargetTexture(void* apiTexturePtr, MikanClientFrameRendered& frameInfo)
	{
		return Mikan_PublishRenderTargetTexture(apiTexturePtr, &frameInfo);
	}

	MikanResponseFuture freeRenderTargetBuffers()
	{
		MikanRequestID requestId = INVALID_MIKAN_ID;
		MikanResult result= Mikan_FreeRenderTargetBuffers(&requestId);

		return m_requestManager->addResponseHandler(requestId, result);
	}

	MikanResult connect(const std::string& host="", const std::string& port="")
	{
		return Mikan_Connect(host.c_str(), port.c_str());
	}

	bool getIsConnected()
	{
		return Mikan_GetIsConnected();
	}

	MikanResult fetchNextEvent(MikanEventPtr& out_event)
	{
		return m_eventManager->fetchNextEvent(out_event);
	}
	
	MikanResult disconnect()
	{
		return Mikan_Disconnect();
	}

	MikanResult shutdown()
	{
		return Mikan_Shutdown();
	}

private:
	std::unique_ptr<MikanRequestManager> m_requestManager;
	std::unique_ptr<MikanEventManager> m_eventManager;

	std::unique_ptr<MikanVideoSourceAPI> m_videoSourceAPI;
	std::unique_ptr<MikanVRDeviceAPI> m_vrDeviceAPI;
	std::unique_ptr<MikanScriptAPI> m_scriptAPI;
	std::unique_ptr<MikanStencilAPI> m_stencilAPI;
	std::unique_ptr<MikanSpatialAnchorAPI> m_spatialAnchorAPI;
};