#include "App.h"
#include "AppStage.h"
#include "VRDeviceRequestHandler.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MikanServer.h"
#include "MikanClientConnectionState.h"
#include "MikanVideoSourceEvents.h"
#include "MikanVideoSourceRequests.h"
#include "MikanVRDeviceRequests.h"
#include "ServerResponseHelpers.h"
#include "VRDeviceManager.h"
#include "VRDeviceView.h"

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
	
	VRDeviceManager* vrDeviceManager = VRDeviceManager::getInstance();

	for (auto deviceId : m_subscribedVRDevices)
	{
		VRDeviceViewPtr vrDeviceView = vrDeviceManager->getVRDeviceViewById(deviceId);

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

				m_owner->publishMikanJsonEvent(mikanTypeToJsonString(poseUpdate));
			}
		}
	}
}

// -- VRDeviceRequestHandler -- //
bool VRDeviceRequestHandler::startup(MainWindow* mainWindow)
{
	auto* vrDeviceManager= VRDeviceManager::getInstance();
	IInterprocessMessageServer* messageServer = m_owner->getMessageServer();

	vrDeviceManager->OnDeviceListChanged
		+= MakeDelegate(this, &VRDeviceRequestHandler::publishVRDeviceListChanged);
	vrDeviceManager->OnDevicePosesChanged
		+= MakeDelegate(this, &VRDeviceRequestHandler::publishVRDevicePoses);

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
	auto* vrDeviceManager= VRDeviceManager::getInstance();

	vrDeviceManager->OnDeviceListChanged
		-= MakeDelegate(this, &VRDeviceRequestHandler::publishVRDeviceListChanged);
	vrDeviceManager->OnDevicePosesChanged
		-= MakeDelegate(this, &VRDeviceRequestHandler::publishVRDevicePoses);
}

void VRDeviceRequestHandler::publishVRDeviceListChanged()
{
	MikanVRDeviceListUpdateEvent listChangedEvent = {};

	m_owner->publishMikanJsonEvent(mikanTypeToJsonString(listChangedEvent));
}

void VRDeviceRequestHandler::publishVRDevicePoses(int64_t newFrameIndex)
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
	VRDeviceList deviceList = VRDeviceManager::getInstance()->getVRDeviceList();

	MikanVRDeviceListResponse vrDeviceListResult = {};
	for (VRDeviceViewPtr deviceView : deviceList)
	{
		vrDeviceListResult.vr_device_id_list.push_back(deviceView->getDeviceID());
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

	VRDeviceViewPtr vrDeviceView = VRDeviceManager::getInstance()->getVRDeviceViewById(deviceRequest.deviceId);
	if (!vrDeviceView)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidDeviceId, response);
		return;
	}

	MikanVRDeviceInfoResponse infoResponse = {};
	MikanVRDeviceInfo& info = infoResponse.vr_device_info;
	info.device_path = vrDeviceView->getDevicePath();

	switch (vrDeviceView->getVRTrackerDriverType())
	{
		case IVRDeviceInterface::eDriverType::SteamVR:
			info.vr_device_api = MikanVRDeviceApi_STEAM_VR;
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