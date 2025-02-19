#include "App.h"
#include "AppStage.h"
#include "IRemoteControllableAppStage.h"
#include "MainWindow.h"
#include "MikanServer.h"
#include "MikanRemoteControlEvents.h"
#include "MikanRemoteControlRequests.h"
#include "RemoteControlManager.h"
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
bool RemoteControlManager::startup(MainWindow* mainWindow)
{
	IInterprocessMessageServer* messageServer= m_owner->getMessageServer();

	// Register remote control request handlers
	messageServer->setRequestHandler(
		PushAppStage::staticGetArchetype().getId(),
		std::bind(&RemoteControlManager::pushAppStageHandler, this, _1, _2));
	messageServer->setRequestHandler(
		PopAppStage::staticGetArchetype().getId(),
		std::bind(&RemoteControlManager::popAppStageHandler, this, _1, _2));
	messageServer->setRequestHandler(
		GetAppStageInfo::staticGetArchetype().getId(),
		std::bind(&RemoteControlManager::getAppStageInfoHandler, this, _1, _2));
	messageServer->setRequestHandler(
		MikanRemoteControlCommand::staticGetArchetype().getId(),
		std::bind(&RemoteControlManager::remoteControlCommandHandler, this, _1, _2));

	// Create the factories for the remote-controllable app stages
	m_remoteControllableAppStageFactories[AppStage_AlignmentCalibration::APP_STAGE_NAME]=
		TypedRemoteControllableAppStageFactory<AppStage_AlignmentCalibration>::createFactory();
	m_remoteControllableAppStageFactories[AppStage_MonoLensCalibration::APP_STAGE_NAME] =
		TypedRemoteControllableAppStageFactory<AppStage_MonoLensCalibration>::createFactory();
	m_remoteControllableAppStageFactories[AppStage_VRTrackingRecenter::APP_STAGE_NAME] =
		TypedRemoteControllableAppStageFactory<AppStage_VRTrackingRecenter>::createFactory();

	mainWindow->OnAppStageEntered += MakeDelegate(this, &RemoteControlManager::onAppStageEntered);

	return true;
}

void RemoteControlManager::pushAppStageHandler(
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

void RemoteControlManager::popAppStageHandler(
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

void RemoteControlManager::getAppStageInfoHandler(
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

void RemoteControlManager::remoteControlCommandHandler(
	const ClientRequest& request, 
	ClientResponse& response)
{
	MikanRemoteControlCommand remoteControlCommand;
	if (!readTypedRequest(request.utf8RequestString, remoteControlCommand))
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::MalformedParameters, response);
		return;
	}

	// Get the current app stage and check if it is remote controllable
	MainWindow* mainWindow= App::getInstance()->getMainWindow();
	AppStage* appStage= mainWindow->getCurrentAppStage();
	auto* remoteControllableAppStage= dynamic_cast<IRemoteControllableAppStage*>(appStage);

	if (remoteControllableAppStage != nullptr)
	{
		// Pull args out of Serialization types
		const std::string& command = remoteControlCommand.command.getValue();
		std::vector<std::string> parameters;
		for (const auto& parameter : remoteControlCommand.parameters)
		{
			parameters.push_back(parameter.getValue());
		}

		// Pass the command to the app stage
		// If the command is not recognized/supported, return an error
		std::vector<std::string> results;
		if (remoteControllableAppStage->handleRemoteControlCommand(command, parameters, results))
		{
			MikanRemoteControlCommandResult commandResponse= {};
			const size_t resultCount = results.size();
			if (resultCount > 0)
			{
				commandResponse.results.resize(resultCount);
				for (size_t i = 0; i < resultCount; i++)
				{
					commandResponse.results[i].setValue(results[i]);
				}
			}

			writeTypedJsonResponse(request.requestId, commandResponse, response);
		}
		else
		{
			writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidParam, response);
		}
	}
	else
	{
		writeSimpleJsonResponse(request.requestId, MikanAPIResult::InvalidParam, response);
	}
}

// -- App Events ----
void RemoteControlManager::onAppStageEntered(AppStage* oldAppStage, AppStage* newAppStage)
{
	// If the new app stage is remote-controllable, 
	// store a reference to the remote control manager on it so that post reote control events
	auto remoteControllable= dynamic_cast<IRemoteControllableAppStage*>(newAppStage);
	if (remoteControllable != nullptr)
	{
		remoteControllable->setRemoteControlManager(this);
	}

	publishAppStageChangedEvent(
		oldAppStage != nullptr ? oldAppStage->getAppStageName() : "",
		newAppStage != nullptr ? newAppStage->getAppStageName() : "");
}

void RemoteControlManager::onAppStageExited(AppStage* oldAppStage, AppStage* newAppStage)
{
	publishAppStageChangedEvent(
		oldAppStage != nullptr ? oldAppStage->getAppStageName() : "",
		newAppStage != nullptr ? newAppStage->getAppStageName() : "");
}

void RemoteControlManager::publishAppStageChangedEvent(
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

void RemoteControlManager::sendRemoteControlEvent(
	const std::string& event,
	const std::vector<std::string>& parameters)
{
	MikanRemoteControlEvent remoteControlEvent = {};
	remoteControlEvent.remoteControlEvent.setValue(event);

	const size_t parameterCount = parameters.size();
	if (parameterCount > 0)
	{
		remoteControlEvent.parameters.resize(parameterCount);

		for (size_t i = 0; i < parameterCount; i++)
		{
			remoteControlEvent.parameters[i].setValue(parameters[i]);
		}
	}

	std::string jsonStr;
	Serialization::serializeToJsonString(remoteControlEvent, jsonStr);
	m_owner->publishMikanJsonEvent(jsonStr);
}