#include "AlignCameraByUtilityMarker/GuiPanel_AlignCameraByUtilityMarker.h"

#include "imgui.h"

void GuiPanel_AlignCameraByUtilityMarker::onGui()
{
	switch (m_menuState)
	{
		case eAlignCameraByUtilityMarkerMenuState::selectSourceCamera:
		{
			// ModalDialog_SelectCamera is open; nothing additional to show here
		} break;

		case eAlignCameraByUtilityMarkerMenuState::pendingVideoStart:
		{
			ImGui::TextWrapped("Starting video streams...");
			ImGui::Spacing();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eAlignCameraByUtilityMarkerMenuState::verifySetup:
		{
			ImGui::TextWrapped("Point both cameras at the Utility Marker until it is detected, then click Begin.");
			ImGui::Spacing();
			ImGui::Text("Source Camera - Marker Detected: %s", m_isSourceMarkerVisible ? "Yes" : "No");
			ImGui::Text("Target Camera - Marker Detected: %s", m_isTargetMarkerVisible ? "Yes" : "No");
			ImGui::Spacing();
			const bool canBegin = m_isSourceMarkerVisible && m_isTargetMarkerVisible;
			if (!canBegin) ImGui::BeginDisabled();
			if (ImGui::Button("Begin")) { if (OnBeginEvent) OnBeginEvent(); }
			if (!canBegin) ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eAlignCameraByUtilityMarkerMenuState::capturing:
		{
			ImGui::Text("Sampling Utility Marker transforms...");
			ImGui::Spacing();
			ImGui::Text("Source Camera:");
			ImGui::ProgressBar(m_sourcePercent / 100.f);
			ImGui::Text("Target Camera:");
			ImGui::ProgressBar(m_targetPercent / 100.f);
			ImGui::Spacing();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eAlignCameraByUtilityMarkerMenuState::testCalibration:
		{
			ImGui::Text("Target camera aligned successfully!");
			ImGui::TextWrapped("Verify the axes overlay appears correctly over the Utility Marker.");
			ImGui::Spacing();
			if (ImGui::Button("Restart")) { if (OnRestartEvent) OnRestartEvent(); }
			ImGui::SameLine();
			if (ImGui::Button("Ok")) { if (OnReturnEvent) OnReturnEvent(); }
		} break;

		case eAlignCameraByUtilityMarkerMenuState::failedVideoStart:
		{
			ImGui::TextWrapped("Error: Failed to start video stream(s).");
			ImGui::Spacing();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		default:
			break;
	}
}
