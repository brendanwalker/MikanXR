#include "VRTrackingRecenter/GuiPanel_VRTrackingRecenter.h"

#include "imgui.h"
#include "LocText.h"

void GuiPanel_VRTrackingRecenter::setCurrentMarkerValid(bool valid)
{
	if (m_isCurrentMarkerStable && !valid)
	{
		setCurrentMarkerStable(false);
	}
	m_isCurrentMarkerValid= valid;
}

void GuiPanel_VRTrackingRecenter::setCurrentMarkerStable(bool stable)
{
	if (!stable)
		m_markerStabilityTimer= 0.f;

	if (m_isCurrentMarkerStable != stable)
	{
		m_isCurrentMarkerStable= stable;
		if (OnMarkerStabilityChangedEvent)
			OnMarkerStabilityChangedEvent(m_isCurrentMarkerStable);
	}
}

void GuiPanel_VRTrackingRecenter::updateMarkerStabilityTimer(float deltaTime)
{
	if (m_isCurrentMarkerValid && !m_isCurrentMarkerStable)
	{
		m_markerStabilityTimer+= deltaTime;
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
		ImGui::TextWrapped("%s", locText("vrTrackingRecenter.startingVideoStream"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eVRTrackingRecenterMenuState::verifySetup:
	{
		ImGui::TextWrapped("%s", locText("vrTrackingRecenter.placeMarker"));
		ImGui::Spacing();
		ImGui::Text(locText("vrTrackingRecenter.markerDetectedFmt"),
					m_isCurrentMarkerValid ? locText("vrTrackingRecenter.yes") : locText("vrTrackingRecenter.no"));
		ImGui::Text(locText("vrTrackingRecenter.markerStableFmt"),
					m_isCurrentMarkerStable ? locText("vrTrackingRecenter.yes") : locText("vrTrackingRecenter.no"));
		ImGui::Spacing();
		if (!m_isCurrentMarkerStable)
			ImGui::BeginDisabled();
		if (ImGui::Button(locLabel("vrTrackingRecenter.begin")))
		{
			if (OnBeginEvent)
				OnBeginEvent();
		}
		if (!m_isCurrentMarkerStable)
			ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eVRTrackingRecenterMenuState::capture:
	{
		ImGui::TextUnformatted(locText("vrTrackingRecenter.capturingSamples"));
		ImGui::Spacing();
		ImGui::ProgressBar(m_calibrationPercent / 100.f);
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eVRTrackingRecenterMenuState::testCalibration:
	{
		ImGui::TextUnformatted(locText("vrTrackingRecenter.recenteredSuccessfully"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("vrTrackingRecenter.restart")))
		{
			if (OnRestartEvent)
				OnRestartEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.ok")))
		{
			if (OnReturnEvent)
				OnReturnEvent();
		}
	}
	break;

	case eVRTrackingRecenterMenuState::failedVideoStartStreamRequest:
	{
		ImGui::TextWrapped("%s", locText("vrTrackingRecenter.failedVideoStream"));
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
