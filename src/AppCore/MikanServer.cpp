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
#include "StencilObjectSystemConfig.h"
#include "VideoCapabilitiesConfig.h"
#include "VideoSourceView.h"
#include "VideoSourceManager.h"
#include "VRDeviceManager.h"
#include "VRDeviceView.h"

#include "WebsocketInterprocessMessageServer.h"
#include "MikanClientTypes.h"
#include "MikanClientTypes_json.h"
#include "MikanEventTypes.h"
#include "MikanEventTypes_json.h"
#include "MikanVideoSourceTypes.h"
#include "MikanVideoSourceTypes_json.h"

#include <set>
#include <assert.h>

#include <easy/profiler.h>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

#ifdef _MSC_VER
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strncpy
#endif

using namespace std::placeholders;

// -- ClientConnectionState -----
class ClientConnectionState
{
public:
	ClientConnectionState(	
		MikanClientInfo& clientInfo,
		IInterprocessMessageServer* messageServer)
		: m_messageServer(messageServer)
	{	
		m_connectionInfo.clientInfo= clientInfo;
		m_connectionInfo.renderTargetReadAccessor= new InterprocessRenderTargetReadAccessor(clientInfo.clientId);
	}

	virtual ~ClientConnectionState()
	{
		freeRenderTargetBuffers();
		delete m_connectionInfo.renderTargetReadAccessor;
	}

	const MikanClientConnectionInfo& getClientConnectionInfo() const 
	{
		return m_connectionInfo;
	}
	
	const std::string& getClientId() const 
	{
		return m_connectionInfo.clientId;
	}

	const MikanClientInfo& getMikanClientInfo() const 
	{
	
		return m_connectionInfo.clientInfo;
	}

	bool readRenderTarget()
	{
		EASY_FUNCTION();

		return m_connectionInfo.renderTargetReadAccessor->readRenderTargetMemory();
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
	
	bool allocateRenderTargetBuffers(const MikanRenderTargetDescriptor& desc)
	{
		EASY_FUNCTION();

		freeRenderTargetBuffers();

		if (m_connectionInfo.renderTargetReadAccessor->initialize(&desc))
		{
			MikanServer* mikanServer= MikanServer::getInstance();

			if (mikanServer->OnClientRenderTargetAllocated)
			{
				mikanServer->OnClientRenderTargetAllocated(
					m_connectionInfo.clientId, 
					m_connectionInfo.clientInfo, 
					m_connectionInfo.renderTargetReadAccessor);
			}

			return true;
		}

		return false;
	}

	void freeRenderTargetBuffers()
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
					m_connectionInfo.clientId,
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
		m_messageServer->sendMessageToClient(getClientId(), mikanTypeToJsonString(mikanEvent));
	}

	// Scripting Events
	void publishScriptMessageEvent(const std::string& message)
	{
		MikanScriptMessageInfo messageInfo;
		messageInfo.content = message;

		m_messageServer->sendMessageToClient(getClientId(), mikanTypeToJsonString(messageInfo));
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
		m_messageServer->sendMessageToClient(getClientId(), mikanTypeToJsonString(newFrameEvent));
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
				glm::mat4 xform= vrDeviceView->getCalibrationPose();

				// Send a pose update to the client
				MikanVRDevicePoseUpdateEvent poseUpdate;
				poseUpdate.transform= glm_mat4_to_MikanMatrix4f(xform);
				poseUpdate.device_id= deviceId;
				poseUpdate.frame= newVRFrameIndex;

				m_messageServer->sendMessageToClient(getClientId(), mikanTypeToJsonString(poseUpdate));
			}
		}
	}

	void publishVRDeviceListChangedEvent()
	{
		publishSimpleEvent<MikanVRDeviceListUpdateEvent>();
	}

	// Spatial Anchor Events
	void publishAnchorPoseUpdatedEvent(const MikanAnchorPoseUpdateEvent& newPoseEvent)
	{
		m_messageServer->sendMessageToClient(getClientId(), mikanTypeToJsonString(newPoseEvent));
	}

	void publishAnchorListChangedEvent()
	{
		publishSimpleEvent<MikanAnchorListUpdateEvent>();
	}

private:
	MikanClientConnectionInfo m_connectionInfo;
	IInterprocessMessageServer* m_messageServer;
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
void publishSimpleEvent(std::map<std::string, class ClientConnectionState*>& clientConnections)
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
	m_messageServer->setRequestHandler("allocateRenderTargetBuffers", std::bind(&MikanServer::allocateRenderTargetBuffers, this, _1, _2));
	m_messageServer->setRequestHandler("freeRenderTargetBuffers", std::bind(&MikanServer::freeRenderTargetBuffers, this, _1, _2));
	m_messageServer->setRequestHandler("frameRendered", std::bind(&MikanServer::frameRendered, this, _1, _2));	
	m_messageServer->setRequestHandler("getStencilList", std::bind(&MikanServer::getStencilList, this, _1, _2));
	m_messageServer->setRequestHandler("getQuadStencil", std::bind(&MikanServer::getQuadStencil, this, _1, _2));
	m_messageServer->setRequestHandler("getBoxStencil", std::bind(&MikanServer::getBoxStencil, this, _1, _2));
	m_messageServer->setRequestHandler("getModelStencil", std::bind(&MikanServer::getModelStencil, this, _1, _2));
	m_messageServer->setRequestHandler("getSpatialAnchorList", std::bind(&MikanServer::getSpatialAnchorList, this, _1, _2));
	m_messageServer->setRequestHandler("getSpatialAnchorInfo", std::bind(&MikanServer::getSpatialAnchorInfo, this, _1, _2));
	m_messageServer->setRequestHandler("findSpatialAnchorInfoByName", std::bind(&MikanServer::findSpatialAnchorInfoByName, this, _1, _2));

	VRDeviceManager::getInstance()->OnDeviceListChanged += MakeDelegate(this, &MikanServer::publishVRDeviceListChanged);
	VRDeviceManager::getInstance()->OnDevicePosesChanged += MakeDelegate(this, &MikanServer::publishVRDevicePoses);

	AnchorObjectSystem::getSystem()->getAnchorSystemConfig()->OnMarkedDirty+= 
		MakeDelegate(this, &MikanServer::handleAnchorSystemConfigChange);

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

	for (auto& connection_it : m_clientConnections)
	{
		delete connection_it.second;
	}
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
	if (changedPropertySet.hasPropertyName(SceneComponentDefinition::k_relativePositionPropertyId) ||
		changedPropertySet.hasPropertyName(SceneComponentDefinition::k_relativeRotationPropertyId) ||
		changedPropertySet.hasPropertyName(SceneComponentDefinition::k_relativeScalePropertyId))
	{
		AnchorDefinitionPtr anchorConfig= std::static_pointer_cast<AnchorDefinition>(configPtr);

		MikanAnchorPoseUpdateEvent poseUpdateEvent;
		memset(&poseUpdateEvent, 0, sizeof(MikanAnchorPoseUpdateEvent));
		poseUpdateEvent.anchor_id = anchorConfig->getAnchorId();
		poseUpdateEvent.transform = glm_transform_to_MikanTransform(anchorConfig->getRelativeTransform());

		publishAnchorPoseUpdatedEvent(poseUpdateEvent);

	}
	else if (changedPropertySet.hasPropertyName(AnchorObjectSystemConfig::k_anchorListPropertyId))
	{
		publishSimpleEvent<MikanAnchorListUpdateEvent>(m_clientConnections);
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
void writeTypedResponse(MikanRequestID requestId, t_mikan_type& result, std::string& utf8ResponseString)
{
	result.requestId= requestId;
	result.resultCode= MikanResult_Success;

	json j = result;

	utf8ResponseString= j.dump();
}

void writeSimpleResponse(MikanRequestID requestId, MikanResult result, std::string& utf8ResponseString)
{
	MikanResponse response;
	response.requestId= requestId;
	response.resultCode= result;

	json j= response;
	utf8ResponseString = j.dump();
}

void MikanServer::connectHandler(const ClientRequest& request, std::string& utf8ResponseString)
{
	MikanClientInfo clientInfo;
	if (readRequestPayload(request.utf8RequestString, clientInfo))
	{
		MIKAN_LOG_ERROR("connectHandler") << "Failed to parse client info";
		// TODO send error event
		return;
	}

	const std::string clientId = clientInfo.clientId;
	auto connection_it = m_clientConnections.find(clientId);
	if (connection_it == m_clientConnections.end())
	{
		ClientConnectionState* clientState = new ClientConnectionState(clientInfo, m_messageServer);

		m_clientConnections.insert({clientId, clientState});

		if (OnClientConnected)
		{
			OnClientConnected(clientId, clientInfo);
		}

		clientState->publishSimpleEvent<MikanConnectedEvent>();
	}
	else
	{
		//TODO: send error event
		MIKAN_LOG_ERROR("connectHandler") << "Client already connected: " << clientId;
	}
}

void MikanServer::disconnectHandler(const ClientRequest& request, std::string& utf8ResponseString)
{
	MikanClientInfo clientInfo;
	if (readRequestPayload(request.utf8RequestString, clientInfo))
	{
		MIKAN_LOG_ERROR("disconnectHandler") << "Failed to parse client info";
		// TODO send error event
		return;
	}

	auto connection_it = m_clientConnections.find(clientInfo.clientId);
	if (connection_it != m_clientConnections.end())
	{
		const std::string& clientId = connection_it->first;
		ClientConnectionState* clientState = connection_it->second;

		clientState->publishSimpleEvent<MikanDisconnectedEvent>();

		if (OnClientDisconnected)
		{
			OnClientDisconnected(clientId);
		}

		delete connection_it->second;
		m_clientConnections.erase(connection_it);
	}
	else
	{
		//TODO: send error event
		MIKAN_LOG_ERROR("disconnectHandler") << "Client not connected: " << clientInfo.clientId;
	}
}

void MikanServer::invokeScriptMessageHandler(
	const ClientRequest& request,
	std::string& utf8ResponseString)
{
	MikanScriptMessageInfo messageInfo;
	if (!readRequestPayload(request.utf8RequestString, messageInfo))
	{
		writeSimpleResponse(request.requestId, MikanResult_MalformedParameters, utf8ResponseString);
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

	writeSimpleResponse(request.requestId, MikanResult_Success, utf8ResponseString);
}

void MikanServer::getVideoSourceIntrinsics(
	const ClientRequest& request,
	std::string& utf8ResponseString)
{
	VideoSourceViewPtr videoSourceView= getCurrentVideoSource();

	if (videoSourceView)
	{
		MikanVideoSourceIntrinsics intrinsics;
		videoSourceView->getCameraIntrinsics(intrinsics);

		writeTypedResponse(request.requestId, intrinsics, utf8ResponseString);
	}
	else
	{
		writeSimpleResponse(request.requestId, MikanResult_NoVideoSource, utf8ResponseString);
	}
}

void MikanServer::getVideoSourceMode(
	const ClientRequest& request,
	std::string& utf8ResponseString)
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

		writeTypedResponse(request.requestId, info, utf8ResponseString);
	}
	else
	{
		writeSimpleResponse(request.requestId, MikanResult_NoVideoSource, utf8ResponseString);
	}
}

void MikanServer::getVideoSourceAttachment(
	const ClientRequest& request,
	std::string& utf8ResponseString)
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

			writeTypedResponse(request.requestId, info, utf8ResponseString);
		}
		else
		{
			writeSimpleResponse(request.requestId, MikanResult_NoVideoSource, utf8ResponseString);
		}
	}
	else
	{
		writeSimpleResponse(request.requestId, MikanResult_NoVideoSource, utf8ResponseString);
	}
}

void MikanServer::getVRDeviceList(
	const ClientRequest& request,
	std::string& utf8ResponseString)
{
	VRDeviceList deviceList= VRDeviceManager::getInstance()->getVRDeviceList();

	MikanVRDeviceList vrDeviceListResult= {};

	for (VRDeviceViewPtr deviceView : deviceList)
	{
		vrDeviceListResult.vr_device_id_list.push_back(deviceView->getDeviceID());
	}

	writeTypedResponse(request.requestId, vrDeviceListResult, utf8ResponseString);
}

void MikanServer::getVRDeviceInfo(
	const ClientRequest& request,
	std::string& utf8ResponseString)
{
	MikanVRDeviceID deviceId;
	if (!readRequestPayload(request.utf8RequestString, deviceId))
	{
		writeSimpleResponse(request.requestId, MikanResult_MalformedParameters, utf8ResponseString);
		return;
	}

	VRDeviceViewPtr vrDeviceView = VRDeviceManager::getInstance()->getVRDeviceViewById(deviceId);
	if (!vrDeviceView)
	{
		writeSimpleResponse(request.requestId, MikanResult_InvalidDeviceId, utf8ResponseString);
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

	writeTypedResponse(request.requestId, info, utf8ResponseString);
}

void MikanServer::subscribeToVRDevicePoseUpdates(
	const ClientRequest& request,
	std::string& utf8ResponseString)
{
	MikanVRDeviceID deviceId;
	if (!readRequestPayload(request.utf8RequestString, deviceId))
	{
		writeSimpleResponse(request.requestId, MikanResult_MalformedParameters, utf8ResponseString);
		return;
	}

	auto connection_it = m_clientConnections.find(request.clientId);
	if (connection_it == m_clientConnections.end())
	{
		writeSimpleResponse(request.requestId, MikanResult_UnknownClient, utf8ResponseString);
		return;
	}

	ClientConnectionState* clientState = connection_it->second;
	clientState->subscribeToVRDevicePoseUpdates(deviceId);
	writeSimpleResponse(request.requestId, MikanResult_Success, utf8ResponseString);
}

void MikanServer::unsubscribeFromVRDevicePoseUpdates(
	const ClientRequest& request,
	std::string& utf8ResponseString)
{
	MikanVRDeviceID deviceId;
	if (!readRequestPayload(request.utf8RequestString, deviceId))
	{
		writeSimpleResponse(request.requestId, MikanResult_MalformedParameters, utf8ResponseString);
		return;
	}

	auto connection_it = m_clientConnections.find(request.clientId);
	if (connection_it == m_clientConnections.end())
	{
		writeSimpleResponse(request.requestId, MikanResult_UnknownClient, utf8ResponseString);
		return;
	}

	ClientConnectionState* clientState = connection_it->second;
	clientState->unsubscribeFromVRDevicePoseUpdates(deviceId);
	writeSimpleResponse(request.requestId, MikanResult_Success, utf8ResponseString);
}

void MikanServer::allocateRenderTargetBuffers(
	const ClientRequest& request,
	std::string& utf8ResponseString)
{	
	MikanRenderTargetDescriptor desc;
	if (!readRequestPayload(request.utf8RequestString, desc))
	{
		writeSimpleResponse(request.requestId, MikanResult_MalformedParameters, utf8ResponseString);
		return;
	}

	auto connection_it = m_clientConnections.find(request.clientId);
	if (connection_it == m_clientConnections.end())
	{
		writeSimpleResponse(request.requestId, MikanResult_UnknownClient, utf8ResponseString);
		return;
	}

	ClientConnectionState* clientState = connection_it->second;
	if (clientState->allocateRenderTargetBuffers(desc))
	{
		writeSimpleResponse(request.requestId, MikanResult_Success, utf8ResponseString);
	}
	else
	{
		writeSimpleResponse(request.requestId, MikanResult_GeneralError, utf8ResponseString);
	}
}

void MikanServer::freeRenderTargetBuffers(
	const ClientRequest& request,
	std::string& utf8ResponseString)
{
	auto connection_it = m_clientConnections.find(request.clientId);
	if (connection_it != m_clientConnections.end())
	{
		if (OnClientRenderTargetReleased)
		{
			OnClientRenderTargetReleased(request.clientId, connection_it->second->getRenderTargetReadAccessor());
		}

		connection_it->second->freeRenderTargetBuffers();
		writeSimpleResponse(request.requestId, MikanResult_Success, utf8ResponseString);
	}
	else
	{
		writeSimpleResponse(request.requestId, MikanResult_UnknownClient, utf8ResponseString);
	}
}

void MikanServer::frameRendered(
	const ClientRequest& request,
	std::string& utf8ResponseString)
{
	uint64_t frameIndex= 0;
	if (!readRequestPayload(request.utf8RequestString, frameIndex))
	{
		writeSimpleResponse(request.requestId, MikanResult_MalformedParameters, utf8ResponseString);
		return;
	}

	auto connection_it = m_clientConnections.find(request.clientId);
	if (connection_it != m_clientConnections.end())
	{
		// Process incoming video frames, if we have a compositor active
		if (OnClientRenderTargetUpdated)
		{
			ClientConnectionState* clientState = connection_it->second;

			if (clientState->readRenderTarget())
			{
				OnClientRenderTargetUpdated(request.clientId, frameIndex);
			}
		}

		writeSimpleResponse(request.requestId, MikanResult_Success, utf8ResponseString);
	}
	else
	{
		writeSimpleResponse(request.requestId, MikanResult_UnknownClient, utf8ResponseString);
	}
}

void MikanServer::getStencilList(
	const ClientRequest& request,
	std::string& utf8ResponseString)
{
	MikanStencilList stencilListResult = {};

	auto stencilSystemConfig = App::getInstance()->getProfileConfig()->stencilConfig;
	for (QuadStencilDefinitionPtr quadConfig : stencilSystemConfig->quadStencilList)
	{
		stencilListResult.stencil_id_list.push_back(quadConfig->getStencilId());
	}

	writeTypedResponse(request.requestId, stencilListResult, utf8ResponseString);
}

void MikanServer::getQuadStencil(
	const ClientRequest& request,
	std::string& utf8ResponseString)
{
	MikanStencilID stencilId;
	if (!readRequestPayload(request.utf8RequestString, stencilId))
	{
		writeSimpleResponse(request.requestId, MikanResult_MalformedParameters, utf8ResponseString);
		return;
	}

	auto stencilSystemConfig = App::getInstance()->getProfileConfig()->stencilConfig;
	auto quadConfig= stencilSystemConfig->getQuadStencilConfigConst(stencilId);
	if (quadConfig != nullptr)
	{
		MikanStencilQuad stencil= quadConfig->getQuadInfo();

		writeTypedResponse(request.requestId, stencil, utf8ResponseString);
	}
	else
	{
		writeSimpleResponse(request.requestId, MikanResult_InvalidStencilID, utf8ResponseString);
	}
}

void MikanServer::getBoxStencil(
	const ClientRequest& request,
	std::string& utf8ResponseString)
{
	MikanStencilID stencilId;
	if (!readRequestPayload(request.utf8RequestString, stencilId))
	{
		writeSimpleResponse(request.requestId, MikanResult_MalformedParameters, utf8ResponseString);
		return;
	}

	auto stencilSystemConfig = App::getInstance()->getProfileConfig()->stencilConfig;
	auto boxConfig = stencilSystemConfig->getBoxStencilConfigConst(stencilId);
	if (boxConfig != nullptr)
	{
		MikanStencilBox stencil = boxConfig->getBoxInfo();

		writeTypedResponse(request.requestId, stencil, utf8ResponseString);
	}
	else
	{
		writeSimpleResponse(request.requestId, MikanResult_InvalidStencilID, utf8ResponseString);
	}
}

void MikanServer::getModelStencil(
	const ClientRequest& request,
	std::string& utf8ResponseString)
{
	MikanStencilID stencilId;
	if (!readRequestPayload(request.utf8RequestString, stencilId))
	{
		writeSimpleResponse(request.requestId, MikanResult_MalformedParameters, utf8ResponseString);
		return;
	}

	auto stencilSystemConfig = App::getInstance()->getProfileConfig()->stencilConfig;
	auto modelConfig = stencilSystemConfig->getModelStencilConfigConst(stencilId);
	if (modelConfig != nullptr)
	{
		MikanStencilModel stencil = modelConfig->getModelInfo();

		writeTypedResponse(request.requestId, stencil, utf8ResponseString);
	}
	else
	{
		writeSimpleResponse(request.requestId, MikanResult_InvalidStencilID, utf8ResponseString);
	}
}

void MikanServer::getSpatialAnchorList(
	const ClientRequest& request,
	std::string& utf8ResponseString)
{
	MikanSpatialAnchorList anchorListResult= {};

	auto anchorSystemConfig = App::getInstance()->getProfileConfig()->anchorConfig;
	for (AnchorDefinitionPtr spatialAnchor : anchorSystemConfig->spatialAnchorList)
	{
		anchorListResult.spatial_anchor_id_list.push_back(spatialAnchor->getAnchorId());
	}

	writeTypedResponse(request.requestId, anchorListResult, utf8ResponseString);
}

void MikanServer::getSpatialAnchorInfo(
	const ClientRequest& request,
	std::string& utf8ResponseString)
{
	MikanSpatialAnchorID anchorId;
	if (!readRequestPayload(request.utf8RequestString, anchorId))
	{
		writeSimpleResponse(request.requestId, MikanResult_MalformedParameters, utf8ResponseString);
		return;
	}

	AnchorComponentPtr anchorPtr= AnchorObjectSystem::getSystem()->getSpatialAnchorById(anchorId);
	if (anchorPtr == nullptr)
	{
		writeSimpleResponse(request.requestId, MikanResult_InvalidAnchorID, utf8ResponseString);
		return;
	}
	
	MikanSpatialAnchorInfo anchorInfo;
	anchorPtr->extractAnchorInfoForClientAPI(anchorInfo);

	writeTypedResponse(request.requestId, anchorInfo, utf8ResponseString);
}

void MikanServer::findSpatialAnchorInfoByName(
	const ClientRequest& request,
	std::string& utf8ResponseString)
{
	std::string nameBuffer;
	if (!readRequestPayload(request.utf8RequestString, nameBuffer))
	{
		writeSimpleResponse(request.requestId, MikanResult_MalformedParameters, utf8ResponseString);
		return;
	}

	AnchorComponentPtr anchorPtr = AnchorObjectSystem::getSystem()->getSpatialAnchorByName(nameBuffer);
	if (anchorPtr == nullptr)
	{
		writeSimpleResponse(request.requestId, MikanResult_InvalidAnchorID, utf8ResponseString);
		return;
	}

	MikanSpatialAnchorInfo anchorInfo;
	anchorPtr->extractAnchorInfoForClientAPI(anchorInfo);

	writeTypedResponse(request.requestId, anchorInfo, utf8ResponseString);
}