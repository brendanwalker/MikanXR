//-- inludes -----
#include "AppStage_Compositor.h"
#include "AppStage_MainMenu.h"
#include "AppStage_CalibrationPatternSettings.h"
#include "AppStage_CameraSettings.h"
#include "AppStage_VRDeviceSettings.h"
#include "App.h"
#include "Renderer.h"
#include "Logger.h"

#include "imgui.h"

//-- statics ----
const char* AppStage_MainMenu::APP_STAGE_NAME = "MainMenu";

//-- public methods -----
AppStage_MainMenu::AppStage_MainMenu(App* app)
	: AppStage(app, AppStage_MainMenu::APP_STAGE_NAME)
{ 
}

void AppStage_MainMenu::renderUI()
{
	const ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse;
	const ImVec2 displayCenter= ImGui::GetMainViewport()->GetCenter();
	const ImVec2 panelSize = ImVec2(300, 210);

	ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowPos(displayCenter, 0, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(panelSize);
	ImGui::Begin(locTextUTF8("", "main_menu"), nullptr, window_flags);
	{
		const ImVec2 buttonSize = ImVec2(220, 25);
		const float buttonOffsetX = (panelSize.x / 2) - (buttonSize.x / 2);

		ImGui::Dummy(ImVec2(220, 10));
		ImGui::NewLine();

		ImGui::SameLine(buttonOffsetX);
		if (ImGui::Button(locTextUTF8("", "video_compositor"), buttonSize))
		{
			m_app->pushAppStage<AppStage_Compositor>();
		}
		ImGui::NewLine();

		ImGui::SameLine(buttonOffsetX);
		if (ImGui::Button(locTextUTF8("", "calibration_pattern_settings"), buttonSize))
		{
			m_app->pushAppStage<AppStage_CalibrationPatternSettings>();
		}
		ImGui::NewLine();

		ImGui::SameLine(buttonOffsetX);
		if (ImGui::Button(locTextUTF8("", "camera_settings"), buttonSize))
		{
			m_app->pushAppStage<AppStage_CameraSettings>();
		}
		ImGui::NewLine();

		ImGui::SameLine(buttonOffsetX);
		if (ImGui::Button(locTextUTF8("", "vr_device_settings"), buttonSize))
		{
			m_app->pushAppStage<AppStage_VRDeviceSettings>();
		}
		ImGui::NewLine();
		
		ImGui::SameLine(buttonOffsetX);
		if (ImGui::Button(locTextUTF8("", "exit"), buttonSize))
		{
			m_app->requestShutdown();
		}
	}
	ImGui::End();
	ImGui::PopStyleVar(1);
}