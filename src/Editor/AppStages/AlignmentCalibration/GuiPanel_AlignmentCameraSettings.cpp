#include "AlignmentCalibration/GuiPanel_AlignmentCameraSettings.h"
#include "CameraComponent.h"

#include "imgui.h"

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
	ImGui::Text("Camera Settings");
	ImGui::Spacing();

	// Viewpoint mode radio buttons
	const eAlignmentCalibrationViewpointMode prevMode= m_viewpointMode;

	int modeInt= (int)m_viewpointMode;
	if (m_menuState == eAlignmentCalibrationMenuState::verifySetup)
	{
		ImGui::RadioButton(
			k_alignmentCalibrationViewpointModeStrings[(int)eAlignmentCalibrationViewpointMode::calibration].c_str(),
			&modeInt, (int)eAlignmentCalibrationViewpointMode::calibration);
	}
	ImGui::RadioButton(
		k_alignmentCalibrationViewpointModeStrings[(int)eAlignmentCalibrationViewpointMode::stageView].c_str(),
		&modeInt, (int)eAlignmentCalibrationViewpointMode::stageView);
	ImGui::RadioButton(
		k_alignmentCalibrationViewpointModeStrings[(int)eAlignmentCalibrationViewpointMode::xrView].c_str(), &modeInt,
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
	ImGui::SliderInt("VR Frame Delay", &m_vrFrameDelay, 0, 100);
	if (m_vrFrameDelay != prevDelay)
	{
		if (OnVRFrameDelayChanged)
			OnVRFrameDelayChanged(m_vrFrameDelay);
	}
}
