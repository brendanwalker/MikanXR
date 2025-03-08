#pragma once

#include "IServerRequestHandler.h"

#include <map>
#include <memory>
#include <string>

class RemoteControlManager : public IServerRequestHandler
{
public:
	RemoteControlManager(class MikanServer* owner) : IServerRequestHandler(owner) {}

	virtual bool startup(class MainWindow* mainWindow) override;

	void sendRemoteControlEvent(
		const std::string& event,
		const std::vector<std::string>& parameters);

protected:
	void pushAppStageHandler(const ClientRequest& request, ClientResponse& response);
	void popAppStageHandler(const ClientRequest& request, ClientResponse& response);
	void getAppStageInfoHandler(const ClientRequest& request, ClientResponse& response);
	void remoteControlCommandHandler(const ClientRequest& request, ClientResponse& response);

	// App Events
	void onAppStageEntered(class AppStage* oldAppStage, class AppStage* newAppStage);
	void onAppStageExited(class AppStage* oldAppStage, class AppStage* newAppStage);
	void publishAppStageChangedEvent(const std::string& oldAppStageName, const std::string& newAppStageName);

	using RemoteControllableAppStageFactoryPtr = std::shared_ptr<class RemoteControllableAppStageFactory>;
	std::map<std::string, RemoteControllableAppStageFactoryPtr> m_remoteControllableAppStageFactories;
};