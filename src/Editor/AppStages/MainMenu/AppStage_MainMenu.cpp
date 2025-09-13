//-- inludes -----
#include "Project/AppStage_Project.h"
#include "MainMenu/AppStage_MainMenu.h"
#include "App.h"
#include "MainWindow.h"
#include "PathUtils.h"
#include "Logger.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Debugger.h>

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

	addRmlDocument("main_menu.rml");
}

void AppStage_MainMenu::onRmlClickEvent(const std::string& value)
{
	if (value == "goto_compositor")
	{
		m_ownerWindow->pushAppStageOfType<AppStage_Project>();
	}
	else if (value == "exit_mikan")
	{
		App::getInstance()->requestShutdown();
	}
}