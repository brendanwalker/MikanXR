#include "AlignCameraByOriginMarker/GuiPanel_AlignCameraByOriginMarker.h"
#include "LocText.h"

#include "imgui.h"

void GuiPanel_AlignCameraByOriginMarker::onGui()
{
	switch (m_menuState)
	{
	case eAlignCameraByOriginMarkerMenuState::pendingVideoStart:
	{
		ImGui::TextWrapped("%s", locText("alignCameraByOriginMarker.startingVideoStream"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eAlignCameraByOriginMarkerMenuState::verifySetup:
	{
		ImGui::TextWrapped("%s", locText("alignCameraByOriginMarker.verifySetup"));
		ImGui::Spacing();
		ImGui::Text(locText("alignCameraByOriginMarker.markerDetectedFmt"),
					m_isMarkerVisible ? locText("alignCameraByOriginMarker.yes")
									  : locText("alignCameraByOriginMarker.no"));
		ImGui::Spacing();
		if (!m_isMarkerVisible)
			ImGui::BeginDisabled();
		if (ImGui::Button(locLabel("alignCameraByOriginMarker.begin")))
		{
			if (OnBeginEvent)
				OnBeginEvent();
		}
		if (!m_isMarkerVisible)
			ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eAlignCameraByOriginMarkerMenuState::capturing:
	{
		ImGui::TextUnformatted(locText("alignCameraByOriginMarker.samplingTransforms"));
		ImGui::Spacing();
		ImGui::ProgressBar(m_capturePercent / 100.f);
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eAlignCameraByOriginMarkerMenuState::testCalibration:
	{
		ImGui::TextUnformatted(locText("alignCameraByOriginMarker.alignmentComplete"));
		ImGui::TextWrapped("%s", locText("alignCameraByOriginMarker.verifyAxesOverlay"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("alignCameraByOriginMarker.restart")))
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

	case eAlignCameraByOriginMarkerMenuState::failedVideoStart:
	{
		ImGui::TextWrapped("%s", locText("alignCameraByOriginMarker.failedVideoStart"));
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
