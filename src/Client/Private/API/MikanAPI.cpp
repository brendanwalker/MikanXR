#include "MikanAPI.h"
#include "MikanCoreCAPI.h"
#include "MikanRequestManager.h"
#include "MikanEventManager.h"
#include "MikanVideoSourceAPI.h"
#include "MikanVRDeviceAPI.h"
#include "MikanScriptAPI.h"
#include "MikanStencilAPI.h"
#include "MikanSpatialAnchorAPI.h"

#include "MikanAPITypes_json.h"
#include "MikanEventTypes_json.h"

class MikanAPI : public IMikanAPI
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

	// Initialize the Mikan API
	virtual MikanResult init(MikanLogLevel min_log_level, MikanLogCallback log_callback) override
	{
		MikanResult result = Mikan_Initialize(min_log_level, log_callback);
		if (result != MikanResult_Success)
		{
			return result;
		}

		result = m_requestManager->init();
		if (result != MikanResult_Success)
		{
			return result;
		}

		return MikanResult_Success;
	}

	virtual bool getIsInitialized() override
	{
		return Mikan_GetIsInitialized();
	}

	virtual int getCoreSDKVersion() const override
	{
		return Mikan_GetCoreSDKVersion();
	}

	// Sub API accessors
	virtual IMikanVideoSourceAPI* getVideoSourceAPI() const override { return m_videoSourceAPI.get(); }
	virtual IMikanVRDeviceAPI* getVRDeviceAPI() const override { return m_vrDeviceAPI.get(); }
	virtual IMikanScriptAPI* getScriptAPI() const override { return m_scriptAPI.get(); }
	virtual IMikanStencilAPI* getStencilAPI() const override { return m_stencilAPI.get(); }
	virtual IMikanSpatialAnchorAPI* getSpatialAnchorAPI() const override { return m_spatialAnchorAPI.get(); }

	// Set client properties before calling connect
	virtual MikanResult setClientInfo(const MikanClientInfo& clientInfo) override
	{
		json clientInfoJson = clientInfo;
		const std::string clientInfoString = clientInfoJson.dump();

		return Mikan_SetClientProperty("clientInfo", clientInfoString.c_str());
	}

	virtual MikanResult setGraphicsDeviceInterface(
		MikanClientGraphicsApi api, 
		void* graphicsDeviceInterface) override
	{
		return Mikan_SetGraphicsDeviceInterface(api, graphicsDeviceInterface);
	}

	virtual MikanResult getGraphicsDeviceInterface(
		MikanClientGraphicsApi api, 
		void** outGraphicsDeviceInterface) override
	{
		return Mikan_GetGraphicsDeviceInterface(api, outGraphicsDeviceInterface);
	}

	virtual MikanResponseFuture allocateRenderTargetBuffers(
		const MikanRenderTargetDescriptor& descriptor) override
	{
		MikanRequestID requestId = INVALID_MIKAN_ID;
		MikanResult result = Mikan_AllocateRenderTargetBuffers(&descriptor, &requestId);

		return m_requestManager->addResponseHandler(requestId, result);
	}

	virtual MikanResult publishRenderTargetTexture(
		void* apiTexturePtr, 
		MikanClientFrameRendered& frameInfo) override
	{
		return Mikan_PublishRenderTargetTexture(apiTexturePtr, &frameInfo);
	}

	virtual MikanResponseFuture freeRenderTargetBuffers() override
	{
		MikanRequestID requestId = INVALID_MIKAN_ID;
		MikanResult result = Mikan_FreeRenderTargetBuffers(&requestId);

		return m_requestManager->addResponseHandler(requestId, result);
	}

	virtual MikanResult connect(
		const std::string& host, 
		const std::string& port) override
	{
		return Mikan_Connect(host.c_str(), port.c_str());
	}

	virtual bool getIsConnected() override
	{
		return Mikan_GetIsConnected();
	}

	virtual MikanResult fetchNextEvent(MikanEventPtr& out_event) override
	{
		return m_eventManager->fetchNextEvent(out_event);
	}

	virtual MikanResult disconnect() override
	{
		return Mikan_Disconnect();
	}

	virtual MikanResult shutdown() override
	{
		return Mikan_Shutdown();
	}

private:
	std::unique_ptr<MikanRequestManager> m_requestManager;
	std::unique_ptr<MikanEventManager> m_eventManager;

	std::unique_ptr<IMikanVideoSourceAPI> m_videoSourceAPI;
	std::unique_ptr<IMikanVRDeviceAPI> m_vrDeviceAPI;
	std::unique_ptr<IMikanScriptAPI> m_scriptAPI;
	std::unique_ptr<IMikanStencilAPI> m_stencilAPI;
	std::unique_ptr<IMikanSpatialAnchorAPI> m_spatialAnchorAPI;
};

// -- Mikan API Factory -----
IMikanAPIPtr IMikanAPI::createMikanAPI()
{
	return std::make_shared<MikanAPI>();
}