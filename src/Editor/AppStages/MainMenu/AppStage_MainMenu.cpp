//-- inludes -----
#include "Project/AppStage_Project.h"
#include "MainMenu/AppStage_MainMenu.h"
#include "MainMenu/RmlModel_MainMenu.h"
#include "ProjectManager.h"
#include "App.h"
#include "AppSettingsConfig.h"
#include "MainWindow.h"
#include "PathUtils.h"
#include "Logger.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Debugger.h>

#include "tinyfiledialogs.h"

//-- statics ----
const char* AppStage_MainMenu::APP_STAGE_NAME = "MainMenu";

//-- public methods -----
AppStage_MainMenu::AppStage_MainMenu(IEditorWindow* ownerWindow)
	: AppStage(ownerWindow, AppStage_MainMenu::APP_STAGE_NAME)
{ 
}

void AppStage_MainMenu::enter()
{
	AppStage::enter();

	App* app = App::getInstance();
	m_appSettingsConfig= app->getAppSettings();
	m_projectManager = app->getMainWindow()->getProjectManager();

	// Create app stage UI models and views
	// (Auto cleaned up on app state exit)
	{
		Rml::Context* context = getRmlContext();

		// Init calibration model
		auto* mainMenuModel = addRmlModel<RmlModel_MainMenu>();
		mainMenuModel->init(context, m_appSettingsConfig);
		mainMenuModel->OnResumeProject = MakeDelegate(this, &AppStage_MainMenu::onResumeProject);
		mainMenuModel->OnOpenProject = MakeDelegate(this, &AppStage_MainMenu::onOpenProject);
		mainMenuModel->OnNewProject = MakeDelegate(this, &AppStage_MainMenu::onNewProject);
		mainMenuModel->OnTutorial = MakeDelegate(this, &AppStage_MainMenu::onTutorial);
		mainMenuModel->OnExit = MakeDelegate(this, &AppStage_MainMenu::onExit);

		// Init calibration view now that the dependent model has been created
		addRmlDocument("main_menu.rml");
	}
}

void AppStage_MainMenu::onResumeProject()
{
	std::vector<std::string> outResults;
	handleResumeProjectCommand(outResults);
}

void AppStage_MainMenu::onOpenProject()
{
	std::string defaultPath = (PathUtils::getHomeDirectory() / "").string();
	static const char* filterItems[1] = { "*.mikanproj" };
	std::filesystem::path projectFilePath =
		tinyfd_openFileDialog(
			"Open Project",
			defaultPath.c_str(),
			1,
			filterItems,
			"Project Files (*.mikanproj)",
			1);

	std::vector<std::string> parameters = { projectFilePath.string() };
	std::vector<std::string> outResults;
	handleOpenProjectCommand(parameters, outResults);
}

void AppStage_MainMenu::onNewProject()
{
	std::string defaultPath = (PathUtils::getHomeDirectory() / "").string();
	static const char* filterItems[1] = { "*.mikanproj" };
	std::filesystem::path projectFilePath =
		tinyfd_saveFileDialog(
			"New Project",
			defaultPath.c_str(),
			1,
			filterItems,
			"Project Files (*.mikanproj)");

	std::vector<std::string> parameters = { projectFilePath.string() };
	std::vector<std::string> outResults;
	handleNewProjectCommand(parameters, outResults);
}

void AppStage_MainMenu::onTutorial()
{
	std::vector<std::string> outResults;
	handleTutorialCommand(outResults);
}

void AppStage_MainMenu::onExit()
{
	std::vector<std::string> outResults;
	handleExitCommand(outResults);
}

// -- IRemoteControllableAppStage Interface -- //
bool AppStage_MainMenu::handleRemoteControlCommand(
	const std::string& command,
	const std::vector<std::string>& parameters,
	std::vector<std::string>& outResults)
{
	if (command == "resume_project")
	{
		return handleResumeProjectCommand(outResults);
	}
	else if (command == "open_project")
	{
		return handleOpenProjectCommand(parameters, outResults);
	}
	else if (command == "new_project")
	{
		return handleNewProjectCommand(parameters, outResults);
	}
	else if (command == "tutorial")
	{
		return handleTutorialCommand(outResults);
	}
	else if (command == "exit")
	{
		return handleExitCommand(outResults);
	}

	return AppStage::handleRemoteControlCommand(command, parameters, outResults);
}

bool AppStage_MainMenu::handleResumeProjectCommand(std::vector<std::string>& outResults)
{
	if (m_projectManager->hasLoadedProject())
	{
		m_ownerWindow->pushAppStageOfType<AppStage_Project>();
		outResults.push_back(IRemoteControllable::k_success);
	}
	else if (m_appSettingsConfig->hasLastProjectPath())
	{
		std::filesystem::path projectFilePath = m_appSettingsConfig->getLastProjectPath();

		if (m_projectManager->loadProject(projectFilePath.string()))
		{
			m_ownerWindow->pushAppStageOfType<AppStage_Project>();
			outResults.push_back(IRemoteControllable::k_success);
		}
	}

	outResults.push_back(IRemoteControllable::k_failure);
	return true;
}

bool AppStage_MainMenu::handleOpenProjectCommand(
	const std::vector<std::string>& parameters, 
	std::vector<std::string>& outResults)
{
	std::string projectFilePathStr= !parameters.empty() ? parameters[0] : "";

	if (!projectFilePathStr.empty())
	{
		if (m_projectManager->loadProject(projectFilePathStr))
		{
			// Remember the last opened project path
			m_appSettingsConfig->setLastProjectPath(projectFilePathStr);

			m_ownerWindow->pushAppStageOfType<AppStage_Project>();

			outResults.push_back(IRemoteControllable::k_success);
			return true;
		}
		else
		{
			outResults.push_back(IRemoteControllable::k_failure);
			return true;
		}
	}

	outResults.push_back(IRemoteControllable::k_failure);
	return false;
}

bool AppStage_MainMenu::handleNewProjectCommand(
	const std::vector<std::string>& parameters,
	std::vector<std::string>& outResults)
{
	if (!parameters.empty())
	{
		const std::string& projectFilePathStr = parameters[0];

		if (m_projectManager->newProject(projectFilePathStr))
		{
			// Remember the last opened project path
			m_appSettingsConfig->setLastProjectPath(projectFilePathStr);

			m_ownerWindow->pushAppStageOfType<AppStage_Project>();
			outResults.push_back(IRemoteControllable::k_success);
		}
		else
		{
			outResults.push_back(IRemoteControllable::k_failure);
		}

		return true;
	}

	outResults.push_back(IRemoteControllable::k_failure);
	return false;
}

bool AppStage_MainMenu::handleTutorialCommand(std::vector<std::string>& outResults)
{
	//TODO
	outResults.push_back(IRemoteControllable::k_failure);
	return false;
}

bool AppStage_MainMenu::handleExitCommand(std::vector<std::string>& outResults)
{
	App::getInstance()->requestShutdown();
	outResults.push_back(IRemoteControllable::k_success);

	return true;
}