#include "App.h"
#include "AppStage.h"
#include "CameraComponent.h"
#include "CameraObjectSystem.h"
#include "CameraRequestHandler.h"
#include "MainWindow.h"
#include "MathTypeConversion.h"
#include "MikanServer.h"
#include "MikanCameraEvents.h"
#include "MikanCameraRequests.h"
#include "ProjectConfig.h"
#include "ServerResponseHelpers.h"

#include <functional>

using namespace std::placeholders;

bool CameraRequestHandler::startup(MainWindow* mainWindow)
{
	IInterprocessMessageServer* messageServer = m_owner->getMessageServer();

	// Camera Requests
	messageServer->setRequestHandler(
		GetCameraList::staticGetArchetype().getId(),
		std::bind(&CameraRequestHandler::getCameraListHandler, this, _1, _2));
	messageServer->setRequestHandler(
		FindCameraInfoByName::staticGetArchetype().getId(),
		std::bind(&CameraRequestHandler::findCameraByNameHandler, this, _1, _2));
	messageServer->setRequestHandler(
		GetCameraInfo::staticGetArchetype().getId(),
		std::bind(&CameraRequestHandler::getCameraInfoHandler, this, _1, _2));

	return true;
}

// Camera Events
void CameraRequestHandler::publishCameraNewFrameEvent(const MikanCameraNewFrameEvent& newFrameEvent)
{
	m_owner->publishMikanJsonEvent(mikanTypeToJsonString(newFrameEvent));
}

void CameraRequestHandler::publishCameraAttachmentChangedEvent()
{
	MikanCameraAttachmentChangedEvent attachmentChangedEvent= {};

	m_owner->publishMikanJsonEvent(mikanTypeToJsonString(attachmentChangedEvent));
}

// Camera Requests
void CameraRequestHandler::getCameraListHandler(
	const struct ClientRequest& request, 
	struct ClientResponse& response)
{
	MikanCameraListResponse cameraListResult = {};

	auto cameraSystemConfig = App::getInstance()->getProfileConfig()->cameraConfig;
	for (CameraDefinitionPtr cameraConfig : cameraSystemConfig->cameraList)
	{
		cameraListResult.camera_id_list.push_back(cameraConfig->getCameraId());
	}

	writeTypedJsonResponse(request.requestId, cameraListResult, response);
}

void CameraRequestHandler::findCameraByNameHandler(
	const struct ClientRequest& request, 
	struct ClientResponse& response)
{
	FindCameraInfoByName cameraRequest;
	if (!readTypedRequest(request.utf8RequestString, cameraRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	const std::string& cameraName = cameraRequest.camera_name.getValue();
	CameraComponentPtr cameraPtr = CameraObjectSystem::getSystem()->getCameraByName(cameraName);
	if (cameraPtr == nullptr)
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidAnchorID, response);
		return;
	}

	MikanCameraInfoResponse cameraInfoResponse = {};
	cameraInfoResponse.camera_info= cameraPtr->getCameraDefinition()->getCameraInfo();

	writeTypedJsonResponse(request.requestId, cameraInfoResponse, response);
}

void CameraRequestHandler::getCameraInfoHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	GetCameraInfo cameraInfoRequest;
	if (!readTypedRequest(request.utf8RequestString, cameraInfoRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	auto cameraSystemConfig = App::getInstance()->getProfileConfig()->cameraConfig;
	auto cameraConfig = cameraSystemConfig->getCameraConfig(cameraInfoRequest.camera_id);
	if (cameraConfig != nullptr)
	{
		MikanCameraInfoResponse cameraResponse = {};
		cameraResponse.camera_info = cameraConfig->getCameraInfo();

		writeTypedJsonResponse(request.requestId, cameraResponse, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidStencilID, response);
	}
}