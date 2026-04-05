#include "MonoLensCalibration/GuiPanel_MonoCameraSettings.h"

#include "imgui.h"

void GuiPanel_MonoCameraSettings::onGui()
{
	ImGui::Separator();
	ImGui::Text("Display Mode");

	const int prevMode = m_videoDisplayMode;

	ImGui::RadioButton("Normal (BGR)", &m_videoDisplayMode, (int)eVideoDisplayMode::mode_bgr);
	ImGui::RadioButton("Undistorted", &m_videoDisplayMode, (int)eVideoDisplayMode::mode_undistored);
	ImGui::RadioButton("Grayscale", &m_videoDisplayMode, (int)eVideoDisplayMode::mode_grayscale);

	if (m_videoDisplayMode != prevMode)
	{
		if (OnVideoDisplayModeChanged)
			OnVideoDisplayModeChanged((eVideoDisplayMode)m_videoDisplayMode);
	}
}
