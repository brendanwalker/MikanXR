#include "AlignCameraByOriginMarker/GuiPanel_AlignCameraByOriginMarker.h"

#include "imgui.h"

void GuiPanel_AlignCameraByOriginMarker::onGui()
{
	switch (m_menuState)
	{
		case eAlignCameraByOriginMarkerMenuState::pendingVideoStart:
		{
			ImGui::TextWrapped("Starting video stream...");
			ImGui::Spacing();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eAlignCameraByOriginMarkerMenuState::verifySetup:
		{
			ImGui::TextWrapped("Point the camera at the Origin Marker until it is detected, then click Begin.");
			ImGui::Spacing();
			ImGui::Text("Origin Marker Detected: %s", m_isMarkerVisible ? "Yes" : "No");
			ImGui::Spacing();
			if (!m_isMarkerVisible) ImGui::BeginDisabled();
			if (ImGui::Button("Begin")) { if (OnBeginEvent) OnBeginEvent(); }
			if (!m_isMarkerVisible) ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eAlignCameraByOriginMarkerMenuState::capturing:
		{
			ImGui::Text("Sampling Origin Marker transforms...");
			ImGui::Spacing();
			ImGui::ProgressBar(m_capturePercent / 100.f);
			ImGui::Spacing();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eAlignCameraByOriginMarkerMenuState::testCalibration:
		{
			ImGui::Text("Camera aligned successfully!");
			ImGui::TextWrapped("Verify the axes overlay appears correctly over the Origin Marker.");
			ImGui::Spacing();
			if (ImGui::Button("Restart")) { if (OnRestartEvent) OnRestartEvent(); }
			ImGui::SameLine();
			if (ImGui::Button("Ok")) { if (OnReturnEvent) OnReturnEvent(); }
		} break;

		case eAlignCameraByOriginMarkerMenuState::failedVideoStart:
		{
			ImGui::TextWrapped("Error: Failed to start video stream.");
			ImGui::Spacing();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		default:
			break;
	}
}
