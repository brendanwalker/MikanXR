#include "AlignmentCalibration/GuiPanel_AlignmentCalibration.h"

#include "imgui.h"

void GuiPanel_AlignmentCalibration::setCurrentChessboardValid(bool valid)
{
	if (m_isCurrentChessboardStable && !valid)
	{
		setCurrentChessboardStable(false);
	}
	m_isCurrentChessboardValid= valid;
}

void GuiPanel_AlignmentCalibration::setCurrentChessboardStable(bool stable)
{
	if (!stable)
		m_chessboardStabilityTimer= 0.f;

	if (m_isCurrentChessboardStable != stable)
	{
		m_isCurrentChessboardStable= stable;
		if (OnChessboardStabilityChangedEvent)
			OnChessboardStabilityChangedEvent(m_isCurrentChessboardStable);
	}
}

void GuiPanel_AlignmentCalibration::updateChessboardStabilityTimer(float deltaTime)
{
	if (m_isCurrentChessboardValid && !m_isCurrentChessboardStable)
	{
		m_chessboardStabilityTimer+= deltaTime;
		if (m_chessboardStabilityTimer >= k_chessboardStabilityDuration)
		{
			setCurrentChessboardStable(true);
		}
	}
}

void GuiPanel_AlignmentCalibration::onGui()
{
	switch (m_menuState)
	{
	case eAlignmentCalibrationMenuState::pendingVideoStart:
	{
		ImGui::TextWrapped("Starting video stream...");
		ImGui::Spacing();
		if (ImGui::Button("Cancel"))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eAlignmentCalibrationMenuState::verifySetup:
	{
		ImGui::TextWrapped("Position the calibration chessboard so it is visible in the camera view.");
		ImGui::Spacing();
		ImGui::Text("Chessboard Detected: %s", m_isCurrentChessboardValid ? "Yes" : "No");
		ImGui::Text("Chessboard Stable: %s", m_isCurrentChessboardStable ? "Yes" : "No");
		ImGui::Spacing();
		if (!m_isCurrentChessboardStable)
			ImGui::BeginDisabled();
		if (ImGui::Button("Begin"))
		{
			if (OnBeginEvent)
				OnBeginEvent();
		}
		if (!m_isCurrentChessboardStable)
			ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eAlignmentCalibrationMenuState::capture:
	{
		ImGui::Text("Capturing calibration samples...");
		ImGui::Spacing();
		ImGui::ProgressBar(m_calibrationPercent / 100.f);
		ImGui::Spacing();
		if (ImGui::Button("Cancel"))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eAlignmentCalibrationMenuState::testCalibration:
	{
		ImGui::Text("Camera alignment calibrated successfully!");
		ImGui::Spacing();
		if (ImGui::Button("Restart"))
		{
			if (OnRestartEvent)
				OnRestartEvent();
		}
		ImGui::SameLine();
		if (ImGui::Button("Ok"))
		{
			if (OnReturnEvent)
				OnReturnEvent();
		}
	}
	break;

	case eAlignmentCalibrationMenuState::failedVideoStartStreamRequest:
	{
		ImGui::TextWrapped("Error: Failed to start video stream.");
		ImGui::Spacing();
		if (ImGui::Button("Cancel"))
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
