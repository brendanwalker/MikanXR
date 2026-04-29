#pragma once

#include "ComponentFwd.h"
#include "Constants_AlignmentCalibration.h"
#include "Shared/GuiPanel.h"

#include <functional>

class AppStage;

class GuiPanel_AlignmentCameraSettings : public GuiPanel
{
public:
	GuiPanel_AlignmentCameraSettings(AppStage* ownerAppStage) : GuiPanel(ownerAppStage) {}
	void setCameraDefinition(CameraDefinitionConstPtr cameraDefinition);
	virtual void onGui() override;

	eAlignmentCalibrationMenuState getMenuState() const { return m_menuState; }
	void setMenuState(eAlignmentCalibrationMenuState newState) { m_menuState = newState; }

	eAlignmentCalibrationViewpointMode getViewpointMode() const { return m_viewpointMode; }
	void setViewpointMode(eAlignmentCalibrationViewpointMode mode) { m_viewpointMode = mode; }

	int getVRFrameDelay() const { return m_vrFrameDelay; }
	void setVRFrameDelay(int delay) { m_vrFrameDelay = delay; }

	std::function<void(eAlignmentCalibrationViewpointMode)> OnViewpointModeChanged;
	std::function<void(int)> OnVRFrameDelayChanged;

private:
	eAlignmentCalibrationMenuState m_menuState = eAlignmentCalibrationMenuState::inactive;
	eAlignmentCalibrationViewpointMode m_viewpointMode = eAlignmentCalibrationViewpointMode::calibration;
	int m_vrFrameDelay = 0;
};
