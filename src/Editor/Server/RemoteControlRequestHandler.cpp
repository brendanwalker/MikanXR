#include "App.h"
#include "AppStage.h"
#include "MainWindow.h"
#include "MikanServer.h"
#include "MikanRemoteControlRequests.h"
#include "RemoteControlRequestHandler.h"
#include "ServerResponseHelpers.h"

#include <functional>

using namespace std::placeholders;

bool RemoteControlRequestHandler::startup()
{
	IInterprocessMessageServer* messageServer= m_owner->getMessageServer();

	messageServer->setRequestHandler(
		GotoAppStage::staticGetArchetype().getId(),
		std::bind(&RemoteControlRequestHandler::gotoAppStageHandler, this, _1, _2));
	messageServer->setRequestHandler(
		GetAppStageInfo::staticGetArchetype().getId(),
		std::bind(&RemoteControlRequestHandler::getAppStageInfoHandler, this, _1, _2));

	return true;
}

void RemoteControlRequestHandler::gotoAppStageHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	GotoAppStage appStageRequest;
	if (!readTypedRequest(request.utf8RequestString, appStageRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	// TODO Look up the app stage factory by name and allocate (if not already active)

	writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
}

void RemoteControlRequestHandler::getAppStageInfoHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	GetAppStageInfo getAppStageRequest;
	if (!readTypedRequest(request.utf8RequestString, getAppStageRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}
	
	AppStage* currentAppStage= App::getInstance()->getMainWindow()->getCurrentAppStage();

	MikanAppStageInfoResponse appStageInfoResult;
	appStageInfoResult.app_stage_info.app_state_name = currentAppStage->getAppStageName();

	writeTypedJsonResponse(request.requestId, appStageInfoResult, response);
}