//-- includes -----
#include "App.h"
#include "InterprocessRenderTargetReader.h"
#include "InterprocessMessages.h"
#include "MathTypeConversion.h"
#include "Logger.h"
#include "MikanServer.h"
#include "ProfileConfig.h"
#include "VideoCapabilitiesConfig.h"
#include "VideoSourceView.h"
#include "VideoSourceManager.h"
#include "VRDeviceManager.h"
#include "VRDeviceView.h"

#include <set>
#include <assert.h>

#include <easy/profiler.h>

#ifdef _MSC_VER
#pragma warning (disable: 4996) // 'This function or variable may be unsafe': strncpy
#endif

using namespace std::placeholders;

// -- ClientConnectionState -----
class ClientConnectionState
{
public:
	ClientConnectionState(	
		const std::string& clientName, 
		MikanClientInfo& clientInfo,
		InterprocessMessageServer* messageServer)
		: m_messageServer(messageServer)
	{	
		m_connectionInfo.clientId= clientName;
		m_connectionInfo.clientInfo= clientInfo;
		m_connectionInfo.renderTargetReadAccessor= new InterprocessRenderTargetReadAccessor(clientName);
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

	bool pollRenderTarget()
	{
		EASY_FUNCTION();

		return m_connectionInfo.renderTargetReadAccessor->readRenderTargetMemory();
	}

	InterprocessRenderTargetReadAccessor* getRenderTargetReadAccessor() const
	{
		return m_connectionInfo.renderTargetReadAccessor;
	}

	const MikanRenderTargetMemory& getLocalRenderTargetMemory() const
	{
		return m_connectionInfo.renderTargetReadAccessor->getLocalMemory();
	}

	MikanClientGraphicsApi getClientGraphicsAPI() const
	{
		return m_connectionInfo.renderTargetReadAccessor->getClientGraphicsAPI();
	}

	uint64_t getLocalRenderTargetFrameIndex() const 
	{
		return m_connectionInfo.renderTargetReadAccessor->getLocalFrameIndex();
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

	// Video Source Events
	void publishVideoSourceOpenedEvent()
	{
		publishSimpleEvent(MikanEvent_videoSourceOpened);
	}

	void publishVideoSourceClosedEvent()
	{
		publishSimpleEvent(MikanEvent_videoSourceClosed);
	}

	void publishNewVideoFrameEvent(const MikanVideoSourceNewFrameEvent& newFrameEvent)
	{
		EASY_FUNCTION();

		MikanEvent mikanEvent;
		memset(&mikanEvent, 0, sizeof(MikanEvent));
		mikanEvent.event_type = MikanEvent_videoSourceNewFrame;
		mikanEvent.event_payload.video_source_new_frame= newFrameEvent;

		m_messageServer->sendServerEventToClient(getClientId(), &mikanEvent);
	}

	void publishVideoSourceAttachmentChangedEvent()
	{
		publishSimpleEvent(MikanEvent_videoSourceAttachmentChanged);
	}

	void publishVideoSourceIntrinsicsChangedEvent()
	{
		publishSimpleEvent(MikanEvent_videoSourceIntrinsicsChanged);
	}

	void publishVideoSourceModeChangedEvent()
	{
		publishSimpleEvent(MikanEvent_videoSourceModeChanged);
	}

	// VR Device Events
	void publishVRDevicePoses(uint64_t newVRFrameIndex)
	{
		EASY_FUNCTION();

		VRDeviceManager* vrDeviceManager= VRDeviceManager::getInstance();

		for (auto deviceId : m_subscribedVRDevices)
		{
			VRDeviceViewPtr vrDeviceView= vrDeviceManager->getVRDeviceViewPtr(deviceId);

			if (vrDeviceView && vrDeviceView->getIsOpen() && vrDeviceView->getIsPoseValid())
			{
				// TODO: We should provide option to select which component we want the pose updates for
				glm::mat4 xform= vrDeviceView->getCalibrationPose();

				// Send a pose update to the client
				MikanEvent mikanEvent;
				memset(&mikanEvent, 0, sizeof(MikanEvent));
				mikanEvent.event_type = MikanEvent_vrDevicePoseUpdated;
				{
					MikanVRDevicePoseUpdateEvent& poseUpdate= mikanEvent.event_payload.vr_device_pose_updated;

					poseUpdate.transform= glm_mat4_to_MikanMatrix4f(xform);
					poseUpdate.device_id= deviceId;
					poseUpdate.frame= newVRFrameIndex;
				}				
				m_messageServer->sendServerEventToClient(getClientId(), &mikanEvent);
			}
		}
	}

	void publishVRDeviceListChangedEvent()
	{
		publishSimpleEvent(MikanEvent_vrDeviceListUpdated);
	}

	// Spatial Anchor Events
	void publishAnchorPoseUpdatedEvent(const MikanAnchorPoseUpdateEvent& newPoseEvent)
	{
		EASY_FUNCTION();

		MikanEvent mikanEvent;
		memset(&mikanEvent, 0, sizeof(MikanEvent));
		mikanEvent.event_type = MikanEvent_anchorPoseUpdated;
		mikanEvent.event_payload.anchor_pose_updated = newPoseEvent;

		m_messageServer->sendServerEventToClient(getClientId(), &mikanEvent);
	}

	void publishAnchorListChangedEvent()
	{
		publishSimpleEvent(MikanEvent_anchorListUpdated);
	}

	void publishSimpleEvent(MikanEventType eventType)
	{
		EASY_FUNCTION();

		MikanEvent mikanEvent;
		memset(&mikanEvent, 0, sizeof(MikanEvent));
		mikanEvent.event_type = eventType;

		m_messageServer->sendServerEventToClient(getClientId(), &mikanEvent);
	}

private:
	MikanClientConnectionInfo m_connectionInfo;
	InterprocessMessageServer* m_messageServer;
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
	: m_messageServer(new InterprocessMessageServer())
{
	m_instance= this;
}

MikanServer::~MikanServer()
{
	delete m_messageServer;
	m_instance= nullptr;
}

// -- ClientMikanAPI System -----
bool MikanServer::startup()
{
	EASY_FUNCTION();

	if (!m_messageServer->initialize())
	{
		MIKAN_LOG_ERROR("MikanServer::startup()") << "Failed to initialize interprocess message server";
		return false;
	}

	m_messageServer->setRPCHandler(CONNECT_FUNCTION_NAME, std::bind(&MikanServer::connect, this, _1, _2));
	m_messageServer->setRPCHandler(DISCONNECT_FUNCTION_NAME, std::bind(&MikanServer::disconnect, this, _1, _2));
	m_messageServer->setRPCHandler("getVideoSourceIntrinsics", std::bind(&MikanServer::getVideoSourceIntrinsics, this, _1, _2));
	m_messageServer->setRPCHandler("getVideoSourceMode", std::bind(&MikanServer::getVideoSourceMode, this, _1, _2));	
	m_messageServer->setRPCHandler("getVRDeviceList", std::bind(&MikanServer::getVRDeviceList, this, _1, _2));
	m_messageServer->setRPCHandler("getVideoSourceAttachment", std::bind(&MikanServer::getVideoSourceAttachment, this, _1, _2));
	m_messageServer->setRPCHandler("getVRDeviceInfo", std::bind(&MikanServer::getVRDeviceInfo, this, _1, _2));
	m_messageServer->setRPCHandler("subscribeToVRDevicePoseUpdates", std::bind(&MikanServer::subscribeToVRDevicePoseUpdates, this, _1, _2));
	m_messageServer->setRPCHandler("unsubscribeFromVRDevicePoseUpdates", std::bind(&MikanServer::unsubscribeFromVRDevicePoseUpdates, this, _1, _2));
	m_messageServer->setRPCHandler("allocateRenderTargetBuffers", std::bind(&MikanServer::allocateRenderTargetBuffers, this, _1, _2));
	m_messageServer->setRPCHandler("freeRenderTargetBuffers", std::bind(&MikanServer::freeRenderTargetBuffers, this, _1, _2));
	m_messageServer->setRPCHandler("getStencilList", std::bind(&MikanServer::getStencilList, this, _1, _2));
	m_messageServer->setRPCHandler("getQuadStencil", std::bind(&MikanServer::getQuadStencil, this, _1, _2));
	m_messageServer->setRPCHandler("getBoxStencil", std::bind(&MikanServer::getBoxStencil, this, _1, _2));
	m_messageServer->setRPCHandler("getModelStencil", std::bind(&MikanServer::getModelStencil, this, _1, _2));
	m_messageServer->setRPCHandler("getSpatialAnchorList", std::bind(&MikanServer::getSpatialAnchorList, this, _1, _2));
	m_messageServer->setRPCHandler("getSpatialAnchorInfo", std::bind(&MikanServer::getSpatialAnchorInfo, this, _1, _2));
	m_messageServer->setRPCHandler("findSpatialAnchorInfoByName", std::bind(&MikanServer::findSpatialAnchorInfoByName, this, _1, _2));	

	VRDeviceManager::getInstance()->OnDeviceListChanged += MakeDelegate(this, &MikanServer::publishVRDeviceListChanged);
	VRDeviceManager::getInstance()->OnDevicePosesChanged += MakeDelegate(this, &MikanServer::publishVRDevicePoses);

	return true;
}

void MikanServer::update()
{
	EASY_FUNCTION();

	// Process incoming function calls from clients
	{
		EASY_BLOCK("processRemoteFunctionCalls");

		m_messageServer->processRemoteFunctionCalls();
	}

	// Process incoming video frames, if we have a compositor active
	if (OnClientRenderTargetUpdated)
	{
		EASY_BLOCK("pollAllRenderTargets");

		for (auto& connection_it : m_clientConnections)
		{
			ClientConnectionState* connection= connection_it.second;

			if (connection->pollRenderTarget())
			{
				const std::string clientId= connection->getClientId();
				const uint64_t frameIndex= connection->getLocalRenderTargetFrameIndex();
				const MikanClientGraphicsApi api= connection->getClientGraphicsAPI();

				OnClientRenderTargetUpdated(clientId, frameIndex);
			}
		}
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

// Video Source Events
void MikanServer::publishVideoSourceOpenedEvent()
{
	publishSimpleEvent(MikanEvent_videoSourceOpened);
}

void MikanServer::publishVideoSourceClosedEvent()
{
	publishSimpleEvent(MikanEvent_videoSourceClosed);
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
	publishSimpleEvent(MikanEvent_videoSourceAttachmentChanged);
}

void MikanServer::publishVideoSourceIntrinsicsChangedEvent()
{
	publishSimpleEvent(MikanEvent_videoSourceIntrinsicsChanged);
}

void MikanServer::publishVideoSourceModeChangedEvent()
{
	publishSimpleEvent(MikanEvent_videoSourceModeChanged);
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

void MikanServer::publishAnchorListChangedEvent()
{
	publishSimpleEvent(MikanEvent_anchorListUpdated);
}

// VRManager Callbacks
void MikanServer::publishVRDeviceListChanged()
{
	publishSimpleEvent(MikanEvent_vrDeviceListUpdated);
}

void MikanServer::publishVRDevicePoses(uint64_t newFrameIndex)
{
	for (auto& connection_it : m_clientConnections)
	{
		connection_it.second->publishVRDevicePoses(newFrameIndex);
	}
}

void MikanServer::publishSimpleEvent(MikanEventType eventType)
{
	EASY_FUNCTION();

	for (auto& connection_it : m_clientConnections)
	{
		connection_it.second->publishSimpleEvent(eventType);
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

void MikanServer::getRelevantQuadStencilList(
	const std::vector<std::string>* allowedStencilNames,
	const glm::vec3& cameraPosition,
	const glm::vec3& cameraForward,
	std::vector<const MikanStencilQuad*>& outStencilList) const
{
	const ProfileConfig* profile = App::getInstance()->getProfileConfig();

	outStencilList.clear();
	for (const MikanStencilQuad& stencil : profile->quadStencilList)
	{
		if (stencil.is_disabled)
			continue;

		// If there is an active allow list, make sure stencil is on it
		if (allowedStencilNames != nullptr && allowedStencilNames->size() > 0)
		{
			if (std::find(
					allowedStencilNames->begin(), allowedStencilNames->end(), 
					stencil.stencil_name) 
				== allowedStencilNames->end())
			{
				continue;
			}
		}

		{
			const glm::mat4 worldXform= profile->getQuadStencilWorldTransform(&stencil);
			const glm::vec3 stencilCenter= glm::vec3(worldXform[3]); // position is 3rd column
			const glm::vec3 stencilForward= glm::vec3(worldXform[2]); // forward is 2nd column
			const glm::vec3 cameraToStencil= stencilCenter - cameraPosition;

			// Stencil is in front of the camera
			// Stencil is facing the camera (or double sided)
			if (glm::dot(cameraToStencil, cameraForward) > 0.f && 
				(stencil.is_double_sided || glm::dot(stencilForward, cameraForward) < 0.f))
			{
				outStencilList.push_back(&stencil);
			}
		}
	}
}

void MikanServer::getRelevantBoxStencilList(
	const std::vector<std::string>* allowedStencilNames,
	const glm::vec3& cameraPosition,
	const glm::vec3& cameraForward,
	std::vector<const MikanStencilBox*>& outStencilList) const
{
	const ProfileConfig* profile = App::getInstance()->getProfileConfig();

	outStencilList.clear();
	for (const MikanStencilBox& stencil : profile->boxStencilList)
	{
		if (stencil.is_disabled)
			continue;

		// If there is an active allow list, make sure stencil is on it
		if (allowedStencilNames != nullptr && allowedStencilNames->size() > 0)
		{
			if (std::find(
				allowedStencilNames->begin(), allowedStencilNames->end(),
				stencil.stencil_name)
				== allowedStencilNames->end())
			{
				continue;
			}
		}
		
		{
			const glm::mat4 worldXform = profile->getBoxStencilWorldTransform(&stencil);
			const glm::vec3 stencilCenter = glm::vec3(worldXform[3]); // position is 3rd column
			const glm::vec3 stencilZAxis = glm::vec3(worldXform[2]); // Z is 2nd column
			const glm::vec3 stencilYAxis = glm::vec3(worldXform[1]); // Y is 1st column
			const glm::vec3 stencilXAxis = glm::vec3(worldXform[0]); // X is 0th column
			const glm::vec3 cameraToStencil = stencilCenter - cameraPosition;

			const bool bIsStencilInFrontOfCamera= glm::dot(cameraToStencil, cameraForward) > 0.f;
			const bool bIsCameraInStecil=
				fabsf(glm::dot(cameraToStencil, stencilXAxis)) <= stencil.box_x_size &&
				fabsf(glm::dot(cameraToStencil, stencilYAxis)) <= stencil.box_y_size &&
				fabsf(glm::dot(cameraToStencil, stencilZAxis)) <= stencil.box_z_size;

			if (bIsStencilInFrontOfCamera || bIsCameraInStecil)
			{
				outStencilList.push_back(&stencil);
			}
		}
	}
}

void MikanServer::getRelevantModelStencilList(
	const std::vector<std::string>* allowedStencilNames,
	std::vector<const MikanStencilModelConfig*>& outStencilList) const
{
	const ProfileConfig* profile = App::getInstance()->getProfileConfig();

	outStencilList.clear();
	for (const MikanStencilModelConfig& stencil : profile->modelStencilList)
	{
		if (stencil.modelInfo.is_disabled)
			continue;

		if (stencil.modelPath.c_str() == 0)
			continue;

		// If there is an active allow list, make sure stencil is on it
		if (allowedStencilNames != nullptr && allowedStencilNames->size() > 0)
		{
			if (std::find(
				allowedStencilNames->begin(), allowedStencilNames->end(),
				stencil.modelInfo.stencil_name)
				== allowedStencilNames->end())
			{
				continue;
			}
		}

		outStencilList.push_back(&stencil);
	}
}

static VideoSourceViewPtr getCurrentVideoSource()
{
	const ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

	return VideoSourceListIterator(profileConfig->videoSourcePath).getCurrent();
}

static VRDeviceViewPtr getCurrentCameraVRDevice()
{
	const ProfileConfig* profileConfig = App::getInstance()->getProfileConfig();

	return VRDeviceListIterator(eDeviceType::VRTracker, profileConfig->cameraVRDevicePath).getCurrent();
}

void MikanServer::connect(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult)
{
	const std::string clientId= inFunctionCall->getClientId();

	MikanClientInfo clientInfo;
	if (inFunctionCall->extractParameters(clientInfo))
	{
		auto connection_it = m_clientConnections.find(clientId);

		if (connection_it == m_clientConnections.end())
		{
			ClientConnectionState* clientState= new ClientConnectionState(clientId, clientInfo, m_messageServer);

			m_clientConnections.insert({ clientId, clientState });

			if (OnClientConnected)
			{
				OnClientConnected(clientId, clientInfo);
			}
		}
	}
}

void MikanServer::disconnect(const class MikanRemoteFunctionCall* inFunctionCall, class MikanRemoteFunctionResult* outResult)
{
	const std::string clientId = inFunctionCall->getClientId();

	auto connection_it = m_clientConnections.find(clientId);
	if (connection_it != m_clientConnections.end())
	{
		const std::string& clientId= connection_it->first;

		if (OnClientDisconnected)
		{
			OnClientDisconnected(clientId);
		}

		delete connection_it->second;
		m_clientConnections.erase(connection_it);
	}
}

void MikanServer::getVideoSourceIntrinsics(
	const MikanRemoteFunctionCall* inFunctionCall, 
	MikanRemoteFunctionResult* outResult)
{
	VideoSourceViewPtr videoSourceView= getCurrentVideoSource();

	if (videoSourceView)
	{
		MikanVideoSourceIntrinsics intrinsics;
		videoSourceView->getCameraIntrinsics(intrinsics);

		outResult->setResultBuffer((uint8_t *)&intrinsics, sizeof(MikanVideoSourceIntrinsics));
		outResult->setResultCode(MikanResult_Success);
	}
	else
	{
		outResult->setResultCode(MikanResult_NoSelectedCamera);
	}
}

void MikanServer::getVideoSourceMode(
	const MikanRemoteFunctionCall* inFunctionCall,
	MikanRemoteFunctionResult* outResult)
{
	VideoSourceViewPtr videoSourceView = getCurrentVideoSource();

	if (videoSourceView)
	{
		const std::string devicePath= videoSourceView->getUSBDevicePath();
		const IVideoSourceInterface::eDriverType driverType= videoSourceView->getVideoSourceDriverType();
		const VideoModeConfig* modeConfig= videoSourceView->getVideoMode();

		MikanVideoSourceMode info;
		memset(&info, 0, sizeof(MikanVideoSourceMode));
		strncpy(info.device_path, devicePath.c_str(), sizeof(info.device_path)-1);
		info.frame_rate = modeConfig->frameRate;
		info.resolution_x = modeConfig->bufferPixelWidth;
		info.resolution_y = modeConfig->bufferPixelHeight;
		strncpy(info.video_mode_name, modeConfig->modeName.c_str(), sizeof(info.video_mode_name)-1);
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

		outResult->setResultBuffer((uint8_t*)&info, sizeof(MikanVideoSourceMode));
		outResult->setResultCode(MikanResult_Success);
	}
	else
	{
		outResult->setResultCode(MikanResult_NoSelectedCamera);
	}
}

void MikanServer::getVideoSourceAttachment(
	const MikanRemoteFunctionCall* inFunctionCall,
	MikanRemoteFunctionResult* outResult)
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

			// Get the camera parent anchor properties
			{
				ProfileConfig* profile = App::getInstance()->getProfileConfig();

				info.parent_anchor_id = profile->cameraParentAnchorId;
				info.camera_scale = profile->cameraScale;
			}

			outResult->setResultBuffer((uint8_t*)&info, sizeof(MikanVideoSourceAttachmentInfo));
			outResult->setResultCode(MikanResult_Success);
		}
		else
		{
			outResult->setResultCode(MikanResult_NoCameraAssignedTracker);
		}
	}
	else
	{
		outResult->setResultCode(MikanResult_NoSelectedCamera);
	}
}

void MikanServer::getVRDeviceList(
	const MikanRemoteFunctionCall* inFunctionCall,
	class MikanRemoteFunctionResult* outResult)
{
	VRDeviceList deviceList= VRDeviceManager::getInstance()->getVRDeviceList();

	MikanVRDeviceList vrDeviceListResult;
	memset(&vrDeviceListResult, 0, sizeof(MikanVRDeviceList));

	for (VRDeviceViewPtr deviceView : deviceList)
	{
		vrDeviceListResult.vr_device_id_list[vrDeviceListResult.vr_device_count]= deviceView->getDeviceID();
		++vrDeviceListResult.vr_device_count;

		assert(vrDeviceListResult.vr_device_count < MAX_MIKAN_VR_DEVICES);
		if (vrDeviceListResult.vr_device_count >= MAX_MIKAN_VR_DEVICES)
		{
			break;
		}
	}

	outResult->setResultBuffer((uint8_t*)&vrDeviceListResult, sizeof(MikanVRDeviceList));
	outResult->setResultCode(MikanResult_Success);
}

void MikanServer::getVRDeviceInfo(
	const class MikanRemoteFunctionCall* inFunctionCall, 
	class MikanRemoteFunctionResult* outResult)
{
	MikanVRDeviceID deviceId;
	if (!inFunctionCall->extractParameters(deviceId))
	{
		outResult->setResultCode(MikanResult_MalformedParameters);
		return;
	}

	VRDeviceViewPtr vrDeviceView = VRDeviceManager::getInstance()->getVRDeviceViewPtr(deviceId);
	if (!vrDeviceView)
	{
		outResult->setResultCode(MikanResult_InvalidDeviceID);
		return;
	}

	MikanVRDeviceInfo info;
	memset(&info, 0, sizeof(MikanVRDeviceInfo));

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

	std::string devicePath= vrDeviceView->getDevicePath();
	strncpy(info.device_path, devicePath.c_str(), sizeof(info.device_path) - 1);

	outResult->setResultBuffer((uint8_t*)&info, sizeof(MikanVRDeviceInfo));
	outResult->setResultCode(MikanResult_Success);
}

void MikanServer::subscribeToVRDevicePoseUpdates(
	const class MikanRemoteFunctionCall* inFunctionCall,
	class MikanRemoteFunctionResult* outResult)
{
	const std::string clientId = inFunctionCall->getClientId();

	MikanVRDeviceID deviceId;
	if (!inFunctionCall->extractParameters(deviceId))
	{
		outResult->setResultCode(MikanResult_MalformedParameters);
		return;
	}

	auto connection_it = m_clientConnections.find(clientId);
	if (connection_it == m_clientConnections.end())
	{
		outResult->setResultCode(MikanResult_UnknownClient);
		return;
	}

	ClientConnectionState* clientState = connection_it->second;
	clientState->subscribeToVRDevicePoseUpdates(deviceId);
	outResult->setResultCode(MikanResult_Success);
}

void MikanServer::unsubscribeFromVRDevicePoseUpdates(
	const class MikanRemoteFunctionCall* inFunctionCall,
	class MikanRemoteFunctionResult* outResult)
{
	const std::string clientId = inFunctionCall->getClientId();

	MikanVRDeviceID deviceId;
	if (!inFunctionCall->extractParameters(deviceId))
	{
		outResult->setResultCode(MikanResult_MalformedParameters);
		return;
	}

	auto connection_it = m_clientConnections.find(clientId);
	if (connection_it == m_clientConnections.end())
	{
		outResult->setResultCode(MikanResult_UnknownClient);
		return;
	}

	ClientConnectionState* clientState = connection_it->second;
	clientState->unsubscribeFromVRDevicePoseUpdates(deviceId);
	outResult->setResultCode(MikanResult_Success);
}

void MikanServer::allocateRenderTargetBuffers(
	const class MikanRemoteFunctionCall* inFunctionCall, 
	class MikanRemoteFunctionResult* outResult)
{
	const std::string clientId = inFunctionCall->getClientId();

	MikanRenderTargetDescriptor desc;
	if (!inFunctionCall->extractParameters(desc))
	{
		outResult->setResultCode(MikanResult_MalformedParameters);
		return;
	}

	auto connection_it = m_clientConnections.find(clientId);
	if (connection_it == m_clientConnections.end())
	{
		outResult->setResultCode(MikanResult_UnknownClient);
		return;
	}

	ClientConnectionState* clientState = connection_it->second;
	if (clientState->allocateRenderTargetBuffers(desc))
	{
		outResult->setResultCode(MikanResult_Success);
	}
	else
	{
		outResult->setResultCode(MikanResult_GeneralError);
	}
}

void MikanServer::freeRenderTargetBuffers(
	const class MikanRemoteFunctionCall* inFunctionCall, 
	class MikanRemoteFunctionResult* outResult)
{
	const std::string clientId = inFunctionCall->getClientId();

	auto connection_it = m_clientConnections.find(clientId);
	if (connection_it != m_clientConnections.end())
	{
		if (OnClientRenderTargetReleased)
		{
			OnClientRenderTargetReleased(clientId, connection_it->second->getRenderTargetReadAccessor());
		}

		connection_it->second->freeRenderTargetBuffers();
		outResult->setResultCode(MikanResult_Success);
	}
	else
	{
		outResult->setResultCode(MikanResult_UnknownClient);
	}
}

void MikanServer::getStencilList(
	const MikanRemoteFunctionCall* inFunctionCall,
	MikanRemoteFunctionResult* outResult)
{
	MikanStencilList stencilListResult;
	memset(&stencilListResult, 0, sizeof(MikanStencilList));

	const ProfileConfig* profile = App::getInstance()->getProfileConfig();
	for (const MikanStencilQuad& stencil : profile->quadStencilList)
	{
		stencilListResult.stencil_id_list[stencilListResult.stencil_count] = stencil.stencil_id;
		++stencilListResult.stencil_count;

		assert(stencilListResult.stencil_count < MAX_MIKAN_STENCILS);
		if (stencilListResult.stencil_count >= MAX_MIKAN_STENCILS)
		{
			break;
		}
	}

	outResult->setResultBuffer((uint8_t*)&stencilListResult, sizeof(MikanStencilList));
	outResult->setResultCode(MikanResult_Success);
}

void MikanServer::getQuadStencil(
	const MikanRemoteFunctionCall* inFunctionCall,
	MikanRemoteFunctionResult* outResult)
{
	MikanStencilQuad stencil;
	if (!inFunctionCall->extractParameters(stencil))
	{
		outResult->setResultCode(MikanResult_MalformedParameters);
		return;
	}

	const ProfileConfig* profile = App::getInstance()->getProfileConfig();
	if (profile->getQuadStencilInfo(stencil.stencil_id, stencil))
	{
		outResult->setResultBuffer((uint8_t*)&stencil, sizeof(MikanStencilQuad));
		outResult->setResultCode(MikanResult_Success);
	}
	else
	{
		outResult->setResultCode(MikanResult_InvalidStencilID);
	}
}

void MikanServer::getBoxStencil(
	const MikanRemoteFunctionCall* inFunctionCall,
	MikanRemoteFunctionResult* outResult)
{
	MikanStencilBox stencil;
	if (!inFunctionCall->extractParameters(stencil))
	{
		outResult->setResultCode(MikanResult_MalformedParameters);
		return;
	}

	const ProfileConfig* profile = App::getInstance()->getProfileConfig();
	if (profile->getBoxStencilInfo(stencil.stencil_id, stencil))
	{
		outResult->setResultBuffer((uint8_t*)&stencil, sizeof(MikanStencilBox));
		outResult->setResultCode(MikanResult_Success);
	}
	else
	{
		outResult->setResultCode(MikanResult_InvalidStencilID);
	}
}

void MikanServer::getModelStencil(
	const MikanRemoteFunctionCall* inFunctionCall,
	MikanRemoteFunctionResult* outResult)
{
	MikanStencilModel stencil;
	if (!inFunctionCall->extractParameters(stencil))
	{
		outResult->setResultCode(MikanResult_MalformedParameters);
		return;
	}

	const ProfileConfig* profile = App::getInstance()->getProfileConfig();
	if (profile->getModelStencilInfo(stencil.stencil_id, stencil))
	{
		outResult->setResultBuffer((uint8_t*)&stencil, sizeof(MikanStencilModel));
		outResult->setResultCode(MikanResult_Success);
	}
	else
	{
		outResult->setResultCode(MikanResult_InvalidStencilID);
	}
}

void MikanServer::getSpatialAnchorList(
	const MikanRemoteFunctionCall* inFunctionCall, 
	MikanRemoteFunctionResult* outResult)
{
	MikanSpatialAnchorList anchorListResult;
	memset(&anchorListResult, 0, sizeof(MikanSpatialAnchorList));

	const ProfileConfig* profile= App::getInstance()->getProfileConfig();
	for (const MikanSpatialAnchorInfo& spatialAnchor : profile->spatialAnchorList)
	{
		anchorListResult.spatial_anchor_id_list[anchorListResult.spatial_anchor_count] = spatialAnchor.anchor_id;
		++anchorListResult.spatial_anchor_count;

		assert(anchorListResult.spatial_anchor_count < MAX_MIKAN_SPATIAL_ANCHORS);
		if (anchorListResult.spatial_anchor_count >= MAX_MIKAN_SPATIAL_ANCHORS)
		{
			break;
		}
	}

	outResult->setResultBuffer((uint8_t*)&anchorListResult, sizeof(MikanSpatialAnchorList));
	outResult->setResultCode(MikanResult_Success);
}

void MikanServer::getSpatialAnchorInfo(
	const MikanRemoteFunctionCall* inFunctionCall, 
	MikanRemoteFunctionResult* outResult)
{
	MikanSpatialAnchorID anchorId;
	if (!inFunctionCall->extractParameters(anchorId))
	{
		outResult->setResultCode(MikanResult_MalformedParameters);
		return;
	}

	MikanSpatialAnchorInfo info;
	if (!App::getInstance()->getProfileConfig()->getSpatialAnchorInfo(anchorId, info))
	{
		outResult->setResultCode(MikanResult_InvalidDeviceID);
		return;
	}
	
	outResult->setResultBuffer((uint8_t*)&info, sizeof(MikanSpatialAnchorInfo));
	outResult->setResultCode(MikanResult_Success);
}

void MikanServer::findSpatialAnchorInfoByName(
	const MikanRemoteFunctionCall* inFunctionCall,
	MikanRemoteFunctionResult* outResult)
{
	char nameBuffer[MAX_MIKAN_ANCHOR_NAME_LEN];
	if (!inFunctionCall->extractParameters(nameBuffer))
	{
		outResult->setResultCode(MikanResult_MalformedParameters);
		return;
	}

	MikanSpatialAnchorInfo info;
	if (!App::getInstance()->getProfileConfig()->findSpatialAnchorInfoByName(nameBuffer, info))
	{
		outResult->setResultCode(MikanResult_InvalidDeviceID);
		return;
	}

	outResult->setResultBuffer((uint8_t*)&info, sizeof(MikanSpatialAnchorInfo));
	outResult->setResultCode(MikanResult_Success);
}