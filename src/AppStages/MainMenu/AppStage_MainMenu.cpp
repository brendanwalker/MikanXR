//-- inludes -----
#include "Compositor/AppStage_Compositor.h"
#include "MainMenu/AppStage_MainMenu.h"
#include "CalibrationPatternSettings/AppStage_CalibrationPatternSettings.h"
#include "CameraSettings/AppStage_CameraSettings.h"
#include "VRDeviceSettings/AppStage_VRDeviceSettings.h"
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
AppStage_MainMenu::AppStage_MainMenu(MainWindow* ownerWindow)
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
		m_ownerWindow->pushAppStage<AppStage_Compositor>();
	}
	else if (value == "goto_pattern_settings")
	{
		m_ownerWindow->pushAppStage<AppStage_CalibrationPatternSettings>();
	}
	else if (value == "goto_camera_settings")
	{
		m_ownerWindow->pushAppStage<AppStage_CameraSettings>();
	}
	else if (value == "goto_vr_device_settings")
	{
		m_ownerWindow->pushAppStage<AppStage_VRDeviceSettings>();
	}
	else if (value == "exit_mikan")
	{
		App::getInstance()->requestShutdown();
	}
}