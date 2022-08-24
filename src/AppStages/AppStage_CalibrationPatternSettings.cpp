//-- inludes -----
#include "AppStage_MainMenu.h"
#include "AppStage_CalibrationPatternSettings.h"
#include "App.h"
#include "ProfileConfig.h"
#include "Renderer.h"
#include "Logger.h"
#include "MathUtility.h"

#include "imgui.h"

//-- statics ----
const char* AppStage_CalibrationPatternSettings::APP_STAGE_NAME = "CalibrationPatternSettings";

//-- public methods -----
AppStage_CalibrationPatternSettings::AppStage_CalibrationPatternSettings(App* app)
	: AppStage(app, AppStage_CalibrationPatternSettings::APP_STAGE_NAME)
{
}

void AppStage_CalibrationPatternSettings::renderUI()
{
	ProfileConfig* profileConfig= App::getInstance()->getProfileConfig();

	ImGuiWindowFlags window_flags =
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoScrollbar |
		ImGuiWindowFlags_NoCollapse;
	const ImVec2 displayCenter = ImGui::GetMainViewport()->GetCenter();
	const ImVec2 panelSize = ImVec2(300, 350);

	bool bDirty= false;

	ImGui::PushStyleVar(ImGuiStyleVar_WindowTitleAlign, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowPos(displayCenter, 0, ImVec2(0.5f, 0.5f));
	ImGui::SetNextWindowSize(panelSize);
	ImGui::Begin("Calibration Pattern Settings", nullptr, window_flags);
	{
		ImGui::SetNextItemWidth(150);
		const char* patterns[] = { "Chessboard", "Circle Grid" };
		bDirty |= ImGui::Combo("Pattern Type", (int*)&profileConfig->calibrationPatternType, patterns, IM_ARRAYSIZE(patterns));

		ImGui::PushItemWidth(100);
		ImGui::Separator();

		int* chessbordRows = &profileConfig->chessbordRows;
		if (ImGui::InputInt("Chessboard Rows", chessbordRows))
		{
			*chessbordRows = int_clamp(*chessbordRows, 5, 15);
			bDirty = true;
		}

		int* chessbordCols = &profileConfig->chessbordCols;
		if (ImGui::InputInt("Chessboard Columns", chessbordCols))
		{
			*chessbordCols = int_clamp(*chessbordCols, 5, 15);
			bDirty = true;
		}

		float* squareLength = &profileConfig->squareLengthMM;
		if (ImGui::InputFloat("Square Length (mm)", squareLength, 0.5f, 1.f))
		{
			*squareLength = clampf(*squareLength, 1.f, 100.f);
			bDirty = true;
		}

		ImGui::Separator();

		int* circleGridRows = &profileConfig->circleGridRows;
		if (ImGui::InputInt("Circle Grid Rows", circleGridRows))
		{
			*circleGridRows = int_clamp(*circleGridRows, 5, 15);
			bDirty = true;
		}

		int* circleGridCols = &profileConfig->circleGridCols;
		if (ImGui::InputInt("Circle Grid Columns", circleGridCols))
		{
			*circleGridCols = int_clamp(*circleGridCols, 5, 15);
			bDirty = true;
		}

		float* circleSpacingMM = &profileConfig->circleSpacingMM;
		if (ImGui::InputFloat("Circle Spacing (mm)", circleSpacingMM, 0.5f, 1.f))
		{
			*circleSpacingMM = clampf(*circleSpacingMM, 1.f, 100.f);
			bDirty = true;
		}

		float* circleDiameterMM = &profileConfig->circleDiameterMM;
		if (ImGui::InputFloat("Circle Diameter (mm)", circleDiameterMM, 0.5f, 1.f))
		{
			*circleDiameterMM = clampf(*circleDiameterMM, 1.f, 100.f);
			bDirty = true;
		}

		ImGui::Separator();

		float* puckHorizontalOffset = &profileConfig->puckHorizontalOffsetMM;
		if (ImGui::InputFloat("Puck Horizontal Offset (mm)", puckHorizontalOffset, 0.5f, 1.f))
		{
			*puckHorizontalOffset = clampf(*puckHorizontalOffset, 1.f, 200.f);
			bDirty = true;
		}

		float* puckVerticalOffset = &profileConfig->puckVerticalOffsetMM;
		if (ImGui::InputFloat("Puck Vertical Offset (mm)", puckVerticalOffset, 0.5f, 1.f))
		{
			*puckVerticalOffset = clampf(*puckVerticalOffset, 1.f, 200.f);
			bDirty = true;
		}

		float* puckDepthOffset = &profileConfig->puckDepthOffsetMM;
		if (ImGui::InputFloat("Puck Depth Offset (mm)", puckDepthOffset, 0.5f, 1.f))
		{
			*puckDepthOffset = clampf(*puckDepthOffset, 1.f, 10.f);
			bDirty = true;
		}

		ImGui::PopItemWidth();

		const ImVec2 buttonSize = ImVec2(150, 25);
		ImGui::NewLine();
		ImGui::SameLine((panelSize.x / 2) - (buttonSize.x / 2));
		if (ImGui::Button(locTextUTF8("", "return_to_main_menu"), buttonSize))
		{
			m_app->popAppState();
		}

		if (bDirty)
		{
			profileConfig->save();
		}
	}
	ImGui::End();
	ImGui::PopStyleVar(1);
}

void AppStage_CalibrationPatternSettings::enter()
{

}

void AppStage_CalibrationPatternSettings::exit()
{

}
