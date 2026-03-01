#pragma once

//-- includes -----
#include "AppStage.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "RmlFwd.h"

//-- definitions -----
class AppStage_MainMenu : public AppStage
{
public:
	AppStage_MainMenu(class IEditorWindow* ownerWindow);

	virtual void enter() override;

	static const char* APP_STAGE_NAME;

protected:
	void onResumeProject();
	void onOpenProject();
	void onNewProject();
	void onTutorial();
	void onExit();

	// -- IRemoteControllableAppStage Interface -- //
	virtual bool handleRemoteControlCommand(
		const std::string& command,
		const std::vector<std::string>& parameters,
		std::vector<std::string>& outResults);
	bool handleResumeProjectCommand(std::vector<std::string>& outResults);
	bool handleOpenProjectCommand(const std::vector<std::string>& parameters, std::vector<std::string>& outResults);
	bool handleNewProjectCommand(const std::vector<std::string>& parameters, std::vector<std::string>& outResults);
	bool handleTutorialCommand(std::vector<std::string>& outResults);
	bool handleExitCommand(std::vector<std::string>& outResults);

private:
	AppSettingsConfigPtr m_appSettingsConfig;
	ProjectManagerPtr m_projectManager;
};
