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
	, m_mainMenuModel(new RmlModel_MainMenu)
{ 
}

AppStage_MainMenu::~AppStage_MainMenu()
{
	delete m_mainMenuModel;
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
		m_mainMenuModel->init(context, m_appSettingsConfig);
		m_mainMenuModel->OnResumeProject = MakeDelegate(this, &AppStage_MainMenu::onResumeProject);
		m_mainMenuModel->OnOpenProject = MakeDelegate(this, &AppStage_MainMenu::onOpenProject);
		m_mainMenuModel->OnNewProject = MakeDelegate(this, &AppStage_MainMenu::onNewProject);
		m_mainMenuModel->OnTutorial = MakeDelegate(this, &AppStage_MainMenu::onTutorial);
		m_mainMenuModel->OnExit = MakeDelegate(this, &AppStage_MainMenu::onExit);

		// Init calibration view now that the dependent model has been created
		m_mainMenuView = addRmlDocument("main_menu.rml");;
	}
}

void AppStage_MainMenu::onResumeProject()
{
	if (m_projectManager->hasLoadedProject())
	{
		m_ownerWindow->pushAppStageOfType<AppStage_Project>();
	}
	else if (m_appSettingsConfig->hasLastProjectPath())
	{
		std::filesystem::path projectFilePath = m_appSettingsConfig->getLastProjectPath();

		if (m_projectManager->loadProject(projectFilePath.string()))
		{
			m_ownerWindow->pushAppStageOfType<AppStage_Project>();
		}
	}	
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

	if (!projectFilePath.empty())
	{
		if (m_projectManager->loadProject(projectFilePath.string()))
		{
			// Remember the last opened project path
			m_appSettingsConfig->setLastProjectPath(projectFilePath);

			m_ownerWindow->pushAppStageOfType<AppStage_Project>();
		}
	}
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

	if (m_projectManager->newProject(projectFilePath.string()))
	{
		// Remember the last opened project path
		m_appSettingsConfig->setLastProjectPath(projectFilePath);

		m_ownerWindow->pushAppStageOfType<AppStage_Project>();
	}
}

void AppStage_MainMenu::onTutorial()
{
	//TODO
}

void AppStage_MainMenu::onExit()
{
	App::getInstance()->requestShutdown();
}