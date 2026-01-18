#include "MikanAPI.h"
#include "MikanCoreCAPI.h"
#include "MikanRequestManager.h"
#include "MikanRenderTargetAPI.h"
#include "MikanEventManager.h"
#include "JsonSerializer.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanAnchorTypes.rfks.h"

#include "MikanAPITypes.rfks.h"

#include "MikanCameraEvents.rfks.h"
#include "MikanCameraTypes.rfks.h"

#include "MikanClientEvents.rfks.h"
#include "MikanClientTypes.rfks.h"
#include "MikanClientRequests.rfks.h"

#include "MikanComponentTypes.rfks.h"

#include "MikanCompositorTypes.rfks.h"

#include "MikanMarkerTypes.rfks.h"

#include "MikanMathTypes.rfks.h"

#include "MikanPropertyEvents.rfks.h"
#include "MikanPropertyTypes.rfks.h"
#include "MikanPropertyRequests.rfks.h"

#include "MikanRemoteControlEvents.rfks.h"
#include "MikanRemoteControlTypes.rfks.h"
#include "MikanRemoteControlRequests.rfks.h"

#include "MikanRenderTargetRequests.rfks.h"

#include "MikanSceneTypes.rfks.h"

#include "MikanScriptEvents.rfks.h"
#include "MikanScriptTypes.rfks.h"
#include "MikanScriptRequests.rfks.h"

#include "MikanStageTypes.rfks.h"

#include "MikanStencilTypes.rfks.h"
#include "MikanStencilRequests.rfks.h"

#include "MikanTextureSourceEvents.rfks.h"
#include "MikanTextureSourceTypes.rfks.h"

#include "MikanTrackingMountTypes.rfks.h"

#include "MikanTrackingVolumeTypes.rfks.h"

#include "MikanTransformTypes.rfks.h"

#include "MikanVariantTypes.rfks.h"

#include "MikanVideoSourceEvents.rfks.h"
#include "MikanVideoSourceRequests.rfks.h"
#include "MikanVideoSourceTypes.rfks.h"

#include "MikanVRDeviceTypes.rfks.h"
#endif // MIKANAPI_REFLECTION_ENABLED

#include "MikanAnchorTypes.h"
#include "MikanCameraTypes.h"
#include "MikanCompositorTypes.h"
#include "MikanMarkerTypes.h"
#include "MikanSceneTypes.h"
#include "MikanStageTypes.h"
#include "MikanStencilTypes.h"
#include "MikanTextureSourceTypes.h"
#include "MikanTrackingMountTypes.h"
#include "MikanTrackingVolumeTypes.h"
#include "MikanVideoSourceTypes.h"
#include "MikanVRDeviceTypes.h"

// Static component class name and owner system name definitions
const char* MikanAnchorComponentValues::k_componentClassName = "AnchorComponent";
const char* MikanAnchorComponentValues::k_ownerSystemName = "AnchorObjectSystem";

const char* MikanCameraComponentValues::k_componentClassName = "CameraComponent";
const char* MikanCameraComponentValues::k_ownerSystemName = "CameraObjectSystem";

const char* MikanCompositorComponentValues::k_componentClassName = "CompositorComponent";
const char* MikanCompositorComponentValues::k_ownerSystemName = "CompositorObjectSystem";

const char* MikanMarkerComponentValues::k_componentClassName = "MarkerComponent";
const char* MikanMarkerComponentValues::k_ownerSystemName = "MarkerObjectSystem";

const char* MikanSceneComponentValues::k_componentClassName = "SceneComponent";
const char* MikanSceneComponentValues::k_ownerSystemName = "SceneObjectSystem";

const char* MikanStageComponentValues::k_componentClassName = "StageComponent";
const char* MikanStageComponentValues::k_ownerSystemName = "StageObjectSystem";

const char* MikanQuadStencilComponentValues::k_componentClassName = "QuadStencilComponent";
const char* MikanQuadStencilComponentValues::k_ownerSystemName = "QuadStencilObjectSystem";

const char* MikanBoxStencilComponentValues::k_componentClassName = "BoxStencilComponent";
const char* MikanBoxStencilComponentValues::k_ownerSystemName = "BoxStencilObjectSystem";

const char* MikanModelStencilComponentValues::k_componentClassName = "ModelStencilComponent";
const char* MikanModelStencilComponentValues::k_ownerSystemName = "ModelStencilObjectSystem";

const char* MikanMarkerTrackingVolumeComponentValues::k_componentClassName = "MarkerTrackingVolumeComponent";
const char* MikanMarkerTrackingVolumeComponentValues::k_ownerSystemName = "MarkerTrackingVolumeObjectSystem";

const char* MikanVRTrackingVolumeComponentValues::k_componentClassName = "VRTrackingVolumeComponent";
const char* MikanVRTrackingVolumeComponentValues::k_ownerSystemName = "VRTrackingVolumeObjectSystem";

const char* MikanTrackingMountComponentValues::k_componentClassName = "TrackingMountComponent";
const char* MikanTrackingMountComponentValues::k_ownerSystemName = "TrackingMountObjectSystem";

const char* MikanClientTextureSourceValues::k_componentClassName = "ClientTextureSourceComponent";
const char* MikanClientTextureSourceValues::k_ownerSystemName = "ClientTextureSourceSystem";

const char* MikanSpoutTextureSourceValues::k_componentClassName = "SpoutTextureSourceComponent";
const char* MikanSpoutTextureSourceValues::k_ownerSystemName = "SpoutTextureSourceSystem";

const char* MikanNetworkVideoSourceValues::k_componentClassName = "NetworkVideoSourceComponent";
const char* MikanNetworkVideoSourceValues::k_ownerSystemName = "NetworkVideoSourceSystem";

const char* MikanUSBVideoSourceValues::k_componentClassName = "USBVideoSourceComponent";
const char* MikanUSBVideoSourceValues::k_ownerSystemName = "USBVideoSourceSystem";

const char* MikanVRDeviceComponentValues::k_componentClassName = "VRDeviceComponent";
const char* MikanVRDeviceComponentValues::k_ownerSystemName = "VRObjectSystem";

// -- MikanAPI Implementation -----
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
	virtual MikanAPIResult init(MikanLogLevel min_log_level, MikanLogCallback log_callback) override
	{
		MikanAPIResult result = (MikanAPIResult)Mikan_Initialize(min_log_level, log_callback, &m_context);
		if (result != MikanAPIResult::Success)
		{
			return result;
		}

		result = m_eventManager->init(m_context);
		if (result != MikanAPIResult::Success)
		{
			return result;
		}

		result = m_requestManager->init(m_context);
		if (result != MikanAPIResult::Success)
		{
			return result;
		}

		return MikanAPIResult::Success;
	}

	virtual bool getIsInitialized() override
	{
		return Mikan_GetIsInitialized(m_context);
	}

	virtual int getClientAPIVersion() const override
	{
		return MikanConstants_ClientAPIVersion;
	}

	virtual std::string getClientUniqueID() const override
	{
		return Mikan_GetClientUniqueID(m_context);
	}

	virtual MikanClientInfo allocateClientInfo() const override
	{
		MikanClientInfo clientInfo = {};

		// Stamp the request with the core sdk version and client id
		clientInfo.clientId = getClientUniqueID();

		return clientInfo;
	}

	// Send a request to the Mikan API
	virtual MikanResponseFuture sendRequest(MikanRequest& request) override
	{
		MikanResponseFuture responseFuture= m_renderTargetAPI->tryProcessRequest(request);
		if (responseFuture.isValid())
		{
			return responseFuture;
		}

		return m_requestManager->sendRequest(request);
	}

	virtual MikanAPIResult cancelRequest(const MikanRequestID& requestId) override
	{
		return m_requestManager->cancelRequest(requestId);
	}

	// Set client properties before calling connect
	virtual MikanAPIResult setGraphicsDeviceInterface(MikanClientGraphicsApi api, void* graphicsDeviceInterface) override
	{
		return m_renderTargetAPI->setGraphicsDeviceInterface(api, graphicsDeviceInterface);
	}
	virtual MikanAPIResult getGraphicsDeviceInterface(MikanClientGraphicsApi api, void** outGraphicsDeviceInterface) override
	{
		return m_renderTargetAPI->getGraphicsDeviceInterface(api, outGraphicsDeviceInterface);
	}

	virtual MikanAPIResult connect() override
	{
		return (MikanAPIResult)Mikan_Connect(m_context, "", "");
	}

	virtual MikanAPIResult connect(
		const std::string& host, 
		const std::string& port) override
	{
		return (MikanAPIResult)Mikan_Connect(m_context, host.c_str(), port.c_str());
	}

	virtual bool getIsConnected() override
	{
		return Mikan_GetIsConnected(m_context);
	}

	virtual MikanAPIResult fetchNextEvent(MikanEventPtr& out_event) override
	{
		return m_eventManager->fetchNextEvent(out_event);
	}

	virtual MikanAPIResult disconnect() override
	{
		return (MikanAPIResult)Mikan_Disconnect(m_context, 0, "");
	}

	virtual MikanAPIResult disconnect(uint16_t code, const std::string& reason) override
	{
		return (MikanAPIResult)Mikan_Disconnect(m_context, code, reason.c_str());
	}

	virtual MikanAPIResult shutdown() override
	{
		auto result= (MikanAPIResult)Mikan_Shutdown(m_context);
		m_context= nullptr;
		return result;
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