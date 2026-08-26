#include "LightFixtureCalibration/GuiPanel_LightFixtureCalibration.h"
#include "LocText.h"

#include "imgui.h"

void GuiPanel_LightFixtureCalibration::onGui()
{
	switch (m_menuState)
	{
	case eLightFixtureCalibrationMenuState::verifyInitialCameraSetup:
	{
		ImGui::TextWrapped(locText("lightFixtureCalibration.verifyInitialCameraSetupFmt"), m_fixtureName.c_str());
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.ok")))
		{
			if (OnOkEvent)
				OnOkEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eLightFixtureCalibrationMenuState::capturePosition1:
	{
		ImGui::TextWrapped(locText("lightFixtureCalibration.capturePosition1Fmt"), m_fixtureName.c_str());
		ImGui::Spacing();
		if (ImGui::Button(locLabel("lightFixtureCalibration.redo")))
		{
			if (OnRedoEvent)
				OnRedoEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eLightFixtureCalibrationMenuState::moveCamera:
	{
		ImGui::TextWrapped("%s", locText("lightFixtureCalibration.moveCamera"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.ok")))
		{
			if (OnOkEvent)
				OnOkEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button(locLabel("lightFixtureCalibration.redo")))
		{
			if (OnRedoEvent)
				OnRedoEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eLightFixtureCalibrationMenuState::capturePosition2:
	{
		ImGui::TextWrapped(locText("lightFixtureCalibration.capturePosition2Fmt"), m_fixtureName.c_str());
		ImGui::Spacing();
		if (ImGui::Button(locLabel("lightFixtureCalibration.redo")))
		{
			if (OnRedoEvent)
				OnRedoEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eLightFixtureCalibrationMenuState::verifyTriangulatedPosition:
	{
		ImGui::TextWrapped("%s", locText("lightFixtureCalibration.verifyTriangulatedPosition"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.ok")))
		{
			if (OnOkEvent)
				OnOkEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button(locLabel("lightFixtureCalibration.redo")))
		{
			if (OnRedoEvent)
				OnRedoEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eLightFixtureCalibrationMenuState::calibrationComplete:
	{
		ImGui::TextWrapped(locText("lightFixtureCalibration.calibrationCompleteFmt"), m_fixtureName.c_str());
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.ok")))
		{
			if (OnOkEvent)
				OnOkEvent();
		}
	}
	break;

	case eLightFixtureCalibrationMenuState::pendingVideoStartStreamRequest:
	{
		ImGui::TextWrapped("%s", locText("lightFixtureCalibration.pendingVideoStart"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eLightFixtureCalibrationMenuState::failedVideoStartStreamRequest:
	{
		ImGui::TextWrapped("%s", locText("lightFixtureCalibration.failedVideoStart"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	default:
		break;
	}
}
