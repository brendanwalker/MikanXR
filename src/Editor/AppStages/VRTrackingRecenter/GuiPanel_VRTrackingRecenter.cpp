#include "VRTrackingRecenter/GuiPanel_VRTrackingRecenter.h"

#include "imgui.h"

void GuiPanel_VRTrackingRecenter::setCurrentMarkerValid(bool valid)
{
	if (m_isCurrentMarkerStable && !valid)
	{
		setCurrentMarkerStable(false);
	}
	m_isCurrentMarkerValid = valid;
}

void GuiPanel_VRTrackingRecenter::setCurrentMarkerStable(bool stable)
{
	if (!stable)
		m_markerStabilityTimer = 0.f;

	if (m_isCurrentMarkerStable != stable)
	{
		m_isCurrentMarkerStable = stable;
		if (OnMarkerStabilityChangedEvent)
			OnMarkerStabilityChangedEvent(m_isCurrentMarkerStable);
	}
}

void GuiPanel_VRTrackingRecenter::updateMarkerStabilityTimer(float deltaTime)
{
	if (m_isCurrentMarkerValid && !m_isCurrentMarkerStable)
	{
		m_markerStabilityTimer += deltaTime;
		if (m_markerStabilityTimer >= k_markerStabilityDuration)
		{
			setCurrentMarkerStable(true);
		}
	}
}

void GuiPanel_VRTrackingRecenter::onGui()
{
	switch (m_menuState)
	{
		case eVRTrackingRecenterMenuState::pendingVideoStart:
		{
			ImGui::TextWrapped("Starting video stream...");
			ImGui::Spacing();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eVRTrackingRecenterMenuState::verifySetup:
		{
			ImGui::TextWrapped("Place the Aruco marker on the floor at the desired tracking origin.");
			ImGui::Spacing();
			ImGui::Text("Marker Detected: %s", m_isCurrentMarkerValid ? "Yes" : "No");
			ImGui::Text("Marker Stable: %s", m_isCurrentMarkerStable ? "Yes" : "No");
			ImGui::Spacing();
			if (!m_isCurrentMarkerStable) ImGui::BeginDisabled();
			if (ImGui::Button("Begin")) { if (OnBeginEvent) OnBeginEvent(); }
			if (!m_isCurrentMarkerStable) ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eVRTrackingRecenterMenuState::capture:
		{
			ImGui::Text("Capturing marker pose samples...");
			ImGui::Spacing();
			ImGui::ProgressBar(m_calibrationPercent / 100.f);
			ImGui::Spacing();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eVRTrackingRecenterMenuState::testCalibration:
		{
			ImGui::Text("VR tracking recentered successfully!");
			ImGui::Spacing();
			if (ImGui::Button("Restart")) { if (OnRestartEvent) OnRestartEvent(); }
			ImGui::SameLine();
			if (ImGui::Button("Ok")) { if (OnReturnEvent) OnReturnEvent(); }
		} break;

		case eVRTrackingRecenterMenuState::failedVideoStartStreamRequest:
		{
			ImGui::TextWrapped("Error: Failed to start video stream.");
			ImGui::Spacing();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		default:
			break;
	}
}
