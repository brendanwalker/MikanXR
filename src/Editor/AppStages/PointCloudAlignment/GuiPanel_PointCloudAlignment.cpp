#include "PointCloudAlignment/GuiPanel_PointCloudAlignment.h"

#include "imgui.h"

void GuiPanel_PointCloudAlignment::onGui()
{
	switch (m_menuState)
	{
	case ePointCloudAlignmentMenuState::pendingVideoStart:
	{
		ImGui::TextWrapped("Starting video stream...");
		ImGui::Spacing();
		if (ImGui::Button("Cancel") && OnCancelEvent)
			OnCancelEvent();
	}
	break;

	case ePointCloudAlignmentMenuState::verifyInitialCameraSetup:
	{
		ImGui::TextWrapped("Automatic stencil alignment.");
		ImGui::Spacing();
		ImGui::TextWrapped("Position the tracked camera so the physical object is clearly in view, then choose how to "
						   "restrict feature capture.");
		ImGui::Spacing();
		if (ImGui::Button("Draw Region") && OnBeginRoiEvent)
			OnBeginRoiEvent();
		ImGui::SameLine();
		if (ImGui::Button("Capture Whole View") && OnSkipRoiEvent)
			OnSkipRoiEvent();
		ImGui::SameLine();
		if (ImGui::Button("Cancel") && OnCancelEvent)
			OnCancelEvent();
	}
	break;

	case ePointCloudAlignmentMenuState::paintRegionOfInterest:
	{
		ImGui::TextWrapped("Click two opposite corners in the camera image to box the object, then capture.");
		ImGui::Spacing();
		if (ImGui::Button("Skip") && OnSkipRoiEvent)
			OnSkipRoiEvent();
		ImGui::SameLine();
		if (ImGui::Button("Cancel") && OnCancelEvent)
			OnCancelEvent();
	}
	break;

	case ePointCloudAlignmentMenuState::captureFeatureCloud:
	{
		ImGui::TextWrapped("Slowly move the camera around the object to build a 3D feature cloud.");
		ImGui::Spacing();
		ImGui::Text("Tracked features: %d", m_captureStats.trackedFeatureCount);
		ImGui::Text("Cloud points: %d", m_captureStats.cloudPointCount);
		ImGui::Text("Keyframes: %d", m_captureStats.keyframeCount);
		ImGui::Text("Coverage: %.2f m", m_captureStats.coverageMeters);
		ImGui::Text("Mean reproj error: %.2f px", m_captureStats.meanReprojErrorPx);
		ImGui::Spacing();
		if (ImGui::Button("Done Capturing") && OnStopCaptureEvent)
			OnStopCaptureEvent();
		ImGui::SameLine();
		if (ImGui::Button("Cancel") && OnCancelEvent)
			OnCancelEvent();
	}
	break;

	case ePointCloudAlignmentMenuState::reviewCloud:
	{
		ImGui::TextWrapped("Review the captured point cloud.");
		ImGui::Spacing();
		ImGui::Text("Cloud points: %d", m_captureStats.cloudPointCount);
		ImGui::Text("Coverage: %.2f m", m_captureStats.coverageMeters);
		ImGui::Spacing();
		if (ImGui::Button("Auto-Align") && OnRunAlignmentEvent)
			OnRunAlignmentEvent();
		ImGui::SameLine();
		if (ImGui::Button("Recapture") && OnRedoEvent)
			OnRedoEvent();
		ImGui::SameLine();
		if (ImGui::Button("Cancel") && OnCancelEvent)
			OnCancelEvent();
	}
	break;

	case ePointCloudAlignmentMenuState::runAutoAlignment:
	{
		ImGui::TextWrapped("Aligning model to the feature cloud...");
	}
	break;

	case ePointCloudAlignmentMenuState::verifyAlignment:
	{
		if (m_alignmentResult.converged)
			ImGui::Text("Alignment complete.");
		else
			ImGui::TextWrapped("Alignment finished (did not fully converge). Verify the overlay.");
		ImGui::Spacing();
		ImGui::Text("RMS residual: %.1f mm", m_alignmentResult.rmsResidualMeters * 1000.f);
		ImGui::Text("Inliers: %d", m_alignmentResult.inlierCount);
		ImGui::Text("Scale: %.3f", m_alignmentResult.scale);
		ImGui::Spacing();
		ImGui::TextWrapped(
			"Check the wireframe overlay from multiple angles. Nudge with the gizmo and re-align if needed.");
		ImGui::Spacing();
		if (ImGui::Button("Accept") && OnOkEvent)
			OnOkEvent();
		ImGui::SameLine();
		if (ImGui::Button("Re-Align") && OnRunAlignmentEvent)
			OnRunAlignmentEvent();
		ImGui::SameLine();
		if (ImGui::Button("Recapture") && OnRedoEvent)
			OnRedoEvent();
		ImGui::SameLine();
		if (ImGui::Button("Cancel") && OnCancelEvent)
			OnCancelEvent();
	}
	break;

	case ePointCloudAlignmentMenuState::failedVideoStartStreamRequest:
	{
		ImGui::TextWrapped("Error: Failed to start video stream.");
		ImGui::Spacing();
		if (ImGui::Button("Cancel") && OnCancelEvent)
			OnCancelEvent();
	}
	break;

	default:
		break;
	}
}
