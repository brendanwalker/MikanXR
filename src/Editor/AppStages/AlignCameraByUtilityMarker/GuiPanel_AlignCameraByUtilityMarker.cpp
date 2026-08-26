#include "AlignCameraByUtilityMarker/GuiPanel_AlignCameraByUtilityMarker.h"
#include "LocText.h"

#include "imgui.h"

void GuiPanel_AlignCameraByUtilityMarker::onGui()
{
	switch (m_menuState)
	{
	case eAlignCameraByUtilityMarkerMenuState::selectSourceCamera:
	{
		// ModalDialog_SelectCamera is open; nothing additional to show here
	}
	break;

	case eAlignCameraByUtilityMarkerMenuState::pendingVideoStart:
	{
		ImGui::TextWrapped("%s", locText("alignCameraByUtilityMarker.startingVideoStreams"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eAlignCameraByUtilityMarkerMenuState::verifySetup:
	{
		ImGui::TextWrapped("%s", locText("alignCameraByUtilityMarker.verifySetup"));
		ImGui::Spacing();
		ImGui::Text(locText("alignCameraByUtilityMarker.sourceMarkerDetectedFmt"),
					m_isSourceMarkerVisible ? locText("alignCameraByUtilityMarker.yes")
											: locText("alignCameraByUtilityMarker.no"));
		ImGui::Text(locText("alignCameraByUtilityMarker.targetMarkerDetectedFmt"),
					m_isTargetMarkerVisible ? locText("alignCameraByUtilityMarker.yes")
											: locText("alignCameraByUtilityMarker.no"));
		ImGui::Spacing();
		const bool canBegin= m_isSourceMarkerVisible && m_isTargetMarkerVisible;
		if (!canBegin)
			ImGui::BeginDisabled();
		if (ImGui::Button(locLabel("alignCameraByUtilityMarker.begin")))
		{
			if (OnBeginEvent)
				OnBeginEvent();
		}
		if (!canBegin)
			ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eAlignCameraByUtilityMarkerMenuState::capturing:
	{
		ImGui::TextUnformatted(locText("alignCameraByUtilityMarker.samplingTransforms"));
		ImGui::Spacing();
		ImGui::TextUnformatted(locText("alignCameraByUtilityMarker.sourceCamera"));
		ImGui::ProgressBar(m_sourcePercent / 100.f);
		ImGui::TextUnformatted(locText("alignCameraByUtilityMarker.targetCamera"));
		ImGui::ProgressBar(m_targetPercent / 100.f);
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eAlignCameraByUtilityMarkerMenuState::testCalibration:
	{
		ImGui::TextUnformatted(locText("alignCameraByUtilityMarker.alignmentComplete"));
		ImGui::TextWrapped("%s", locText("alignCameraByUtilityMarker.verifyAxesOverlay"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("alignCameraByUtilityMarker.restart")))
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

	case eAlignCameraByUtilityMarkerMenuState::failedVideoStart:
	{
		ImGui::TextWrapped("%s", locText("alignCameraByUtilityMarker.failedVideoStart"));
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
