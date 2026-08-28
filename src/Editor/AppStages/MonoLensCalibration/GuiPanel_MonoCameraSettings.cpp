#include "MonoLensCalibration/GuiPanel_MonoCameraSettings.h"

#include "imgui.h"
#include "LocText.h"

void GuiPanel_MonoCameraSettings::onGui()
{
	ImGui::Separator();
	ImGui::TextUnformatted(locText("monoLensCalibration.displayMode"));

	const int prevMode= m_videoDisplayMode;

	ImGui::RadioButton(locLabel("monoLensCalibration.normalBgr"), &m_videoDisplayMode,
					   (int)eVideoDisplayMode::mode_bgr);
	ImGui::RadioButton(locLabel("monoLensCalibration.undistorted"), &m_videoDisplayMode,
					   (int)eVideoDisplayMode::mode_undistored);
	ImGui::RadioButton(locLabel("monoLensCalibration.grayscale"), &m_videoDisplayMode,
					   (int)eVideoDisplayMode::mode_grayscale);

	if (m_videoDisplayMode != prevMode)
	{
		if (OnVideoDisplayModeChanged)
			OnVideoDisplayModeChanged((eVideoDisplayMode)m_videoDisplayMode);
	}
}
