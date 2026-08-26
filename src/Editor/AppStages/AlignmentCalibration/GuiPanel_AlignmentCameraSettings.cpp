#include "AlignmentCalibration/GuiPanel_AlignmentCameraSettings.h"
#include "CameraComponent.h"

#include "imgui.h"
#include "LocText.h"

void GuiPanel_AlignmentCameraSettings::setCameraDefinition(CameraDefinitionConstPtr cameraDefinition)
{
	if (cameraDefinition)
	{
		m_vrFrameDelay= cameraDefinition->getTrackingFrameDelay();
	}
}

void GuiPanel_AlignmentCameraSettings::onGui()
{
	// Only show camera settings during capture and test states
	if (m_menuState != eAlignmentCalibrationMenuState::verifySetup
		&& m_menuState != eAlignmentCalibrationMenuState::testCalibration)
	{
		return;
	}

	ImGui::Separator();
	ImGui::TextUnformatted(locText("alignmentCalibration.cameraSettings"));
	ImGui::Spacing();

	// Viewpoint mode radio buttons
	const eAlignmentCalibrationViewpointMode prevMode= m_viewpointMode;

	int modeInt= (int)m_viewpointMode;
	if (m_menuState == eAlignmentCalibrationMenuState::verifySetup)
	{
		ImGui::RadioButton(locLabel("alignmentCalibration.viewpointCalibration"), &modeInt,
						   (int)eAlignmentCalibrationViewpointMode::calibration);
	}
	ImGui::RadioButton(locLabel("alignmentCalibration.viewpointStageView"), &modeInt,
					   (int)eAlignmentCalibrationViewpointMode::stageView);
	ImGui::RadioButton(locLabel("alignmentCalibration.viewpointXrView"), &modeInt,
					   (int)eAlignmentCalibrationViewpointMode::xrView);

	if ((eAlignmentCalibrationViewpointMode)modeInt != prevMode)
	{
		m_viewpointMode= (eAlignmentCalibrationViewpointMode)modeInt;
		if (OnViewpointModeChanged)
			OnViewpointModeChanged(m_viewpointMode);
	}

	ImGui::Spacing();

	// VR Frame Delay slider
	const int prevDelay= m_vrFrameDelay;
	ImGui::SliderInt(locLabel("alignmentCalibration.vrFrameDelay"), &m_vrFrameDelay, 0, 100);
	if (m_vrFrameDelay != prevDelay)
	{
		if (OnVRFrameDelayChanged)
			OnVRFrameDelayChanged(m_vrFrameDelay);
	}
}
