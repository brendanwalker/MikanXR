#include "App.h"
#include "AppStage.h"
#include "IRemoteControllableAppStage.h"
#include "MainWindow.h"
#include "MikanServer.h"
#include "MikanRemoteControlEvents.h"
#include "MikanRemoteControlRequests.h"
#include "RemoteControlRequestHandler.h"
#include "ServerResponseHelpers.h"

// Remote Controllable AppStages
#include "AlignmentCalibration/AppStage_AlignmentCalibration.h"
#include "MonoLensCalibration/AppStage_MonoLensCalibration.h"
#include "VRTrackingRecenter/AppStage_VRTrackingRecenter.h"

#include <functional>

using namespace std::placeholders;

template <typename t_app_stage_class>
class TypedRemoteControllableAppStageFactory : public RemoteControllableAppStageFactory
{
public:
	using RemoteControllableAppStageFactoryPtr = std::shared_ptr<RemoteControllableAppStageFactory>;

	virtual std::string getAppStageName() override
	{
		return t_app_stage_class::APP_STAGE_NAME;	
	}

	virtual IRemoteControllableAppStage* pushAppStage() override
	{
		MainWindow* MainWindow= App::getInstance()->getMainWindow();

		return MainWindow->pushAppStage<t_app_stage_class>();
	}

	static RemoteControllableAppStageFactoryPtr createFactory()
	{
		return std::make_shared< TypedRemoteControllableAppStageFactory<t_app_stage_class> >();
	}
};

// -- RemoteControlRequestHandler -- //
bool RemoteControlRequestHandler::startup()
{
	IInterprocessMessageServer* messageServer= m_owner->getMessageServer();

	// Register remote control request handlers
	messageServer->setRequestHandler(
		PushAppStage::staticGetArchetype().getId(),
		std::bind(&RemoteControlRequestHandler::pushAppStageHandler, this, _1, _2));
	messageServer->setRequestHandler(
		PopAppStage::staticGetArchetype().getId(),
		std::bind(&RemoteControlRequestHandler::popAppStageHandler, this, _1, _2));
	messageServer->setRequestHandler(
		GetAppStageInfo::staticGetArchetype().getId(),
		std::bind(&RemoteControlRequestHandler::getAppStageInfoHandler, this, _1, _2));

	// Create the factories for the remote-controllable app stages
	m_remoteControllableAppStageFactories[AppStage_AlignmentCalibration::APP_STAGE_NAME]=
		TypedRemoteControllableAppStageFactory<AppStage_AlignmentCalibration>::createFactory();
	m_remoteControllableAppStageFactories[AppStage_MonoLensCalibration::APP_STAGE_NAME] =
		TypedRemoteControllableAppStageFactory<AppStage_MonoLensCalibration>::createFactory();
	m_remoteControllableAppStageFactories[AppStage_VRTrackingRecenter::APP_STAGE_NAME] =
		TypedRemoteControllableAppStageFactory<AppStage_VRTrackingRecenter>::createFactory();

	MainWindow* mainWindow= App::getInstance()->getMainWindow();
	mainWindow->OnAppStageEntered += MakeDelegate(this, &RemoteControlRequestHandler::onAppStageEntered);

	return true;
}

void RemoteControlRequestHandler::pushAppStageHandler(
	const ClientRequest& request,
	ClientResponse& response)
{
	PushAppStage appStageRequest;
	if (!readTypedRequest(request.utf8RequestString, appStageRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	const std::string& desiredAppStageName= appStageRequest.app_state_name.getValue();
	auto iter= m_remoteControllableAppStageFactories.find(desiredAppStageName);
	if (iter == m_remoteControllableAppStageFactories.end())
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidParam, response);
		return;
	}

	MainWindow* mainWindow= App::getInstance()->getMainWindow();
	if (mainWindow->getCurrentAppStage()->getAppStageName() != desiredAppStageName)
	{
		auto factory = iter->second;
		
		factory->pushAppStage();
	}

	writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
}

void RemoteControlRequestHandler::popAppStageHandler(
	const ClientRequest& request, 
	ClientResponse& response)
{
	PopAppStage appStageRequest;
	if (!readTypedRequest(request.utf8RequestString, appStageRequest))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	MainWindow* mainWindow= App::getInstance()->getMainWindow();
	if (mainWindow->getParentAppStage() != nullptr)
	{
		mainWindow->popAppState();
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::Success, response);
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidParam, response);
		return;
	}
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

// -- App Events ----
void RemoteControlRequestHandler::onAppStageEntered(AppStage* oldAppStage, AppStage* newAppStage)
{
	publishAppStageChangedEvent(
		oldAppStage->getAppStageName(),
		newAppStage->getAppStageName());
}

void RemoteControlRequestHandler::onAppStageExited(AppStage* oldAppStage, AppStage* newAppStage)
{
	publishAppStageChangedEvent(
		oldAppStage->getAppStageName(),
		newAppStage->getAppStageName());
}

void RemoteControlRequestHandler::publishAppStageChangedEvent(
	const std::string& oldAppStageName,
	const std::string& newAppStageName)
{
	MikanAppStageChagedEvent appStageChangedEvent = {};
	appStageChangedEvent.old_app_state_name.setValue(oldAppStageName);
	appStageChangedEvent.new_app_state_name.setValue(newAppStageName);

	std::string jsonStr;
	Serialization::serializeToJsonString(appStageChangedEvent, jsonStr);
	m_owner->publishMikanJsonEvent(jsonStr);
}