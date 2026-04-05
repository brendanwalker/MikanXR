#include "MonoLensCalibration/GuiPanel_MonoLensCalibration.h"

#include "imgui.h"

bool GuiPanel_MonoLensCalibration::init(AppStage* ownerAppStage)
{
	return true;
}

void GuiPanel_MonoLensCalibration::setCurrentImagePointsValid(bool valid)
{
	if (m_areCurrentImagePointsStable && !valid)
	{
		setCurrentImagePointsStable(false);
	}
	m_areCurrentImagePointsValid = valid;
}

void GuiPanel_MonoLensCalibration::setCurrentImagePointsStable(bool stable)
{
	if (!stable)
		m_imagePointsStabilityTimer = 0.f;

	if (m_areCurrentImagePointsStable != stable)
	{
		m_areCurrentImagePointsStable = stable;
		if (OnImagePointStabilityChangedEvent)
			OnImagePointStabilityChangedEvent(m_areCurrentImagePointsStable);
	}
}

void GuiPanel_MonoLensCalibration::updateImagePointStabilityTimer(float deltaTime)
{
	if (m_areCurrentImagePointsValid && !m_areCurrentImagePointsStable)
	{
		m_imagePointsStabilityTimer += deltaTime;
		if (m_imagePointsStabilityTimer >= k_imagePointStabilityDuration)
		{
			setCurrentImagePointsStable(true);
		}
	}
}

void GuiPanel_MonoLensCalibration::resetCalibrationState()
{
	m_calibrationPercent = 0.f;
	m_reprojectionError = 0.f;
	setCurrentImagePointsValid(false);
}

void GuiPanel_MonoLensCalibration::onGui()
{
	switch (m_menuState)
	{
		case eMonoLensCalibrationMenuState::capture:
		{
			ImGui::Text("Pattern Detected: %s", m_areCurrentImagePointsValid ? "Yes" : "No");
			ImGui::Text("Pattern Stable: %s", m_areCurrentImagePointsStable ? "Yes" : "No");
			ImGui::Spacing();
			ImGui::Text("Hold the calibration board steady in different positions.");
			ImGui::Spacing();
			ImGui::ProgressBar(m_calibrationPercent / 100.f);
			ImGui::Spacing();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		case eMonoLensCalibrationMenuState::processingCalibration:
		{
			ImGui::Text("Processing calibration...");
		} break;

		case eMonoLensCalibrationMenuState::testCalibration:
		{
			ImGui::Text("Calibration complete!");
			ImGui::Text("Reprojection Error: %.4f", m_reprojectionError);
			ImGui::Spacing();
			if (ImGui::Button("Restart")) { if (OnRestartEvent) OnRestartEvent(); }
			ImGui::SameLine();
			if (ImGui::Button("Ok")) { if (OnReturnEvent) OnReturnEvent(); }
		} break;

		case eMonoLensCalibrationMenuState::failedCalibration:
		{
			ImGui::TextWrapped("Calibration failed. Please try again with more samples.");
			ImGui::Spacing();
			if (ImGui::Button("Restart")) { if (OnRestartEvent) OnRestartEvent(); }
			ImGui::SameLine();
			if (ImGui::Button("Ok")) { if (OnReturnEvent) OnReturnEvent(); }
		} break;

		case eMonoLensCalibrationMenuState::failedVideoStartStreamRequest:
		{
			ImGui::TextWrapped("Error: Failed to start video stream.");
			ImGui::Spacing();
			if (ImGui::Button("Cancel")) { if (OnCancelEvent) OnCancelEvent(); }
		} break;

		default:
			break;
	}
}
