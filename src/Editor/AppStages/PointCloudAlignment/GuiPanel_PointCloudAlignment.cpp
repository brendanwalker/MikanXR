#include "PointCloudAlignment/GuiPanel_PointCloudAlignment.h"
#include "LocText.h"

#include "imgui.h"

void GuiPanel_PointCloudAlignment::onGui()
{
	switch (m_menuState)
	{
	case ePointCloudAlignmentMenuState::pendingVideoStart:
	{
		ImGui::TextWrapped("%s", locText("pointCloudAlignment.startingVideoStream"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.cancel")) && OnCancelEvent)
			OnCancelEvent();
	}
	break;

	case ePointCloudAlignmentMenuState::verifyInitialCameraSetup:
	{
		ImGui::TextWrapped("%s", locText("pointCloudAlignment.automaticAlignment"));
		ImGui::Spacing();
		ImGui::TextWrapped("%s", locText("pointCloudAlignment.verifyInitialCameraSetup"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("pointCloudAlignment.drawRegion")) && OnBeginRoiEvent)
			OnBeginRoiEvent();
		ImGui::SameLine();
		if (ImGui::Button(locLabel("pointCloudAlignment.captureWholeView")) && OnSkipRoiEvent)
			OnSkipRoiEvent();
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.cancel")) && OnCancelEvent)
			OnCancelEvent();
	}
	break;

	case ePointCloudAlignmentMenuState::paintRegionOfInterest:
	{
		ImGui::TextWrapped("%s", locText("pointCloudAlignment.paintRegionOfInterest"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("pointCloudAlignment.skip")) && OnSkipRoiEvent)
			OnSkipRoiEvent();
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.cancel")) && OnCancelEvent)
			OnCancelEvent();
	}
	break;

	case ePointCloudAlignmentMenuState::captureFeatureCloud:
	{
		ImGui::TextWrapped("%s", locText("pointCloudAlignment.captureFeatureCloud"));
		ImGui::Spacing();
		ImGui::Text(locText("pointCloudAlignment.trackedFeaturesFmt"), m_captureStats.trackedFeatureCount);
		ImGui::Text(locText("pointCloudAlignment.cloudPointsFmt"), m_captureStats.cloudPointCount);
		ImGui::Text(locText("pointCloudAlignment.keyframesFmt"), m_captureStats.keyframeCount);
		ImGui::Text(locText("pointCloudAlignment.coverageFmt"), m_captureStats.coverageMeters);
		ImGui::Text(locText("pointCloudAlignment.meanReprojErrorFmt"), m_captureStats.meanReprojErrorPx);
		ImGui::Spacing();
		if (ImGui::Button(locLabel("pointCloudAlignment.doneCapturing")) && OnStopCaptureEvent)
			OnStopCaptureEvent();
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.cancel")) && OnCancelEvent)
			OnCancelEvent();
	}
	break;

	case ePointCloudAlignmentMenuState::reviewCloud:
	{
		ImGui::TextWrapped("%s", locText("pointCloudAlignment.reviewCloud"));
		ImGui::Spacing();
		ImGui::Text(locText("pointCloudAlignment.cloudPointsFmt"), m_captureStats.cloudPointCount);
		ImGui::Text(locText("pointCloudAlignment.coverageFmt"), m_captureStats.coverageMeters);
		ImGui::Spacing();
		if (ImGui::Button(locLabel("pointCloudAlignment.autoAlign")) && OnRunAlignmentEvent)
			OnRunAlignmentEvent();
		ImGui::SameLine();
		if (ImGui::Button(locLabel("pointCloudAlignment.recapture")) && OnRedoEvent)
			OnRedoEvent();
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.cancel")) && OnCancelEvent)
			OnCancelEvent();
	}
	break;

	case ePointCloudAlignmentMenuState::runAutoAlignment:
	{
		ImGui::TextWrapped("%s", locText("pointCloudAlignment.runningAutoAlignment"));
	}
	break;

	case ePointCloudAlignmentMenuState::verifyAlignment:
	{
		if (m_alignmentResult.converged)
			ImGui::TextUnformatted(locText("pointCloudAlignment.alignmentComplete"));
		else
			ImGui::TextWrapped("%s", locText("pointCloudAlignment.alignmentNotConverged"));
		ImGui::Spacing();
		ImGui::Text(locText("pointCloudAlignment.rmsResidualFmt"), m_alignmentResult.rmsResidualMeters * 1000.f);
		ImGui::Text(locText("pointCloudAlignment.inliersFmt"), m_alignmentResult.inlierCount);
		ImGui::Text(locText("pointCloudAlignment.scaleFmt"), m_alignmentResult.scale);
		ImGui::Spacing();
		ImGui::TextWrapped("%s", locText("pointCloudAlignment.verifyAlignmentInstructions"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("pointCloudAlignment.accept")) && OnOkEvent)
			OnOkEvent();
		ImGui::SameLine();
		if (ImGui::Button(locLabel("pointCloudAlignment.realign")) && OnRunAlignmentEvent)
			OnRunAlignmentEvent();
		ImGui::SameLine();
		if (ImGui::Button(locLabel("pointCloudAlignment.recapture")) && OnRedoEvent)
			OnRedoEvent();
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.cancel")) && OnCancelEvent)
			OnCancelEvent();
	}
	break;

	case ePointCloudAlignmentMenuState::failedVideoStartStreamRequest:
	{
		ImGui::TextWrapped("%s", locText("pointCloudAlignment.failedVideoStart"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.cancel")) && OnCancelEvent)
			OnCancelEvent();
	}
	break;

	default:
		break;
	}
}
