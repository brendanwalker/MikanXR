//-- includes -----
#include "App.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "BoxStencilComponent.h"
#include "CommonScriptContext.h"
#include "InterprocessRenderTargetReader.h"
#include "InterprocessMessages.h"
#include "MathTypeConversion.h"
#include "Logger.h"
#include "MikanServer.h"
#include "ModelStencilComponent.h"
#include "ProfileConfig.h"
#include "QuadStencilComponent.h"
#include "SerializationUtils.h"
#include "StencilObjectSystemConfig.h"
#include "StencilObjectSystem.h"
#include "VideoCapabilitiesConfig.h"
#include "VideoSourceView.h"
#include "VideoSourceManager.h"
#include "VRDeviceManager.h"
#include "VRDeviceView.h"

#include "WebsocketInterprocessMessageServer.h"

#include "MikanAPITypes_json.h"
#include "MikanCoreTypes_json.h"
#include "MikanEventTypes_json.h"
#include "MikanMathTypes_json.h"
#include "MikanScriptTypes_json.h"
#include "MikanSpatialAnchorTypes_json.h"
#include "MikanStencilTypes_json.h"
#include "MikanVideoSourceTypes_json.h"
#include "MikanVRDeviceTypes_json.h"

#include "MikanStencilTypes_binary.h"

#include <set>
#include <assert.h>

#include <easy/profiler.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#ifdef _MSC_VER
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strncpy
#endif

using namespace std::placeholders;

// -- MikanClientConnectionState -----
class MikanClientConnectionState
{
public:
	MikanClientConnectionState(
		const std::string& connectionId,
		MikanClientInfo& clientInfo,
		IInterprocessMessageServer* messageServer)
		: m_connectionId(connectionId)
		, m_messageServer(messageServer)
	{	
		m_connectionInfo.clientInfo= clientInfo;
		m_connectionInfo.renderTargetReadAccessor= new InterprocessRenderTargetReadAccessor(clientInfo.clientId);
	}

	virtual ~MikanClientConnectionState()
	{
		freeRenderTargetTextures();
		delete m_connectionInfo.renderTargetReadAccessor;
	}

	const std::string& getConnectionId() const
	{
		return m_connectionId;
	}

	const MikanClientConnectionInfo& getClientConnectionInfo() const 
	{
		return m_connectionInfo;
	}
	
	const std::string& getClientId() const 
	{
		return m_connectionInfo.clientInfo.clientId;
	}

	const MikanClientInfo& getMikanClientInfo() const 
	{
	
		return m_connectionInfo.clientInfo;
	}

	bool readRenderTargetTextures(const MikanClientFrameRendered& frameInfo)
	{
		EASY_FUNCTION();

		return m_connectionInfo.renderTargetReadAccessor->readRenderTargetTextures(frameInfo);
	}

	InterprocessRenderTargetReadAccessor* getRenderTargetReadAccessor() const
	{
		return m_connectionInfo.renderTargetReadAccessor;
	}

	MikanClientGraphicsApi getClientGraphicsAPI() const
	{
		return m_connectionInfo.renderTargetReadAccessor->getClientGraphicsAPI();
	}

	void subscribeToVRDevicePoseUpdates(MikanVRDeviceID deviceId)
	{
		m_subscribedVRDevices.insert(deviceId);
	}

	void unsubscribeFromVRDevicePoseUpdates(MikanVRDeviceID deviceId)
	{
		auto it = m_subscribedVRDevices.find(deviceId);
		if (it != m_subscribedVRDevices.end())
		{
			m_subscribedVRDevices.erase(it);
		}
	}
	
	bool allocateRenderTargetTextures(const MikanRenderTargetDescriptor& desc)
	{
		EASY_FUNCTION();

		freeRenderTargetTextures();

		if (m_connectionInfo.renderTargetReadAccessor->initialize(&desc))
		{
			MikanServer* mikanServer= MikanServer::getInstance();

			if (mikanServer->OnClientRenderTargetAllocated)
			{
				mikanServer->OnClientRenderTargetAllocated(
					m_connectionInfo.clientInfo.clientId, 
					m_connectionInfo.clientInfo, 
					m_connectionInfo.renderTargetReadAccessor);
			}

			return true;
		}

		return false;
	}

	void freeRenderTargetTextures()
	{
		EASY_FUNCTION();

		if (m_connectionInfo.hasAllocatedRenderTarget())
		{
			MikanServer* mikanServer = MikanServer::getInstance();

			MikanRenderTargetDescriptor& desc= m_connectionInfo.renderTargetReadAccessor->getRenderTargetDescriptor();
			memset(&desc, 0, sizeof(MikanRenderTargetDescriptor));

			if (mikanServer->OnClientRenderTargetReleased)
			{
				mikanServer->OnClientRenderTargetReleased(
					m_connectionInfo.clientInfo.clientId,
					m_connectionInfo.renderTargetReadAccessor);
			}
		}

		m_connectionInfo.renderTargetReadAccessor->dispose();
	}

	template <typename t_mikan_type>
	std::string mikanTypeToJsonString(const t_mikan_type& mikanType)
	{
		json j = mikanType;

		return j.dump();
	}

	template <typename t_mikan_type>
	void publishSimpleEvent()
	{
		t_mikan_type mikanEvent;
		m_messageServer->sendMessageToClient(getConnectionId(), mikanTypeToJsonString(mikanEvent));
	}

	// Scripting Events
	void publishScriptMessageEvent(const std::string& message)
	{
		MikanScriptMessagePostedEvent messageInfo;
		messageInfo.message = message;

		m_messageServer->sendMessageToClient(getConnectionId(), mikanTypeToJsonString(messageInfo));
	}

	// Video Source Events
	void publishVideoSourceOpenedEvent()
	{
		publishSimpleEvent<MikanVideoSourceOpenedEvent>();
	}

	void publishVideoSourceClosedEvent()
	{
		publishSimpleEvent<MikanVideoSourceClosedEvent>();
	}

	void publishNewVideoFrameEvent(const MikanVideoSourceNewFrameEvent& newFrameEvent)
	{
		m_messageServer->sendMessageToClient(getConnectionId(), mikanTypeToJsonString(newFrameEvent));
	}

	void publishVideoSourceAttachmentChangedEvent()
	{
		publishSimpleEvent<MikanVideoSourceAttachmentChangedEvent>();
	}

	void publishVideoSourceIntrinsicsChangedEvent()
	{
		publishSimpleEvent<MikanVideoSourceIntrinsicsChangedEvent>();
	}

	void publishVideoSourceModeChangedEvent()
	{
		publishSimpleEvent<MikanVideoSourceModeChangedEvent>();
	}

	// VR Device Events
	void publishVRDevicePoses(uint64_t newVRFrameIndex)
	{
		VRDeviceManager* vrDeviceManager= VRDeviceManager::getInstance();

		for (auto deviceId : m_subscribedVRDevices)
		{
			VRDeviceViewPtr vrDeviceView= vrDeviceManager->getVRDeviceViewById(deviceId);

			if (vrDeviceView && vrDeviceView->getIsOpen() && vrDeviceView->getIsPoseValid())
			{
				// TODO: We should provide option to select which component we want the pose updates for
				glm::mat4 xform= vrDeviceView->getDefaultComponentPose();

				// Send a pose update to the client
				MikanVRDevicePoseUpdateEvent poseUpdate;
				poseUpdate.transform= glm_mat4_to_MikanMatrix4f(xform);
				poseUpdate.device_id= deviceId;
				poseUpdate.frame= newVRFrameIndex;

				m_messageServer->sendMessageToClient(getConnectionId(), mikanTypeToJsonString(poseUpdate));
			}
		}
	}

	void publishVRDeviceListChangedEvent()
	{
		publishSimpleEvent<MikanVRDeviceListUpdateEvent>();
	}

	// Spatial Anchor Events
	void publishAnchorNameUpdatedEvent(const MikanAnchorNameUpdateEvent& newNameEvent)
	{
		m_messageServer->sendMessageToClient(getConnectionId(), mikanTypeToJsonString(newNameEvent));
	}

	void publishAnchorPoseUpdatedEvent(const MikanAnchorPoseUpdateEvent& newPoseEvent)
	{
		m_messageServer->sendMessageToClient(getConnectionId(), mikanTypeToJsonString(newPoseEvent));
	}

	// Stencil Events
	void publishStencilNameUpdatedEvent(const MikanStencilNameUpdateEvent& newNameEvent)
	{
		m_messageServer->sendMessageToClient(getConnectionId(), mikanTypeToJsonString(newNameEvent));
	}

	void publishStencilPoseUpdatedEvent(const MikanStencilPoseUpdateEvent& newPoseEvent)
	{
		m_messageServer->sendMessageToClient(getConnectionId(), mikanTypeToJsonString(newPoseEvent));
	}

private:
	std::string m_connectionId;
	IInterprocessMessageServer* m_messageServer;
	MikanClientConnectionInfo m_connectionInfo;
	std::set<MikanVRDeviceID> m_subscribedVRDevices;
};

// -- MikanClientConnectionInfo -----
bool MikanClientConnectionInfo::hasAllocatedRenderTarget() const
{
	if (renderTargetReadAccessor != nullptr)
	{
		const MikanRenderTargetDescriptor& desc = renderTargetReadAccessor->getRenderTargetDescriptor();

		return
			desc.color_buffer_type != MikanColorBuffer_NOCOLOR ||
			desc.depth_buffer_type != MikanDepthBuffer_NODEPTH;
	}

	return false;
}

// -- MikanServer -----
MikanServer* MikanServer::m_instance= nullptr;

MikanServer::MikanServer()
	: m_messageServer(new WebsocketInterprocessMessageServer())
{
	m_instance= this;
}

MikanServer::~MikanServer()
{
	delete m_messageServer;
	m_instance= nullptr;
}

// -- ClientMikanAPI System -----
template <typename t_mikan_type>
void publishSimpleEvent(std::map<std::string, MikanClientConnectionStatePtr>& clientConnections)
{
	EASY_FUNCTION();

	for (auto& connection_it : clientConnections)
	{
		connection_it.second->publishSimpleEvent<t_mikan_type>();
	}
}

bool MikanServer::startup()
{
	EASY_FUNCTION();

	if (!m_messageServer->initialize())
	{
		MIKAN_LOG_ERROR("MikanServer::startup()") << "Failed to initialize interprocess message server";
		return false;
	}

	m_messageServer->setRequestHandler("connect", std::bind(&MikanServer::connectHandler, this, _1, _2));
	m_messageServer->setRequestHandler("disconnect", std::bind(&MikanServer::disconnectHandler, this, _1, _2));
	m_messageServer->setRequestHandler("invokeScriptMessageHandler", std::bind(&MikanServer::invokeScriptMessageHandler, this, _1, _2));
	m_messageServer->setRequestHandler("getVideoSourceIntrinsics", std::bind(&MikanServer::getVideoSourceIntrinsics, this, _1, _2));
	m_messageServer->setRequestHandler("getVideoSourceMode", std::bind(&MikanServer::getVideoSourceMode, this, _1, _2));	
	m_messageServer->setRequestHandler("getVRDeviceList", std::bind(&MikanServer::getVRDeviceList, this, _1, _2));
	m_messageServer->setRequestHandler("getVideoSourceAttachment", std::bind(&MikanServer::getVideoSourceAttachment, this, _1, _2));
	m_messageServer->setRequestHandler("getVRDeviceInfo", std::bind(&MikanServer::getVRDeviceInfo, this, _1, _2));
	m_messageServer->setRequestHandler("subscribeToVRDevicePoseUpdates", std::bind(&MikanServer::subscribeToVRDevicePoseUpdates, this, _1, _2));
	m_messageServer->setRequestHandler("unsubscribeFromVRDevicePoseUpdates", std::bind(&MikanServer::unsubscribeFromVRDevicePoseUpdates, this, _1, _2));
	m_messageServer->setRequestHandler("allocateRenderTargetTextures", std::bind(&MikanServer::allocateRenderTargetTextures, this, _1, _2));
	m_messageServer->setRequestHandler("freeRenderTargetTextures", std::bind(&MikanServer::freeRenderTargetTextures, this, _1, _2));
	m_messageServer->setRequestHandler("frameRendered", std::bind(&MikanServer::frameRendered, this, _1, _2));	
	m_messageServer->setRequestHandler("getQuadStencilList", std::bind(&MikanServer::getQuadStencilList, this, _1, _2));
	m_messageServer->setRequestHandler("getQuadStencil", std::bind(&MikanServer::getQuadStencil, this, _1, _2));
	m_messageServer->setRequestHandler("getBoxStencilList", std::bind(&MikanServer::getBoxStencilList, this, _1, _2));
	m_messageServer->setRequestHandler("getBoxStencil", std::bind(&MikanServer::getBoxStencil, this, _1, _2));
	m_messageServer->setRequestHandler("getModelStencilList", std::bind(&MikanServer::getModelStencilList, this, _1, _2));
	m_messageServer->setRequestHandler("getModelStencil", std::bind(&MikanServer::getModelStencil, this, _1, _2));
	m_messageServer->setRequestHandler("getModelStencilRenderGeometry", std::bind(&MikanServer::getModelStencilRenderGeometry, this, _1, _2));	
	m_messageServer->setRequestHandler("getSpatialAnchorList", std::bind(&MikanServer::getSpatialAnchorList, this, _1, _2));
	m_messageServer->setRequestHandler("getSpatialAnchorInfo", std::bind(&MikanServer::getSpatialAnchorInfo, this, _1, _2));
	m_messageServer->setRequestHandler("findSpatialAnchorInfoByName", std::bind(&MikanServer::findSpatialAnchorInfoByName, this, _1, _2));

	VRDeviceManager::getInstance()->OnDeviceListChanged += MakeDelegate(this, &MikanServer::publishVRDeviceListChanged);
	VRDeviceManager::getInstance()->OnDevicePosesChanged += MakeDelegate(this, &MikanServer::publishVRDevicePoses);

	AnchorObjectSystem::getSystem()->getAnchorSystemConfig()->OnMarkedDirty+= 
		MakeDelegate(this, &MikanServer::handleAnchorSystemConfigChange);

	StencilObjectSystem::getSystem()->getStencilSystemConfig()->OnMarkedDirty+=
		MakeDelegate(this, &MikanServer::handleStencilSystemConfigChange);

	return true;
}

void MikanServer::update()
{
	EASY_FUNCTION();

	// Process incoming function calls from clients
	{
		EASY_BLOCK("processRemoteFunctionCalls");

		m_messageServer->processRequests();
	}
}

void MikanServer::shutdown()
{
	VRDeviceManager::getInstance()->OnDevicePosesChanged -= MakeDelegate(this, &MikanServer::publishVRDevicePoses);

	m_clientConnections.clear();
	m_messageServer->dispose();
}

// Scripting
void MikanServer::bindScriptContect(CommonScriptContextPtr scriptContext)
{
	m_scriptContexts.push_back(scriptContext);
	scriptContext->OnScriptMessage+= MakeDelegate(this, &MikanServer::publishScriptMessageEvent);
}

void MikanServer::unbindScriptContect(CommonScriptContextPtr scriptContext)
{
	for (auto it = m_scriptContexts.begin(); it < m_scriptContexts.end(); it++)
	{
		CommonScriptContextPtr scriptContext= it->lock();

		if (scriptContext == scriptContext)
		{
			m_scriptContexts.erase(it);
			scriptContext->OnScriptMessage-= MakeDelegate(this, &MikanServer::publishScriptMessageEvent);
			break;
		}
	}
}

void MikanServer::publishScriptMessageEvent(const std::string& message)
{
	for (auto& connection_it : m_clientConnections)
	{
		connection_it.second->publishScriptMessageEvent(message);
	}
}

// Video Source Events
void MikanServer::publishVideoSourceOpenedEvent()
{
	publishSimpleEvent<MikanVideoSourceOpenedEvent>(m_clientConnections);
}

void MikanServer::publishVideoSourceClosedEvent()
{
	publishSimpleEvent<MikanVideoSourceClosedEvent>(m_clientConnections);
}

void MikanServer::publishVideoSourceNewFrameEvent(const MikanVideoSourceNewFrameEvent& newFrameEvent)
{
	EASY_FUNCTION();

	for (auto& connection_it : m_clientConnections)
	{
		connection_it.second->publishNewVideoFrameEvent(newFrameEvent);
	}
}

void MikanServer::publishVideoSourceAttachmentChangedEvent()
{
	publishSimpleEvent<MikanVideoSourceAttachmentChangedEvent>(m_clientConnections);
}

void MikanServer::publishVideoSourceIntrinsicsChangedEvent()
{
	publishSimpleEvent<MikanVideoSourceIntrinsicsChangedEvent>(m_clientConnections);
}

void MikanServer::publishVideoSourceModeChangedEvent()
{
	publishSimpleEvent<MikanVideoSourceModeChangedEvent>(m_clientConnections);
}

// Spatial Anchor Events
void MikanServer::publishAnchorNameUpdatedEvent(const MikanAnchorNameUpdateEvent& newNameEvent)
{
	EASY_FUNCTION();

	for (auto& connection_it : m_clientConnections)
	{
		connection_it.second->publishAnchorNameUpdatedEvent(newNameEvent);
	}
}

void MikanServer::publishAnchorPoseUpdatedEvent(const MikanAnchorPoseUpdateEvent& newPoseEvent)
{
	EASY_FUNCTION();

	for (auto& connection_it : m_clientConnections)
	{
		connection_it.second->publishAnchorPoseUpdatedEvent(newPoseEvent);
	}
}

void MikanServer::handleAnchorSystemConfigChange(
	CommonConfigPtr configPtr,
	const class ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(MikanComponentDefinition::k_componentNamePropertyId))
	{
		AnchorDefinitionPtr anchorConfig= std::static_pointer_cast<AnchorDefinition>(configPtr);

		MikanAnchorNameUpdateEvent nameUpdateEvent;
		nameUpdateEvent.anchor_id = anchorConfig->getAnchorId();
		nameUpdateEvent.anchor_name = anchorConfig->getComponentName();

		publishAnchorNameUpdatedEvent(nameUpdateEvent);
	}
	else if (changedPropertySet.hasPropertyName(SceneComponentDefinition::k_relativePositionPropertyId) ||
		changedPropertySet.hasPropertyName(SceneComponentDefinition::k_relativeRotationPropertyId) ||
		changedPropertySet.hasPropertyName(SceneComponentDefinition::k_relativeScalePropertyId))
	{
		AnchorDefinitionPtr anchorConfig= std::static_pointer_cast<AnchorDefinition>(configPtr);

		MikanAnchorPoseUpdateEvent poseUpdateEvent;
		poseUpdateEvent.anchor_id = anchorConfig->getAnchorId();
		poseUpdateEvent.transform = glm_transform_to_MikanTransform(anchorConfig->getRelativeTransform());

		publishAnchorPoseUpdatedEvent(poseUpdateEvent);

	}
	else if (changedPropertySet.hasPropertyName(AnchorObjectSystemConfig::k_anchorListPropertyId))
	{
		publishSimpleEvent<MikanAnchorListUpdateEvent>(m_clientConnections);
	}
}

// Stencil Events
void MikanServer::publishStencilNameUpdatedEvent(const MikanStencilNameUpdateEvent& newNameEvent)
{
	EASY_FUNCTION();

	for (auto& connection_it : m_clientConnections)
	{
		connection_it.second->publishStencilNameUpdatedEvent(newNameEvent);
	}
}

void MikanServer::publishStencilPoseUpdatedEvent(const MikanStencilPoseUpdateEvent& newPoseEvent)
{
	EASY_FUNCTION();

	for (auto& connection_it : m_clientConnections)
	{
		connection_it.second->publishStencilPoseUpdatedEvent(newPoseEvent);
	}
}

void MikanServer::handleStencilSystemConfigChange(
	CommonConfigPtr configPtr,
	const class ConfigPropertyChangeSet& changedPropertySet)
{
	if (changedPropertySet.hasPropertyName(MikanComponentDefinition::k_componentNamePropertyId))
	{
		auto anchorConfig = std::static_pointer_cast<StencilComponentDefinition>(configPtr);

		MikanStencilNameUpdateEvent nameUpdateEvent;
		nameUpdateEvent.stencil_id = anchorConfig->getStencilId();
		nameUpdateEvent.stencil_name = anchorConfig->getComponentName();

		publishStencilNameUpdatedEvent(nameUpdateEvent);
	}
	else if (changedPropertySet.hasPropertyName(SceneComponentDefinition::k_relativePositionPropertyId) ||
		changedPropertySet.hasPropertyName(SceneComponentDefinition::k_relativeRotationPropertyId) ||
		changedPropertySet.hasPropertyName(SceneComponentDefinition::k_relativeScalePropertyId))
	{
		auto anchorConfig = std::static_pointer_cast<StencilComponentDefinition>(configPtr);

		MikanStencilPoseUpdateEvent poseUpdateEvent;
		poseUpdateEvent.stencil_id = anchorConfig->getStencilId();
		poseUpdateEvent.transform = glm_transform_to_MikanTransform(anchorConfig->getRelativeTransform());

		publishStencilPoseUpdatedEvent(poseUpdateEvent);
	}
	else if (changedPropertySet.hasPropertyName(StencilObjectSystemConfig::k_quadStencilListPropertyId))
	{
		publishSimpleEvent<MikanQuadStencilListUpdateEvent>(m_clientConnections);
	}
	else if (changedPropertySet.hasPropertyName(StencilObjectSystemConfig::k_boxStencilListPropertyId))
	{
		publishSimpleEvent<MikanBoxStencilListUpdateEvent>(m_clientConnections);
	}
	else if (changedPropertySet.hasPropertyName(StencilObjectSystemConfig::k_modelStencilListPropertyId))
	{
		publishSimpleEvent<MikanModelStencilListUpdateEvent>(m_clientConnections);
	}
}

// VRManager Callbacks
void MikanServer::publishVRDeviceListChanged()
{
	publishSimpleEvent<MikanVRDeviceListUpdateEvent>(m_clientConnections);
}

void MikanServer::publishVRDevicePoses(uint64_t newFrameIndex)
{
	for (auto& connection_it : m_clientConnections)
	{
		connection_it.second->publishVRDevicePoses(newFrameIndex);
	}
}

// RPC Callbacks
void MikanServer::getConnectedClientInfoList(std::vector<MikanClientConnectionInfo>& outClientList) const
{
	outClientList.clear();
	for (auto& connection_it : m_clientConnections)
	{
		outClientList.push_back(connection_it.second->getClientConnectionInfo());
	}
}

static VideoSourceViewPtr getCurrentVideoSource()
{
	ProfileConfigConstPtr profileConfig = App::getInstance()->getProfileConfig();

	return VideoSourceListIterator(profileConfig->videoSourcePath).getCurrent();
}

static VRDeviceViewPtr getCurrentCameraVRDevice()
{
	ProfileConfigConstPtr profileConfig = App::getInstance()->getProfileConfig();

	return VRDeviceManager::getInstance()->getVRDeviceViewByPath(profileConfig->cameraVRDevicePath);
}

template <typename t_mikan_type>
bool readRequestPayload(const std::string& utf8RequestString, t_mikan_type& outParameters)
{
	try
	{
		json j = json::parse(utf8RequestString);

		outParameters = j["payload"];
	}
	catch (json::exception& e)
	{
		MIKAN_LOG_ERROR("MikanServer::extractParameters") << "Failed to parse JSON: " << e.what();
		return false;
	}

	return true;
}

template <typename t_mikan_type>
void writeTypedJsonResponse(MikanRequestID requestId, t_mikan_type& result, ClientResponse& response)
{
	result.requestId= requestId;
	result.resultCode= MikanResult_Success;

	json j = result;

	response.utf8String= j.dump();
}

void writeSimpleJsonResponse(MikanRequestID requestId, MikanResult result, ClientResponse& response)
{
	// Only write a response if the request ID is valid (i.e. the client expects a response)
	if (requestId != INVALID_MIKAN_ID)
	{
		MikanResponse mikanResponse;
		mikanResponse.requestId = requestId;
		mikanResponse.resultCode = result;

		json j = mikanResponse;
		response.utf8String = j.dump();
	}
	else
	{
		response.utf8String= "";
	}
}

template <typename t_mikan_type>
void writeTypedBinaryResponse(
	MikanRequestID requestId, 
	t_mikan_type& result,
	ClientResponse& response)
{
	result.requestId= requestId;
	result.resultCode= MikanResult_Success;

	BinaryWriter writer(response.binaryData);
	to_binary(writer, result);
}

void writeSimpleBinaryResponse(MikanRequestID requestId, MikanResult result, ClientResponse& response)
{
	// Only write a response if the request ID is valid (i.e. the client expects a response)
	if (requestId != INVALID_MIKAN_ID)
	{
		MikanResponse mikanResponse;
		mikanResponse.requestId = requestId;
		mikanResponse.resultCode = result;

		BinaryWriter writer(response.binaryData);
		to_binary(writer, result);
	}
	else
	{
		response.binaryData.clear();
	}
}

void MikanServer::connectHandler(const ClientRequest& request, ClientResponse& response)
{
	MikanClientInfo clientInfo;
	if (!readRequestPayload(request.utf8RequestString, clientInfo) || 
		clientInfo.clientId.empty())
	{
		MIKAN_LOG_ERROR("connectHandler") << "Failed to parse client info";
		// TODO send error event
		return;
	}

	const std::string& connectionId = request.connectionId;
	const std::string& clientId = clientInfo.clientId;

	auto connection_it = m_clientConnections.find(connectionId);
	if (connection_it == m_clientConnections.end())
	{
		MikanClientConnectionStatePtr clientState = 
			std::make_shared<MikanClientConnectionState>(
				request.connectionId,
				clientInfo, 
				m_messageServer);

		m_clientConnections.insert({connectionId, clientState});

		if (OnClientConnected)
		{
			OnClientConnected(clientId, clientInfo);
		}

		clientState->publishSimpleEvent<MikanConnectedEvent>();
	}
	else
	{
		//TODO: send error event
		MIKAN_LOG_ERROR("connectHandler") 
			<< "Client (connectionId: " << connectionId << 
			", clientId: " << clientId << ") already connected";
	}
}

void MikanServer::disconnectHandler(const ClientRequest& request, ClientResponse& response)
{
	MikanClientInfo clientInfo;
	if (!readRequestPayload(request.utf8RequestString, clientInfo))
	{
		MIKAN_LOG_ERROR("disconnectHandler") << "Failed to parse client info";
		// TODO send error event
		return;
	}

	const std::string& connectionId = request.connectionId;

	auto connection_it = m_clientConnections.find(connectionId);
	if (connection_it != m_clientConnections.end())
	{
		const std::string& clientId = connection_it->first;
		MikanClientConnectionStatePtr clientState = connection_it->second;

		clientState->publishSimpleEvent<MikanDisconnectedEvent>();

		if (OnClientDisconnected)
		{
			OnClientDisconnected(clientId);
		}

		m_clientConnections.erase(connection_it);
	}
	else
	{
		//TODO: send error event
		MIKAN_LOG_ERROR("disconnectHandler") << "Client (connection id: " << connectionId <<") not connected";
	}
}

void MikanServer::invokeScriptMessageHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanScriptMessageInfo messageInfo;
	if (!readRequestPayload(request.utf8RequestString, messageInfo))
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_MalformedParameters, response);
		return;
	}

	// Find the first script context that cares about the message
	for (auto it = m_scriptContexts.begin(); it < m_scriptContexts.end(); it++)
	{
		CommonScriptContextPtr scriptContext = it->lock();

		if (scriptContext == scriptContext)
		{
			if (scriptContext->invokeScriptMessageHandler(messageInfo.content))
			{
				break;
			}
		}
	}

	writeSimpleJsonResponse(request.requestId, MikanResult_Success, response);
}

void MikanServer::getVideoSourceIntrinsics(
	const ClientRequest& request,
	ClientResponse& response)
{
	VideoSourceViewPtr videoSourceView= getCurrentVideoSource();

	if (videoSourceView)
	{
		MikanVideoSourceIntrinsics intrinsics;
		videoSourceView->getCameraIntrinsics(intrinsics);

		writeTypedJsonResponse(request.requestId, intrinsics, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_NoVideoSource, response);
	}
}

void MikanServer::getVideoSourceMode(
	const ClientRequest& request,
	ClientResponse& response)
{
	VideoSourceViewPtr videoSourceView = getCurrentVideoSource();

	if (videoSourceView)
	{
		const std::string devicePath= videoSourceView->getUSBDevicePath();
		const IVideoSourceInterface::eDriverType driverType= videoSourceView->getVideoSourceDriverType();
		const VideoModeConfig* modeConfig= videoSourceView->getVideoMode();

		MikanVideoSourceMode info;
		info.device_path = devicePath;
		info.frame_rate = modeConfig->frameRate;
		info.resolution_x = modeConfig->bufferPixelWidth;
		info.resolution_y = modeConfig->bufferPixelHeight;
		info.video_mode_name = modeConfig->modeName;
		switch (driverType)
		{
		case IVideoSourceInterface::OpenCV:
			info.video_source_api = MikanVideoSourceApi_INVALID;
			break;
		case IVideoSourceInterface::WindowsMediaFramework:
			info.video_source_api = MikanVideoSourceApi_WINDOWS_MEDIA_FOUNDATION;
			break;
		case IVideoSourceInterface::INVALID:
		default:
			info.video_source_api= MikanVideoSourceApi_INVALID;
			break;
		}
		info.video_source_type= videoSourceView->getIsStereoCamera() ? MikanVideoSourceType_STEREO : MikanVideoSourceType_MONO;

		writeTypedJsonResponse(request.requestId, info, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_NoVideoSource, response);
	}
}

void MikanServer::getVideoSourceAttachment(
	const ClientRequest& request,
	ClientResponse& response)
{
	VideoSourceViewPtr videoSourceView = getCurrentVideoSource();

	if (videoSourceView)
	{
		VRDeviceViewPtr vrDeviceView = getCurrentCameraVRDevice();

		if (vrDeviceView)
		{
			MikanVideoSourceAttachmentInfo info;

			// Get the ID of the VR tracker device
			info.attached_vr_device_id = (vrDeviceView) ? vrDeviceView->getDeviceID() : INVALID_MIKAN_ID;

			// Get the camera offset
			const glm::vec3 cameraOffsetPos = MikanVector3d_to_glm_dvec3(videoSourceView->getCameraOffsetPosition());
			const glm::quat cameraOffsetQuat = MikanQuatd_to_glm_dquat(videoSourceView->getCameraOffsetOrientation());
			const glm::mat4 cameraOffsetXform =
				glm::translate(glm::mat4(1.0), cameraOffsetPos) *
				glm::mat4_cast(cameraOffsetQuat);
			info.vr_device_offset_xform = glm_mat4_to_MikanMatrix4f(cameraOffsetXform);

			writeTypedJsonResponse(request.requestId, info, response);
		}
		else
		{
			writeSimpleJsonResponse(request.requestId, MikanResult_NoVideoSource, response);
		}
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_NoVideoSource, response);
	}
}

void MikanServer::getVRDeviceList(
	const ClientRequest& request,
	ClientResponse& response)
{
	VRDeviceList deviceList= VRDeviceManager::getInstance()->getVRDeviceList();

	MikanVRDeviceList vrDeviceListResult= {};

	for (VRDeviceViewPtr deviceView : deviceList)
	{
		vrDeviceListResult.vr_device_id_list.push_back(deviceView->getDeviceID());
	}

	writeTypedJsonResponse(request.requestId, vrDeviceListResult, response);
}

void MikanServer::getVRDeviceInfo(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanVRDeviceID deviceId;
	if (!readRequestPayload(request.utf8RequestString, deviceId))
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_MalformedParameters, response);
		return;
	}

	VRDeviceViewPtr vrDeviceView = VRDeviceManager::getInstance()->getVRDeviceViewById(deviceId);
	if (!vrDeviceView)
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_InvalidDeviceId, response);
		return;
	}

	MikanVRDeviceInfo info= {};
	info.device_path= vrDeviceView->getDevicePath();

	switch (vrDeviceView->getVRTrackerDriverType())
	{
	case IVRDeviceInterface::eDriverType::SteamVR:
		info.vr_device_api= MikanVRDeviceApi_STEAM_VR;
		break;
	default:
		info.vr_device_api = MikanVRDeviceApi_INVALID;
	}

	switch (vrDeviceView->getVRDeviceType())
	{
	case eDeviceType::HMD:
		info.vr_device_type = MikanVRDeviceType_HMD;
		break;
	case eDeviceType::VRController:
		info.vr_device_type = MikanVRDeviceType_CONTROLLER;
		break;
	case eDeviceType::VRTracker:
		info.vr_device_type = MikanVRDeviceType_TRACKER;
		break;
	default:
		info.vr_device_type= MikanVRDeviceType_INVALID;
	}

	writeTypedJsonResponse(request.requestId, info, response);
}

void MikanServer::subscribeToVRDevicePoseUpdates(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanVRDeviceID deviceId;
	if (!readRequestPayload(request.utf8RequestString, deviceId))
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_MalformedParameters, response);
		return;
	}

	auto connection_it = m_clientConnections.find(request.connectionId);
	if (connection_it == m_clientConnections.end())
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_UnknownClient, response);
		return;
	}

	MikanClientConnectionStatePtr clientState = connection_it->second;
	clientState->subscribeToVRDevicePoseUpdates(deviceId);
	writeSimpleJsonResponse(request.requestId, MikanResult_Success, response);
}

void MikanServer::unsubscribeFromVRDevicePoseUpdates(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanVRDeviceID deviceId;
	if (!readRequestPayload(request.utf8RequestString, deviceId))
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_MalformedParameters, response);
		return;
	}

	auto connection_it = m_clientConnections.find(request.connectionId);
	if (connection_it == m_clientConnections.end())
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_UnknownClient, response);
		return;
	}

	MikanClientConnectionStatePtr clientState = connection_it->second;
	clientState->unsubscribeFromVRDevicePoseUpdates(deviceId);
	writeSimpleJsonResponse(request.requestId, MikanResult_Success, response);
}

void MikanServer::allocateRenderTargetTextures(
	const ClientRequest& request,
	ClientResponse& response)
{	
	MikanRenderTargetDescriptor desc;
	if (!readRequestPayload(request.utf8RequestString, desc))
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_MalformedParameters, response);
		return;
	}

	auto connection_it = m_clientConnections.find(request.connectionId);
	if (connection_it == m_clientConnections.end())
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_UnknownClient, response);
		return;
	}

	MikanClientConnectionStatePtr clientState = connection_it->second;
	if (clientState->allocateRenderTargetTextures(desc))
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_Success, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_GeneralError, response);
	}
}

void MikanServer::freeRenderTargetTextures(
	const ClientRequest& request,
	ClientResponse& response)
{
	auto connection_it = m_clientConnections.find(request.connectionId);
	if (connection_it != m_clientConnections.end())
	{
		if (OnClientRenderTargetReleased)
		{
			MikanClientConnectionStatePtr clientState = connection_it->second;

			OnClientRenderTargetReleased(
				clientState->getClientId(), 
				connection_it->second->getRenderTargetReadAccessor());
		}

		connection_it->second->freeRenderTargetTextures();
		writeSimpleJsonResponse(request.requestId, MikanResult_Success, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_UnknownClient, response);
	}
}

void MikanServer::frameRendered(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanClientFrameRendered frameInfo = {};
	if (!readRequestPayload(request.utf8RequestString, frameInfo))
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_MalformedParameters, response);
		return;
	}

	auto connection_it = m_clientConnections.find(request.connectionId);
	if (connection_it != m_clientConnections.end())
	{
		// Process incoming video frames, if we have a compositor active
		if (OnClientRenderTargetUpdated)
		{
			MikanClientConnectionStatePtr clientState = connection_it->second;

			if (clientState->readRenderTargetTextures(frameInfo))
			{
				OnClientRenderTargetUpdated(clientState->getClientId(), frameInfo.frame_index);
			}
		}

		writeSimpleJsonResponse(request.requestId, MikanResult_Success, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_UnknownClient, response);
	}
}

void MikanServer::getQuadStencilList(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanStencilList stencilListResult = {};

	auto stencilSystemConfig = App::getInstance()->getProfileConfig()->stencilConfig;
	for (QuadStencilDefinitionPtr quadConfig : stencilSystemConfig->quadStencilList)
	{
		stencilListResult.stencil_id_list.push_back(quadConfig->getStencilId());
	}

	writeTypedJsonResponse(request.requestId, stencilListResult, response);
}

void MikanServer::getQuadStencil(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanStencilID stencilId;
	if (!readRequestPayload(request.utf8RequestString, stencilId))
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_MalformedParameters, response);
		return;
	}

	auto stencilSystemConfig = App::getInstance()->getProfileConfig()->stencilConfig;
	auto quadConfig= stencilSystemConfig->getQuadStencilConfigConst(stencilId);
	if (quadConfig != nullptr)
	{
		MikanStencilQuadInfo stencil= quadConfig->getQuadInfo();

		writeTypedJsonResponse(request.requestId, stencil, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_InvalidStencilID, response);
	}
}

void MikanServer::getBoxStencilList(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanStencilList stencilListResult = {};

	auto stencilSystemConfig = App::getInstance()->getProfileConfig()->stencilConfig;
	for (BoxStencilDefinitionPtr boxConfig : stencilSystemConfig->boxStencilList)
	{
		stencilListResult.stencil_id_list.push_back(boxConfig->getStencilId());
	}

	writeTypedJsonResponse(request.requestId, stencilListResult, response);
}

void MikanServer::getBoxStencil(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanStencilID stencilId;
	if (!readRequestPayload(request.utf8RequestString, stencilId))
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_MalformedParameters, response);
		return;
	}

	auto stencilSystemConfig = App::getInstance()->getProfileConfig()->stencilConfig;
	auto boxConfig = stencilSystemConfig->getBoxStencilConfigConst(stencilId);
	if (boxConfig != nullptr)
	{
		MikanStencilBoxInfo stencil = boxConfig->getBoxInfo();

		writeTypedJsonResponse(request.requestId, stencil, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_InvalidStencilID, response);
	}
}

void MikanServer::getModelStencilList(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanStencilList stencilListResult = {};

	auto stencilSystemConfig = App::getInstance()->getProfileConfig()->stencilConfig;
	for (ModelStencilDefinitionPtr modelConfig : stencilSystemConfig->modelStencilList)
	{
		stencilListResult.stencil_id_list.push_back(modelConfig->getStencilId());
	}

	writeTypedJsonResponse(request.requestId, stencilListResult, response);
}

void MikanServer::getModelStencil(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanStencilID stencilId;
	if (!readRequestPayload(request.utf8RequestString, stencilId))
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_MalformedParameters, response);
		return;
	}

	auto stencilSystemConfig = App::getInstance()->getProfileConfig()->stencilConfig;
	auto modelConfig = stencilSystemConfig->getModelStencilConfigConst(stencilId);
	if (modelConfig != nullptr)
	{
		MikanStencilModelInfo stencil = modelConfig->getModelInfo();

		writeTypedJsonResponse(request.requestId, stencil, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_InvalidStencilID, response);
	}
}

void MikanServer::getModelStencilRenderGeometry(const ClientRequest& request, ClientResponse& response)
{
	MikanStencilID stencilId;
	if (!readRequestPayload(request.utf8RequestString, stencilId))
	{
		writeSimpleBinaryResponse(request.requestId, MikanResult_MalformedParameters, response);
		return;
	}

	ModelStencilComponentPtr modelStencil= StencilObjectSystem::getSystem()->getModelStencilById(stencilId);
	if (modelStencil)
	{
		MikanStencilModelRenderGeometry renderGeometry = {};
		modelStencil->extractRenderGeometry(renderGeometry);

		writeTypedBinaryResponse(request.requestId, renderGeometry, response);
	}
	else
	{
		writeSimpleBinaryResponse(request.requestId, MikanResult_InvalidStencilID, response);
	}
}

void MikanServer::getSpatialAnchorList(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanSpatialAnchorList anchorListResult= {};

	auto anchorSystemConfig = App::getInstance()->getProfileConfig()->anchorConfig;
	for (AnchorDefinitionPtr spatialAnchor : anchorSystemConfig->spatialAnchorList)
	{
		anchorListResult.spatial_anchor_id_list.push_back(spatialAnchor->getAnchorId());
	}

	writeTypedJsonResponse(request.requestId, anchorListResult, response);
}

void MikanServer::getSpatialAnchorInfo(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanSpatialAnchorID anchorId;
	if (!readRequestPayload(request.utf8RequestString, anchorId))
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_MalformedParameters, response);
		return;
	}

	AnchorComponentPtr anchorPtr= AnchorObjectSystem::getSystem()->getSpatialAnchorById(anchorId);
	if (anchorPtr == nullptr)
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_InvalidAnchorID, response);
		return;
	}
	
	MikanSpatialAnchorInfo anchorInfo;
	anchorPtr->extractAnchorInfoForClientAPI(anchorInfo);

	writeTypedJsonResponse(request.requestId, anchorInfo, response);
}

void MikanServer::findSpatialAnchorInfoByName(
	const ClientRequest& request,
	ClientResponse& response)
{
	std::string nameBuffer;
	if (!readRequestPayload(request.utf8RequestString, nameBuffer))
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_MalformedParameters, response);
		return;
	}

	AnchorComponentPtr anchorPtr = AnchorObjectSystem::getSystem()->getSpatialAnchorByName(nameBuffer);
	if (anchorPtr == nullptr)
	{
		writeSimpleJsonResponse(request.requestId, MikanResult_InvalidAnchorID, response);
		return;
	}

	MikanSpatialAnchorInfo anchorInfo;
	anchorPtr->extractAnchorInfoForClientAPI(anchorInfo);

	writeTypedJsonResponse(request.requestId, anchorInfo, response);
}