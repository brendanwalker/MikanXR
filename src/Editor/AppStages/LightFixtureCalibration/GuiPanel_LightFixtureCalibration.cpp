#include "LightFixtureCalibration/GuiPanel_LightFixtureCalibration.h"

#include "imgui.h"

void GuiPanel_LightFixtureCalibration::onGui()
{
	switch (m_menuState)
	{
	case eLightFixtureCalibrationMenuState::verifyInitialCameraSetup:
	{
		ImGui::TextWrapped(
			"Fixture '%s' has been set to white. "
			"Position the camera so the fixture is visible and verify the video stream is running.",
			m_fixtureName.c_str());
		ImGui::Spacing();
		if (ImGui::Button("Ok"))
		{
			if (OnOkEvent)
				OnOkEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eLightFixtureCalibrationMenuState::capturePosition1:
	{
		ImGui::TextWrapped(
			"Camera position 1: Click on the '%s' fixture in the image.",
			m_fixtureName.c_str());
		ImGui::Spacing();
		if (ImGui::Button("Redo"))
		{
			if (OnRedoEvent)
				OnRedoEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eLightFixtureCalibrationMenuState::moveCamera:
	{
		ImGui::TextWrapped(
			"Move the camera to a second position with a different angle to the fixture.");
		ImGui::Spacing();
		if (ImGui::Button("Ok"))
		{
			if (OnOkEvent)
				OnOkEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button("Redo"))
		{
			if (OnRedoEvent)
				OnRedoEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eLightFixtureCalibrationMenuState::capturePosition2:
	{
		ImGui::TextWrapped(
			"Camera position 2: Click on the '%s' fixture in the image.",
			m_fixtureName.c_str());
		ImGui::Spacing();
		if (ImGui::Button("Redo"))
		{
			if (OnRedoEvent)
				OnRedoEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eLightFixtureCalibrationMenuState::verifyTriangulatedPosition:
	{
		ImGui::TextWrapped(
			"Verify the triangulated fixture position looks correct in the viewport.");
		ImGui::Spacing();
		if (ImGui::Button("Ok"))
		{
			if (OnOkEvent)
				OnOkEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button("Redo"))
		{
			if (OnRedoEvent)
				OnRedoEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eLightFixtureCalibrationMenuState::calibrationComplete:
	{
		ImGui::TextWrapped(
			"Fixture '%s' position calibrated successfully.",
			m_fixtureName.c_str());
		ImGui::Spacing();
		if (ImGui::Button("Ok"))
		{
			if (OnOkEvent)
				OnOkEvent();
		}
	}
	break;

	case eLightFixtureCalibrationMenuState::pendingVideoStartStreamRequest:
	{
		ImGui::TextWrapped("Waiting for video stream to start...");
		ImGui::Spacing();
		if (ImGui::Button("Cancel"))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eLightFixtureCalibrationMenuState::failedVideoStartStreamRequest:
	{
		ImGui::TextWrapped("Error: Failed to start video stream.");
		ImGui::Spacing();
		if (ImGui::Button("Cancel"))
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
