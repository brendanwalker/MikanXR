#include "MikanAPI.h"
#include "MikanCoreCAPI.h"
#include "MikanRequestManager.h"
#include "MikanRenderTargetAPI.h"
#include "MikanEventManager.h"
#include "JsonSerializer.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanAPITypes.rfks.h"
#include "MikanRenderTargetRequests.rfks.h"
#include "MikanClientEvents.rfks.h"
#include "MikanMathTypes.rfks.h"
#include "MikanScriptEvents.rfks.h"
#include "MikanScriptTypes.rfks.h"
#include "MikanScriptRequests.rfks.h"
#include "MikanSpatialAnchorEvents.rfks.h"
#include "MikanSpatialAnchorTypes.rfks.h"
#include "MikanSpatialAnchorRequests.rfks.h"
#include "MikanStencilEvents.rfks.h"
#include "MikanStencilTypes.rfks.h"
#include "MikanStencilRequests.rfks.h"
#include "MikanVideoSourceEvents.rfks.h"
#include "MikanVideoSourceTypes.rfks.h"
#include "MikanVideoSourceRequests.rfks.h"
#include "MikanVRDeviceEvents.rfks.h"
#include "MikanVRDeviceTypes.rfks.h"
#include "MikanVRDeviceRequests.rfks.h"
#endif // MIKANAPI_REFLECTION_ENABLED

class MikanAPI : public IMikanAPI
{
public:
	MikanAPI()
		: m_context(nullptr)
		, m_requestManager(std::make_unique<MikanRequestManager>())
		, m_eventManager(std::make_unique<MikanEventManager>())
		, m_renderTargetAPI(std::make_unique<MikanRenderTargetAPI>(m_requestManager.get()))
	{
	}

	virtual ~MikanAPI()
	{
		shutdown();
	}

	// Initialize the Mikan API
	virtual MikanResult init(MikanLogLevel min_log_level, MikanLogCallback log_callback) override
	{
		MikanResult result = Mikan_Initialize(min_log_level, log_callback, &m_context);
		if (result != MikanResult_Success)
		{
			return result;
		}

		result = m_eventManager->init(m_context);
		if (result != MikanResult_Success)
		{
			return result;
		}

		result = m_requestManager->init(m_context);
		if (result != MikanResult_Success)
		{
			return result;
		}

		return MikanResult_Success;
	}

	virtual bool getIsInitialized() override
	{
		return Mikan_GetIsInitialized(m_context);
	}

	virtual int getCoreSDKVersion() const override
	{
		return Mikan_GetCoreSDKVersion();
	}

	virtual std::string getClientUniqueID() const override
	{
		return Mikan_GetClientUniqueID(m_context);
	}

	// Send a request to the Mikan API
	virtual MikanResponseFuture sendRequest(const MikanRequest& request) override
	{
		MikanResponseFuture responseFuture;
		if (!m_renderTargetAPI->tryProcessRequest(request, responseFuture))
		{
			responseFuture= m_requestManager->sendRequest(request);
		}

		return responseFuture;
	}

	// Set client properties before calling connect
	virtual MikanResult setGraphicsDeviceInterface(MikanClientGraphicsApi api, void* graphicsDeviceInterface) override
	{
		return m_renderTargetAPI->setGraphicsDeviceInterface(api, graphicsDeviceInterface);
	}
	virtual MikanResult getGraphicsDeviceInterface(MikanClientGraphicsApi api, void** outGraphicsDeviceInterface) override
	{
		return m_renderTargetAPI->getGraphicsDeviceInterface(api, outGraphicsDeviceInterface);
	}

	virtual MikanResult setClientInfo(const MikanClientInfo& inClientInfo) override
	{
		MikanClientInfo clientInfo = inClientInfo;
		clientInfo.mikanCoreSdkVersion = getCoreSDKVersion();
		clientInfo.clientId= getClientUniqueID();

		std::string clientInfoString;
		Serialization::serializeToJsonString(clientInfo, clientInfoString);

		return Mikan_SetClientInfo(m_context, clientInfoString.c_str());
	}

	virtual MikanResult connect(
		const std::string& host, 
		const std::string& port) override
	{
		return Mikan_Connect(m_context, host.c_str(), port.c_str());
	}

	virtual bool getIsConnected() override
	{
		return Mikan_GetIsConnected(m_context);
	}

	virtual MikanResult fetchNextEvent(MikanEventPtr& out_event) override
	{
		return m_eventManager->fetchNextEvent(out_event);
	}

	virtual MikanResult disconnect() override
	{
		return Mikan_Disconnect(m_context);
	}

	virtual MikanResult shutdown() override
	{
		return Mikan_Shutdown(&m_context);
	}

private:
	MikanContext m_context;
	std::unique_ptr<MikanRequestManager> m_requestManager;
	std::unique_ptr<MikanEventManager> m_eventManager;
	std::unique_ptr<MikanRenderTargetAPI> m_renderTargetAPI;
};

// -- Mikan API Factory -----
IMikanAPIPtr IMikanAPI::createMikanAPI()
{
	return std::make_shared<MikanAPI>();
}