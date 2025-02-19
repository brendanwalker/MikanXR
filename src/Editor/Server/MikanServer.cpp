//-- includes -----
#include "App.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "BinarySerializer.h"
#include "BinaryUtility.h"
#include "BoxStencilComponent.h"
#include "CommonScriptContext.h"
#include "MathTypeConversion.h"
#include "JsonDeserializer.h"
#include "JsonSerializer.h"
#include "Logger.h"
#include "MikanAPITypes.h"
#include "MikanCoreTypes.h"
#include "MikanRenderTargetRequests.h"
#include "MikanClientRequests.h"
#include "MikanScriptRequests.h"
#include "MikanSpatialAnchorRequests.h"
#include "MikanStencilRequests.h"
#include "MikanVideoSourceRequests.h"
#include "MikanVRDeviceRequests.h"
#include "MikanScriptTypes.h"
#include "MikanServer.h"
#include "ModelStencilComponent.h"
#include "ProfileConfig.h"
#include "QuadStencilComponent.h"
#include "RemoteControlManager.h"
#include "ServerResponseHelpers.h"
#include "SharedTextureReader.h"
#include "StencilObjectSystemConfig.h"
#include "StencilObjectSystem.h"
#include "StringUtils.h"
#include "VideoCapabilitiesConfig.h"
#include "VideoSourceView.h"
#include "VideoSourceManager.h"
#include "VRDeviceManager.h"
#include "VRDeviceView.h"
#include "Version.h"
#include "WebsocketInterprocessMessageServer.h"

#include <set>
#include <assert.h>

#include <Refureku/Refureku.h>
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
		IInterprocessMessageServer* messageServer)
		: m_connectionId(connectionId)
		, m_messageServer(messageServer)
		, m_connectionInfo(new MikanClientConnectionInfo())
		, m_subscribedVRDevices()
	{	
	}

	virtual ~MikanClientConnectionState()
	{
		delete m_connectionInfo;
	}

	const std::string& getConnectionId() const
	{
		return m_connectionId;
	}

	const MikanClientConnectionInfo& getClientConnectionInfo() const 
	{
		return *m_connectionInfo;
	}
	
	const std::string& getClientId() const 
	{
		return m_connectionInfo->getClientId();
	}

	void setMikanClientInfo(const MikanClientInfo& clientInfo)
	{
		m_connectionInfo->setClientInfo(clientInfo);
	}

	void clearMikanClientInfo()
	{
		m_connectionInfo->clearMikanClientInfo();
	}

	const MikanClientInfo& getMikanClientInfo() const 
	{	
		return m_connectionInfo->getClientInfo();
	}

	bool readRenderTargetTextures(const int64_t newFrameIndex)
	{
		EASY_FUNCTION();

		return getRenderTargetReadAccessor()->readRenderTargetTextures(newFrameIndex);
	}

	SharedTextureReadAccessor* getRenderTargetReadAccessor() const
	{
		return m_connectionInfo->getRenderTargetReadAccessor();
	}

	MikanClientGraphicsApi getClientGraphicsAPI() const
	{
		return m_connectionInfo->getRenderTargetReadAccessor()->getClientGraphicsAPI();
	}

	void subscribeToVRDevicePoseUpdatesHandler(MikanVRDeviceID deviceId)
	{
		m_subscribedVRDevices.insert(deviceId);
	}

	void unsubscribeFromVRDevicePoseUpdatesHandler(MikanVRDeviceID deviceId)
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

		freeRenderTargetTexturesHandler();

		if (m_connectionInfo->allocateRenderTargetTextures(desc))
		{
			MikanServer* mikanServer= MikanServer::getInstance();
			if (mikanServer->OnClientRenderTargetAllocated)
			{
				mikanServer->OnClientRenderTargetAllocated(
					m_connectionInfo->getClientId(),
					m_connectionInfo->getClientInfo(),
					m_connectionInfo->getRenderTargetReadAccessor());
			}

			return true;
		}

		return false;
	}

	void freeRenderTargetTexturesHandler()
	{
		EASY_FUNCTION();

		if (m_connectionInfo->hasAllocatedRenderTarget())
		{
			MikanServer* mikanServer = MikanServer::getInstance();
			if (mikanServer->OnClientRenderTargetReleased)
			{
				mikanServer->OnClientRenderTargetReleased(
					m_connectionInfo->getClientId(),
					m_connectionInfo->getRenderTargetReadAccessor());
			}

			m_connectionInfo->freeRenderTargetTexturesHandler();
		}
	}

	template <typename t_mikan_type>
	std::string mikanTypeToJsonString(const t_mikan_type& mikanType)
	{
		EASY_FUNCTION();

		std::string jsonStr;
		Serialization::serializeToJsonString(mikanType, jsonStr);

		return jsonStr;
	}

	void publishMikanJsonEvent(const std::string& mikanJsonEvent)
	{
		m_messageServer->sendMessageToClient(getConnectionId(), mikanJsonEvent);
	}

	template <typename t_mikan_type>
	void publishSimpleEvent()
	{
		t_mikan_type mikanEvent;
		publishMikanJsonEvent(mikanTypeToJsonString(mikanEvent));
	}

	// Connection Events
	void publishClientConnectedEvent(bool bIsClientCompatible)
	{
		MikanConnectedEvent connectedEvent = {};
		connectedEvent.serverVersion.version= MIKAN_SERVER_API_VERSION;
		connectedEvent.minClientVersion.version= MIKAN_MIN_ALLOWED_CLIENT_API_VERSION;
		connectedEvent.isClientCompatible= bIsClientCompatible;

		m_messageServer->sendMessageToClient(getConnectionId(), mikanTypeToJsonString(connectedEvent));
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
	void publishVRDevicePoses(int64_t newVRFrameIndex)
	{
		VRDeviceManager* vrDeviceManager= VRDeviceManager::getInstance();

		for (auto deviceId : m_subscribedVRDevices)
		{
			VRDeviceViewPtr vrDeviceView= vrDeviceManager->getVRDeviceViewById(deviceId);

			if (vrDeviceView && vrDeviceView->getIsOpen() && vrDeviceView->getIsPoseValid())
			{
				// TODO: We should provide option to select which component we want the pose updates for
				glm::mat4 xform;
				if (vrDeviceView->getDefaultComponentPose(xform))
				{
					// Send a pose update to the client
					MikanVRDevicePoseUpdateEvent poseUpdate;
					poseUpdate.transform = glm_mat4_to_MikanMatrix4f(xform);
					poseUpdate.device_id = deviceId;
					poseUpdate.frame = newVRFrameIndex;

					m_messageServer->sendMessageToClient(getConnectionId(), mikanTypeToJsonString(poseUpdate));
				}
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
	IInterprocessMessageServer* m_messageServer= nullptr;
	MikanClientConnectionInfo* m_connectionInfo= nullptr;
	std::set<MikanVRDeviceID> m_subscribedVRDevices;
};

// -- MikanClientConnectionInfo -----
MikanClientConnectionInfo::MikanClientConnectionInfo()
	: m_clientInfo()
	, m_renderTargetReadAccessor(nullptr)
{

}

MikanClientConnectionInfo::~MikanClientConnectionInfo()
{
	disposeRenderTargetAccessor();
}

void MikanClientConnectionInfo::allocateRenderTargetAccessor()
{
	assert(m_renderTargetReadAccessor == nullptr);
	assert(isClientInfoValid());
	m_renderTargetReadAccessor = new SharedTextureReadAccessor(getClientId());
}

void MikanClientConnectionInfo::disposeRenderTargetAccessor()
{
	if (m_renderTargetReadAccessor != nullptr)
	{
		delete m_renderTargetReadAccessor;
		m_renderTargetReadAccessor = nullptr;
	}
}

void MikanClientConnectionInfo::clearMikanClientInfo()
{
	// Free any existing render target
	disposeRenderTargetAccessor();

	// Reset the client info with defaults
	m_clientInfo= MikanClientInfo();
}

void MikanClientConnectionInfo::setClientInfo(const MikanClientInfo& clientInfo)
{
	// Free any existing render target
	disposeRenderTargetAccessor();

	// Set the new client info describing the client render capabilities
	m_clientInfo = clientInfo;

	// Allocate a new render target accessor
	allocateRenderTargetAccessor();
}

const MikanClientInfo& MikanClientConnectionInfo::getClientInfo() const
{
	return m_clientInfo;
}

const std::string& MikanClientConnectionInfo::getClientId() const
{
	return m_clientInfo.clientId.getValue();
}

bool MikanClientConnectionInfo::isClientInfoValid() const
{
	return !getClientId().empty();
}

bool MikanClientConnectionInfo::hasAllocatedRenderTarget() const
{
	if (m_renderTargetReadAccessor != nullptr)
	{
		const MikanRenderTargetDescriptor& desc = m_renderTargetReadAccessor->getRenderTargetDescriptor();

		return
			desc.color_buffer_type != MikanColorBuffer_NOCOLOR ||
			desc.depth_buffer_type != MikanDepthBuffer_NODEPTH;
	}

	return false;
}

bool MikanClientConnectionInfo::allocateRenderTargetTextures(const MikanRenderTargetDescriptor& desc)
{
	EASY_FUNCTION();

	if (m_renderTargetReadAccessor != nullptr)
	{
		// This will free any existing render target
		return m_renderTargetReadAccessor->initialize(&desc);
	}

	return false;
}

void MikanClientConnectionInfo::freeRenderTargetTexturesHandler()
{
	EASY_FUNCTION();

	if (m_renderTargetReadAccessor != nullptr)
	{
		m_renderTargetReadAccessor->dispose();
	}
}

// -- MikanServer -----
MikanServer* MikanServer::m_instance= nullptr;

MikanServer::MikanServer()
	: m_messageServer(new WebsocketInterprocessMessageServer())
	, m_remoteControlManager(new RemoteControlManager(this))
{
	m_instance= this;
}

MikanServer::~MikanServer()
{
	delete m_remoteControlManager;
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

bool MikanServer::startup(MainWindow* mainWindow)
{
	EASY_FUNCTION();

	if (!m_messageServer->initialize())
	{
		MIKAN_LOG_ERROR("MikanServer::startup()") << "Failed to initialize interprocess message server";
		return false;
	}

	// Bind the remote control request handlers
	if (!m_remoteControlManager->startup(mainWindow))
	{
		MIKAN_LOG_ERROR("MikanServer::startup()") << "Failed to bind remote control request handlers";
		return false;
	}

	// Websocket Event Handlers
	m_messageServer->setSocketEventHandler(
		WEBSOCKET_CONNECT_EVENT,
		std::bind(&MikanServer::onClientConnectedHandler, this, _1));
	m_messageServer->setSocketEventHandler(
		WEBSOCKET_DISCONNECT_EVENT,
		std::bind(&MikanServer::onClientDisconnectedHandler, this, _1));
	m_messageServer->setSocketEventHandler(
		WEBSOCKET_ERROR_EVENT,
		std::bind(&MikanServer::onClientErrorHandler, this, _1));

	// Client Init/Dispose Requests
	m_messageServer->setRequestHandler(
		InitClientRequest::staticGetArchetype().getId(), 
		std::bind(&MikanServer::initClientHandler, this, _1, _2));
	m_messageServer->setRequestHandler(
		DisposeClientRequest::staticGetArchetype().getId(), 
		std::bind(&MikanServer::disposeClientHandler, this, _1, _2));

	// Render Target Requests
	m_messageServer->setRequestHandler(
		AllocateRenderTargetTextures::staticGetArchetype().getId(), 
		std::bind(&MikanServer::allocateRenderTargetTexturesHandler, this, _1, _2));
	m_messageServer->setRequestHandler(
		FreeRenderTargetTextures::staticGetArchetype().getId(), 
		std::bind(&MikanServer::freeRenderTargetTexturesHandler, this, _1, _2));
	m_messageServer->setRequestHandler(
		PublishRenderTargetTextures::staticGetArchetype().getId(), 
		std::bind(&MikanServer::frameRenderedHandler, this, _1, _2));

	// Script Requests	
	m_messageServer->setRequestHandler(
		SendScriptMessage::staticGetArchetype().getId(), 
		std::bind(&MikanServer::invokeScriptMessageHandler, this, _1, _2));

	// Spatial Anchor Requests
	m_messageServer->setRequestHandler(
		GetSpatialAnchorList::staticGetArchetype().getId(), 
		std::bind(&MikanServer::getSpatialAnchorListHandler, this, _1, _2));
	m_messageServer->setRequestHandler(
		GetSpatialAnchorInfo::staticGetArchetype().getId(), 
		std::bind(&MikanServer::getSpatialAnchorInfoHandler, this, _1, _2));
	m_messageServer->setRequestHandler(
		FindSpatialAnchorInfoByName::staticGetArchetype().getId(),
		std::bind(&MikanServer::findSpatialAnchorInfoByNameHandler, this, _1, _2));

	// Stencil Requests
	m_messageServer->setRequestHandler(
		GetQuadStencilList::staticGetArchetype().getId(), 
		std::bind(&MikanServer::getQuadStencilListHandler, this, _1, _2));
	m_messageServer->setRequestHandler(
		GetQuadStencil::staticGetArchetype().getId(), 
		std::bind(&MikanServer::getQuadStencilHandler, this, _1, _2));
	m_messageServer->setRequestHandler(
		GetBoxStencilList::staticGetArchetype().getId(), 
		std::bind(&MikanServer::getBoxStencilListHandler, this, _1, _2));
	m_messageServer->setRequestHandler(
		GetBoxStencil::staticGetArchetype().getId(), 
		std::bind(&MikanServer::getBoxStencilHandler, this, _1, _2));
	m_messageServer->setRequestHandler(
		GetModelStencilList::staticGetArchetype().getId(), 
		std::bind(&MikanServer::getModelStencilListHandler, this, _1, _2));
	m_messageServer->setRequestHandler(
		GetModelStencil::staticGetArchetype().getId(), 
		std::bind(&MikanServer::getModelStencilHandler, this, _1, _2));
	m_messageServer->setRequestHandler(
		GetModelStencilRenderGeometry::staticGetArchetype().getId(), 
		std::bind(&MikanServer::getModelStencilRenderGeometryHandler, this, _1, _2));

	// Video Source Requests
	m_messageServer->setRequestHandler(
		GetVideoSourceIntrinsics::staticGetArchetype().getId(), 
		std::bind(&MikanServer::getVideoSourceIntrinsicsHandler, this, _1, _2));
	m_messageServer->setRequestHandler(
		GetVideoSourceMode::staticGetArchetype().getId(), 
		std::bind(&MikanServer::getVideoSourceModeHandler, this, _1, _2));
	m_messageServer->setRequestHandler(
		GetVideoSourceAttachment::staticGetArchetype().getId(), 
		std::bind(&MikanServer::getVideoSourceAttachmentHandler, this, _1, _2));

	// VR Device Requests
	m_messageServer->setRequestHandler(
		GetVRDeviceList::staticGetArchetype().getId(), 
		std::bind(&MikanServer::getVRDeviceListHandler, this, _1, _2));
	m_messageServer->setRequestHandler(
		GetVRDeviceInfo::staticGetArchetype().getId(), 
		std::bind(&MikanServer::getVRDeviceInfoHandler, this, _1, _2));
	m_messageServer->setRequestHandler(
		SubscribeToVRDevicePoseUpdates::staticGetArchetype().getId(), 
		std::bind(&MikanServer::subscribeToVRDevicePoseUpdatesHandler, this, _1, _2));
	m_messageServer->setRequestHandler(
		UnsubscribeFromVRDevicePoseUpdates::staticGetArchetype().getId(), 
		std::bind(&MikanServer::unsubscribeFromVRDevicePoseUpdatesHandler, this, _1, _2));

	VRDeviceManager::getInstance()->OnDeviceListChanged 
		+= MakeDelegate(this, &MikanServer::publishVRDeviceListChanged);
	VRDeviceManager::getInstance()->OnDevicePosesChanged 
		+= MakeDelegate(this, &MikanServer::publishVRDevicePoses);

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

		m_messageServer->processSocketEvents();
		m_messageServer->processRequests();
	}
}

void MikanServer::shutdown()
{
	VRDeviceManager::getInstance()->OnDevicePosesChanged -= MakeDelegate(this, &MikanServer::publishVRDevicePoses);

	m_clientConnections.clear();
	m_messageServer->dispose();
}

void MikanServer::publishMikanJsonEvent(const std::string& mikanJsonEvent)
{
	for (auto& connection_it : m_clientConnections)
	{
		connection_it.second->publishMikanJsonEvent(mikanJsonEvent);
	}
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

void MikanServer::publishVRDevicePoses(int64_t newFrameIndex)
{
	for (auto& connection_it : m_clientConnections)
	{
		connection_it.second->publishVRDevicePoses(newFrameIndex);
	}
}

// RPC Callbacks
void MikanServer::getConnectedClientInfoList(std::vector<const MikanClientConnectionInfo*>& outClientList) const
{
	outClientList.clear();
	for (auto& connection_it : m_clientConnections)
	{
		const auto& connectionInfo= connection_it.second->getClientConnectionInfo();

		outClientList.push_back(&connectionInfo);
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

// Connection State Management
MikanClientConnectionStatePtr MikanServer::allocateClientConnectionState(
	const std::string& connectionId)
{
	MikanClientConnectionStatePtr clientState;

	auto connection_it = m_clientConnections.find(connectionId);
	if (connection_it != m_clientConnections.end())
	{
		// Client already exists
		clientState = connection_it->second;
	}
	else
	{
		// Create a new client state
		clientState =
			std::make_shared<MikanClientConnectionState>(
				connectionId,
				m_messageServer);

		m_clientConnections.insert({connectionId, clientState});
	}

	return clientState;
}

void MikanServer::disposeClientConnectionState(const std::string& connectionId)
{
	auto connection_it = m_clientConnections.find(connectionId);
	if (connection_it != m_clientConnections.end())
	{
		const std::string& clientId = connection_it->first;
		MikanClientConnectionStatePtr connectionState = connection_it->second;

		// Make sure the client info is disposed before removing the client connection
		// (Client may have already done this)
		disposeClientInfo(connectionState);

		// Finally, remove the client connection from the connection list 
		// (which will delete the client state)
		m_clientConnections.erase(connection_it);
	}
}

void MikanServer::initClientInfo(MikanClientConnectionStatePtr connectionState, const MikanClientInfo& clientInfo)
{
	const std::string& clientId = clientInfo.clientId.getValue();

	// Fill in the client info and allocate render target read accessor
	// After this point, the connection can allocate render target textures
	connectionState->setMikanClientInfo(clientInfo);

	// Tell any listeners that the given client ID has initialized new client info
	if (OnClientInitialized)
	{
		OnClientInitialized(clientId, clientInfo);
	}
}

bool MikanServer::disposeClientInfo(MikanClientConnectionStatePtr connectionState)
{
	const MikanClientConnectionInfo& connectionInfo = connectionState->getClientConnectionInfo();

	if (connectionInfo.isClientInfoValid())
	{
		const std::string& clientId = connectionInfo.getClientId();

		// Make sure all render target textures are freed before disposing the client info
		// (Client may have already done this)
		connectionState->freeRenderTargetTexturesHandler();

		// Tell any listeners that the given client ID has initialized is clearing its client info
		if (OnClientDisposed)
		{
			OnClientDisposed(clientId);
		}

		// Dispose any render target textures and reset the client info to defaults
		connectionState->clearMikanClientInfo();
		return true;
	}

	return false;
}

// Websocket Event Handlers
void MikanServer::onClientConnectedHandler(const ClientSocketEvent& event)
{
	MIKAN_LOG_INFO("onClientConnected")
		<< "connectionId: " << event.connectionId
		<< ", protocol: " << event.eventArgs[0];

	// Determine if the client is compatible with the server
	// by checking the protocol version
	bool bIsClientCompatible= false;
	std::vector<std::string> protocols= StringUtils::splitString(event.eventArgs[0], ',');
	int clientProtocol= -1;
	for (const std::string& protocol : protocols)
	{
		std::string prefix = WEBSOCKET_PROTOCOL_PREFIX;
		if (protocol.rfind(prefix.c_str(), 0) == 0)
		{
			std::string versionString = protocol.substr(prefix.length());

			if (!versionString.empty())
			{
				int clientProtocol = std::atoi(versionString.c_str());

				bIsClientCompatible= clientProtocol >= MIKAN_MIN_ALLOWED_CLIENT_API_VERSION;
				break;
			}
		}
	}

	// Create a new client state for the connection
	MikanClientConnectionStatePtr clientState= allocateClientConnectionState(event.connectionId);

	// Tell the client if they are compatible with the server
	// Up to the client to trigger disconnect in response
	clientState->publishClientConnectedEvent(bIsClientCompatible);
}

void MikanServer::onClientDisconnectedHandler(const ClientSocketEvent& event)
{
	MIKAN_LOG_INFO("onClientDisconnected")
		<< "connectionId: " << event.connectionId
		<< ", code: " << event.eventArgs[0]
		<< ", reason: " << event.eventArgs[1];

	disposeClientConnectionState(event.connectionId);
}

void MikanServer::onClientErrorHandler(const ClientSocketEvent& event)
{
	MIKAN_LOG_ERROR("onClientError")
		<< "connectionId: " << event.connectionId 
		<< ", error: " << event.eventArgs[0];
}

// Request Callbacks
void MikanServer::initClientHandler(const ClientRequest& request, ClientResponse& response)
{
	InitClientRequest initClientRequest;
	if (!readTypedRequest(request.utf8RequestString, initClientRequest) || 
		initClientRequest.clientInfo.clientId.getValue().empty())
	{
		MIKAN_LOG_ERROR("connectHandler") << "Failed to parse client info";
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	const std::string& connectionId = request.connectionId;
	const MikanClientInfo& clientInfo = initClientRequest.clientInfo;
	const std::string& clientId = clientInfo.clientId.getValue();

	auto connection_it = m_clientConnections.find(connectionId);
	if (connection_it != m_clientConnections.end())
	{
		MikanClientConnectionStatePtr connectionState= connection_it->second;

		MIKAN_LOG_INFO("e")
			<< "Client (connectionId: " << connectionId
			<< ", clientId: " << clientId << ") allocated client info";

		initClientInfo(connectionState, clientInfo);

		writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
	}
	else
	{
		MIKAN_LOG_ERROR("initClientHandler") 
			<< "Client (connectionId: " << connectionId 
			<< ", clientId: " << clientId << ") already connected";
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::AlreadyConnected, response);
	}
}

void MikanServer::disposeClientHandler(const ClientRequest& request, ClientResponse& response)
{
	const std::string& connectionId = request.connectionId;

	auto connection_it = m_clientConnections.find(connectionId);
	if (connection_it != m_clientConnections.end())
	{
		MikanClientConnectionStatePtr connectionState= connection_it->second;
		const MikanClientConnectionInfo& connectionInfo= connectionState->getClientConnectionInfo();

		// Tear down any active render target textures before destroying the client info
		connectionState->freeRenderTargetTexturesHandler();

		if (disposeClientInfo(connectionState))
		{
			const std::string& clientId= connectionInfo.getClientId();

			MIKAN_LOG_INFO("disposeClientHandler")
				<< "Client (connectionId: " << connectionId 
				<< ", clientId: " << clientId << ") deallocated client info";

			writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
		}
		else
		{
			MIKAN_LOG_ERROR("disposeClientHandler") 
				<< "Client (connection id: " << connectionId << ") already deallocated client info";

			writeSimpleJsonResponse(request.requestId, MikanAPIResult::UnknownClient, response);
		}
	}
	else
	{
		MIKAN_LOG_ERROR("disposeClientHandler") << "Client (connection id: " << connectionId <<") not connected";
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::UnknownClient, response);
	}
}

void MikanServer::invokeScriptMessageHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	SendScriptMessage scriptMessageRequest;
	if (!readTypedRequest(request.utf8RequestString, scriptMessageRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	// Find the first script context that cares about the message
	for (auto it = m_scriptContexts.begin(); it < m_scriptContexts.end(); it++)
	{
		CommonScriptContextPtr scriptContext = it->lock();

		if (scriptContext == scriptContext)
		{
			if (scriptContext->invokeScriptMessageHandler(scriptMessageRequest.message.content.getValue()))
			{
				break;
			}
		}
	}

	writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
}

void MikanServer::getVideoSourceIntrinsicsHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	VideoSourceViewPtr videoSourceView= getCurrentVideoSource();

	if (videoSourceView)
	{
		MikanVideoSourceIntrinsicsResponse intrinsicsResponse;
		videoSourceView->getCameraIntrinsics(intrinsicsResponse.intrinsics);

		writeTypedJsonResponse(request.requestId, intrinsicsResponse, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::NoVideoSource, response);
	}
}

void MikanServer::getVideoSourceModeHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	VideoSourceViewPtr videoSourceView = getCurrentVideoSource();

	if (videoSourceView)
	{
		const std::string devicePath= videoSourceView->getUSBDevicePath();
		const IVideoSourceInterface::eDriverType driverType= videoSourceView->getVideoSourceDriverType();
		const VideoModeConfig* modeConfig= videoSourceView->getVideoMode();

		MikanVideoSourceModeResponse info;
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
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::NoVideoSource, response);
	}
}

void MikanServer::getVideoSourceAttachmentHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	VideoSourceViewPtr videoSourceView = getCurrentVideoSource();

	if (videoSourceView)
	{
		VRDeviceViewPtr vrDeviceView = getCurrentCameraVRDevice();

		if (vrDeviceView)
		{
			MikanVideoSourceAttachmentInfoResponse info;

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
			writeSimpleJsonResponse(request.requestId, MikanAPIResult::NoVideoSource, response);
		}
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::NoVideoSource, response);
	}
}

void MikanServer::getVRDeviceListHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	VRDeviceList deviceList= VRDeviceManager::getInstance()->getVRDeviceList();

	MikanVRDeviceListResponse vrDeviceListResult= {};
	for (VRDeviceViewPtr deviceView : deviceList)
	{
		vrDeviceListResult.vr_device_id_list.push_back(deviceView->getDeviceID());
	}

	writeTypedJsonResponse(request.requestId, vrDeviceListResult, response);
}

void MikanServer::getVRDeviceInfoHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	GetVRDeviceInfo deviceRequest;
	if (!readTypedRequest(request.utf8RequestString, deviceRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	VRDeviceViewPtr vrDeviceView = VRDeviceManager::getInstance()->getVRDeviceViewById(deviceRequest.deviceId);
	if (!vrDeviceView)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidDeviceId, response);
		return;
	}

	MikanVRDeviceInfoResponse infoResponse= {};
	MikanVRDeviceInfo& info= infoResponse.vr_device_info;
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

	writeTypedJsonResponse(request.requestId, infoResponse, response);
}

void MikanServer::subscribeToVRDevicePoseUpdatesHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	SubscribeToVRDevicePoseUpdates deviceRequest;
	if (!readTypedRequest(request.utf8RequestString, deviceRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	auto connection_it = m_clientConnections.find(request.connectionId);
	if (connection_it == m_clientConnections.end())
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::UnknownClient, response);
		return;
	}

	MikanClientConnectionStatePtr clientState = connection_it->second;
	clientState->subscribeToVRDevicePoseUpdatesHandler(deviceRequest.deviceId);
	writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
}

void MikanServer::unsubscribeFromVRDevicePoseUpdatesHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	UnsubscribeFromVRDevicePoseUpdates deviceRequest;
	if (!readTypedRequest(request.utf8RequestString, deviceRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	auto connection_it = m_clientConnections.find(request.connectionId);
	if (connection_it == m_clientConnections.end())
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::UnknownClient, response);
		return;
	}

	MikanClientConnectionStatePtr clientState = connection_it->second;
	clientState->unsubscribeFromVRDevicePoseUpdatesHandler(deviceRequest.deviceId);
	writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
}

void MikanServer::allocateRenderTargetTexturesHandler(
	const ClientRequest& request,
	ClientResponse& response)
{	
	AllocateRenderTargetTextures allocateRequest;
	if (!readTypedRequest(request.utf8RequestString, allocateRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	auto connection_it = m_clientConnections.find(request.connectionId);
	if (connection_it == m_clientConnections.end())
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::UnknownClient, response);
		return;
	}

	MikanClientConnectionStatePtr clientState = connection_it->second;
	if (clientState->allocateRenderTargetTextures(allocateRequest.descriptor))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::GeneralError, response);
	}
}

void MikanServer::freeRenderTargetTexturesHandler(
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

		connection_it->second->freeRenderTargetTexturesHandler();
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::UnknownClient, response);
	}
}

void MikanServer::frameRenderedHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	PublishRenderTargetTextures frameRenderedRequest = {};
	if (!readTypedRequest(request.utf8RequestString, frameRenderedRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	auto connection_it = m_clientConnections.find(request.connectionId);
	if (connection_it != m_clientConnections.end())
	{
		// Process incoming video frames, if we have a compositor active
		if (OnClientRenderTargetUpdated)
		{
			MikanClientConnectionStatePtr clientState = connection_it->second;

			if (clientState->readRenderTargetTextures(frameRenderedRequest.frameIndex))
			{
				OnClientRenderTargetUpdated(clientState->getClientId(), frameRenderedRequest.frameIndex);
			}
		}

		writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::UnknownClient, response);
	}
}

void MikanServer::getQuadStencilListHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanStencilListResponse stencilListResult = {};

	auto stencilSystemConfig = App::getInstance()->getProfileConfig()->stencilConfig;
	for (QuadStencilDefinitionPtr quadConfig : stencilSystemConfig->quadStencilList)
	{
		stencilListResult.stencil_id_list.push_back(quadConfig->getStencilId());
	}

	writeTypedJsonResponse(request.requestId, stencilListResult, response);
}

void MikanServer::getQuadStencilHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	GetQuadStencil stencilRequest;
	if (!readTypedRequest(request.utf8RequestString, stencilRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	auto stencilSystemConfig = App::getInstance()->getProfileConfig()->stencilConfig;
	auto quadConfig= stencilSystemConfig->getQuadStencilConfigConst(stencilRequest.stencilId);
	if (quadConfig != nullptr)
	{
		MikanStencilQuadInfoResponse stencilResponse= {};
		stencilResponse.quad_info= quadConfig->getQuadInfo();

		writeTypedJsonResponse(request.requestId, stencilResponse, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidStencilID, response);
	}
}

void MikanServer::getBoxStencilListHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanStencilListResponse stencilListResult = {};

	auto stencilSystemConfig = App::getInstance()->getProfileConfig()->stencilConfig;
	for (BoxStencilDefinitionPtr boxConfig : stencilSystemConfig->boxStencilList)
	{
		stencilListResult.stencil_id_list.push_back(boxConfig->getStencilId());
	}

	writeTypedJsonResponse(request.requestId, stencilListResult, response);
}

void MikanServer::getBoxStencilHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	GetBoxStencil stencilRequest;
	if (!readTypedRequest(request.utf8RequestString, stencilRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	auto stencilSystemConfig = App::getInstance()->getProfileConfig()->stencilConfig;
	auto boxConfig = stencilSystemConfig->getBoxStencilConfigConst(stencilRequest.stencilId);
	if (boxConfig != nullptr)
	{
		MikanStencilBoxInfoResponse stencilResponse;
		stencilResponse.box_info = boxConfig->getBoxInfo();

		writeTypedJsonResponse(request.requestId, stencilResponse, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidStencilID, response);
	}
}

void MikanServer::getModelStencilListHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanStencilListResponse stencilListResult = {};

	auto stencilSystemConfig = App::getInstance()->getProfileConfig()->stencilConfig;
	for (ModelStencilDefinitionPtr modelConfig : stencilSystemConfig->modelStencilList)
	{
		stencilListResult.stencil_id_list.push_back(modelConfig->getStencilId());
	}

	writeTypedJsonResponse(request.requestId, stencilListResult, response);
}

void MikanServer::getModelStencilHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	GetModelStencil stencilRequest;
	if (!readTypedRequest(request.utf8RequestString, stencilRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	auto stencilSystemConfig = App::getInstance()->getProfileConfig()->stencilConfig;
	auto modelConfig = stencilSystemConfig->getModelStencilConfigConst(stencilRequest.stencilId);
	if (modelConfig != nullptr)
	{
		MikanStencilModelInfoResponse stencilResponse = {};
		stencilResponse.model_info = modelConfig->getModelInfo();

		writeTypedJsonResponse(request.requestId, stencilResponse, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidStencilID, response);
	}
}

void MikanServer::getModelStencilRenderGeometryHandler(const ClientRequest& request, ClientResponse& response)
{
	GetModelStencilRenderGeometry stencilRequest;
	if (!readTypedRequest(request.utf8RequestString, stencilRequest))
	{
		writeSimpleBinaryResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	ModelStencilComponentPtr modelStencil= 
		StencilObjectSystem::getSystem()->getModelStencilById(stencilRequest.stencilId);
	if (modelStencil)
	{
		MikanStencilModelRenderGeometryResponse renderGeometryResponse = {};
		modelStencil->extractRenderGeometry(renderGeometryResponse.render_geometry);

		writeTypedBinaryResponse(request.requestId, renderGeometryResponse, response);
	}
	else
	{
		writeSimpleBinaryResponse(request.requestId, MikanAPIResult::InvalidStencilID, response);
	}
}

void MikanServer::getSpatialAnchorListHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanSpatialAnchorListResponse anchorListResult= {};

	auto anchorSystemConfig = App::getInstance()->getProfileConfig()->anchorConfig;
	for (AnchorDefinitionPtr spatialAnchor : anchorSystemConfig->spatialAnchorList)
	{
		anchorListResult.spatial_anchor_id_list.push_back(spatialAnchor->getAnchorId());
	}

	writeTypedJsonResponse(request.requestId, anchorListResult, response);
}

void MikanServer::getSpatialAnchorInfoHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	GetSpatialAnchorInfo anchorRequest;
	if (!readTypedRequest(request.utf8RequestString, anchorRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	AnchorComponentPtr anchorPtr= AnchorObjectSystem::getSystem()->getSpatialAnchorById(anchorRequest.anchorId);
	if (anchorPtr == nullptr)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidAnchorID, response);
		return;
	}
	
	MikanSpatialAnchorInfoResponse anchorInfoResponse = {};
	anchorPtr->extractAnchorInfoForClientAPI(anchorInfoResponse.anchor_info);

	writeTypedJsonResponse(request.requestId, anchorInfoResponse, response);
}

void MikanServer::findSpatialAnchorInfoByNameHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	FindSpatialAnchorInfoByName anchorRequest;
	if (!readTypedRequest(request.utf8RequestString, anchorRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	const std::string& anchorName= anchorRequest.anchorName.getValue();
	AnchorComponentPtr anchorPtr = AnchorObjectSystem::getSystem()->getSpatialAnchorByName(anchorName);
	if (anchorPtr == nullptr)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidAnchorID, response);
		return;
	}

	MikanSpatialAnchorInfoResponse anchorInfoResponse = {};
	anchorPtr->extractAnchorInfoForClientAPI(anchorInfoResponse.anchor_info);

	writeTypedJsonResponse(request.requestId, anchorInfoResponse, response);
}