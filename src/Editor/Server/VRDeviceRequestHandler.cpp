#include "App.h"
#include "AppStage.h"
#include "IVRDevice.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MikanServer.h"
#include "MikanClientConnectionState.h"
#include "MikanVideoSourceEvents.h"
#include "MikanVideoSourceRequests.h"
#include "MikanVRDeviceRequests.h"
#include "ServerResponseHelpers.h"
#include "VRObjectSystem.h"
#include "VRDeviceComponent.h"
#include "VRDeviceMath.h"
#include "VRDeviceRequestHandler.h"

#include <functional>

using namespace std::placeholders;

// -- VRDeviceClientState -- //
VRDeviceClientState::VRDeviceClientState(class MikanClientConnectionState* owner)
	: m_owner(owner)
{}

void VRDeviceClientState::subscribeToVRDevicePoseUpdatesHandler(MikanVRDeviceID deviceId)
{
	m_subscribedVRDevices.insert(deviceId);
}

void VRDeviceClientState::unsubscribeFromVRDevicePoseUpdatesHandler(MikanVRDeviceID deviceId)
{
	auto it = m_subscribedVRDevices.find(deviceId);
	if (it != m_subscribedVRDevices.end())
	{
		m_subscribedVRDevices.erase(it);
	}
}

void VRDeviceClientState::publishVRDevicePoses(int64_t newVRFrameIndex)
{	
	auto vrObjectSystem = VRObjectSystem::getSystem();

	for (auto deviceId : m_subscribedVRDevices)
	{
		VRDeviceComponentPtr vrDeviceComponent = vrObjectSystem->getVRDeviceById(deviceId);

		if (vrDeviceComponent && vrDeviceComponent->getIsPoseValid())
		{
			VRDevicePose devicePose;
			if (vrDeviceComponent->getDevicePose(devicePose))
			{
				GlmTransform glmTransform= VRDevicePose_to_GlmTransform(devicePose);

				// Send a pose update to the client
				MikanVRDevicePoseUpdateEvent poseUpdate;
				poseUpdate.transform = glm_mat4_to_MikanMatrix4f(glmTransform.getMat4());
				poseUpdate.device_id = deviceId;
				poseUpdate.frame = newVRFrameIndex;

				m_owner->publishMikanJsonEvent(mikanTypeToJsonString(poseUpdate));
			}
		}
	}
}

// -- VRDeviceRequestHandler -- //
bool VRDeviceRequestHandler::startup(MainWindow* mainWindow)
{
	IInterprocessMessageServer* messageServer = m_owner->getMessageServer();

	// Listen for VR Device events directly from the vr device manager interface
	auto vrObjectSystem = VRObjectSystem::getSystem();
	IVRDeviceManagerPtr vrDeviceManager= vrObjectSystem->getVRDeviceManager();
	if (vrDeviceManager)
	{
		vrDeviceManager->addListener(this);
	}
	// TODO: listen for VR System API changes

	// VR Device Requests
	messageServer->setRequestHandler(
		GetVRDeviceList::staticGetArchetype().getId(),
		std::bind(&VRDeviceRequestHandler::getVRDeviceListHandler, this, _1, _2));
	messageServer->setRequestHandler(
		GetVRDeviceInfo::staticGetArchetype().getId(),
		std::bind(&VRDeviceRequestHandler::getVRDeviceInfoHandler, this, _1, _2));
	messageServer->setRequestHandler(
		SubscribeToVRDevicePoseUpdates::staticGetArchetype().getId(),
		std::bind(&VRDeviceRequestHandler::subscribeToVRDevicePoseUpdatesHandler, this, _1, _2));
	messageServer->setRequestHandler(
		UnsubscribeFromVRDevicePoseUpdates::staticGetArchetype().getId(),
		std::bind(&VRDeviceRequestHandler::unsubscribeFromVRDevicePoseUpdatesHandler, this, _1, _2));

	return true;
}

void VRDeviceRequestHandler::shutdown()
{
	auto vrObjectSystem = VRObjectSystem::getSystem();
	IVRDeviceManagerPtr vrDeviceManager = vrObjectSystem->getVRDeviceManager();
	if (vrDeviceManager)
	{
		vrDeviceManager->removeListener(this);
	}
}

void VRDeviceRequestHandler::onActiveDeviceListChanged()
{
	MikanVRDeviceListUpdateEvent listChangedEvent = {};

	m_owner->publishMikanJsonEvent(mikanTypeToJsonString(listChangedEvent));
}

void VRDeviceRequestHandler::onDevicePropertyChanged(int deviceId)
{

}

void VRDeviceRequestHandler::onDevicePosesChanged(int64_t newFrameIndex)
{
	std::vector<MikanClientConnectionStateConstPtr> clienStatetList;
	m_owner->getConnectedClientStateList(clienStatetList);

	for (MikanClientConnectionStateConstPtr& clientStatePtr : clienStatetList)
	{
		clientStatePtr->getVRDeviceClientState()->publishVRDevicePoses(newFrameIndex);
	}
}

void VRDeviceRequestHandler::getVRDeviceListHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	MikanVRDeviceListResponse vrDeviceListResult = {};
	auto vrObjectSystem = VRObjectSystem::getSystem();
	for (const auto& kvpair : vrObjectSystem->getVRDeviceMap())
	{
		VRDeviceComponentPtr deviceComponent= kvpair.second.lock();

		if (deviceComponent)
		{
			vrDeviceListResult.vr_device_id_list.push_back(
				deviceComponent->getVRDeviceDefinition()->getVRDeviceId());
		}
	}

	writeTypedJsonResponse(request.requestId, vrDeviceListResult, response);
}

void VRDeviceRequestHandler::getVRDeviceInfoHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	GetVRDeviceInfo deviceRequest;
	if (!readTypedRequest(request.utf8RequestString, deviceRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	auto vrObjectSystem = VRObjectSystem::getSystem();
	VRDeviceComponentPtr vrDeviceComponent= vrObjectSystem->getVRDeviceById(deviceRequest.deviceId);
	if (!vrDeviceComponent)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidDeviceId, response);
		return;
	}

	MikanVRDeviceInfoResponse infoResponse = {};
	MikanVRDeviceInfo& info = infoResponse.vr_device_info;

	IVRDevice* vrDeviceInterface= vrDeviceComponent->getVRDeviceInterface();
	info.device_path = vrDeviceInterface->getDevicePath();

	switch (vrObjectSystem->getVRSystemConfigConst()->getTrackingRuntimeType())
	{
		case eTrackingRuntime::SteamVR:
			info.vr_device_api = MikanVRDeviceApi_STEAM_VR;
			break;
		default:
			info.vr_device_api = MikanVRDeviceApi_INVALID;
	}

	switch (vrDeviceInterface->getDeviceType())
	{
		case eVRDeviceType::HMD:
			info.vr_device_type = MikanVRDeviceType_HMD;
			break;
		case eVRDeviceType::VRController:
			info.vr_device_type = MikanVRDeviceType_CONTROLLER;
			break;
		case eVRDeviceType::VRTracker:
			info.vr_device_type = MikanVRDeviceType_TRACKER;
			break;
		default:
			info.vr_device_type = MikanVRDeviceType_INVALID;
	}

	writeTypedJsonResponse(request.requestId, infoResponse, response);
}

void VRDeviceRequestHandler::subscribeToVRDevicePoseUpdatesHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	SubscribeToVRDevicePoseUpdates deviceRequest;
	if (!readTypedRequest(request.utf8RequestString, deviceRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	MikanClientConnectionStatePtr clientState= m_owner->getConnectedClientState(request.connectionId);
	if (!clientState)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::UnknownClient, response);
		return;
	}

	clientState->getVRDeviceClientState()->subscribeToVRDevicePoseUpdatesHandler(deviceRequest.deviceId);
	writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
}

void VRDeviceRequestHandler::unsubscribeFromVRDevicePoseUpdatesHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	UnsubscribeFromVRDevicePoseUpdates deviceRequest;
	if (!readTypedRequest(request.utf8RequestString, deviceRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	MikanClientConnectionStatePtr clientState = m_owner->getConnectedClientState(request.connectionId);
	if (!clientState)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::UnknownClient, response);
		return;
	}

	clientState->getVRDeviceClientState()->unsubscribeFromVRDevicePoseUpdatesHandler(deviceRequest.deviceId);
	writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
}