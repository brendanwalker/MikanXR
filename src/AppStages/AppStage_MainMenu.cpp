//-- inludes -----
#include "AppStage_Compositor.h"
#include "AppStage_MainMenu.h"
#include "AppStage_CalibrationPatternSettings.h"
#include "AppStage_CameraSettings.h"
#include "AppStage_VRDeviceSettings.h"
#include "App.h"
#include "PathUtils.h"
#include "Renderer.h"
#include "Logger.h"

#include "imgui.h"

#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>
#include <RmlUi/Core/ElementDocument.h>
#include <RmlUi/Debugger.h>

//-- statics ----
const char* AppStage_MainMenu::APP_STAGE_NAME = "MainMenu";

//-- public methods -----
AppStage_MainMenu::AppStage_MainMenu(App* app)
	: AppStage(app, AppStage_MainMenu::APP_STAGE_NAME)
{ 
}

void AppStage_MainMenu::enter()
{
	AppStage::enter();

	pushRmlDocument("rml\\main_menu.rml");
}

void AppStage_MainMenu::onRmlClickEvent(const std::string& value)
{
	if (value == "goto_compositor")
	{
		m_app->pushAppStage<AppStage_Compositor>();
	}
	else if (value == "goto_pattern_settings")
	{
		m_app->pushAppStage<AppStage_CalibrationPatternSettings>();
	}
	else if (value == "goto_camera_settings")
	{
		m_app->pushAppStage<AppStage_CameraSettings>();
	}
	else if (value == "goto_vr_device_settings")
	{
		m_app->pushAppStage<AppStage_VRDeviceSettings>();
	}
	else if (value == "exit_mikan")
	{
		m_app->requestShutdown();
	}
}