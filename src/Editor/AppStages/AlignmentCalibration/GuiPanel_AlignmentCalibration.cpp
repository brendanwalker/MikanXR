#include "AlignmentCalibration/GuiPanel_AlignmentCalibration.h"

#include "imgui.h"
#include "LocText.h"

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
		ImGui::TextWrapped("%s", locText("alignmentCalibration.startingVideoStream"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eAlignmentCalibrationMenuState::verifySetup:
	{
		ImGui::TextWrapped("%s", locText("alignmentCalibration.positionChessboard"));
		ImGui::Spacing();
		ImGui::Text(locText("alignmentCalibration.chessboardDetectedFmt"), m_isCurrentChessboardValid
																			   ? locText("alignmentCalibration.yes")
																			   : locText("alignmentCalibration.no"));
		ImGui::Text(locText("alignmentCalibration.chessboardStableFmt"), m_isCurrentChessboardStable
																			 ? locText("alignmentCalibration.yes")
																			 : locText("alignmentCalibration.no"));
		ImGui::Spacing();
		if (!m_isCurrentChessboardStable)
			ImGui::BeginDisabled();
		if (ImGui::Button(locLabel("alignmentCalibration.begin")))
		{
			if (OnBeginEvent)
				OnBeginEvent();
		}
		if (!m_isCurrentChessboardStable)
			ImGui::EndDisabled();
		ImGui::SameLine();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eAlignmentCalibrationMenuState::capture:
	{
		ImGui::TextUnformatted(locText("alignmentCalibration.capturingSamples"));
		ImGui::Spacing();
		ImGui::ProgressBar(m_calibrationPercent / 100.f);
		ImGui::Spacing();
		if (ImGui::Button(locLabel("common.cancel")))
		{
			if (OnCancelEvent)
				OnCancelEvent();
		}
	}
	break;

	case eAlignmentCalibrationMenuState::testCalibration:
	{
		ImGui::TextUnformatted(locText("alignmentCalibration.calibratedSuccessfully"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("alignmentCalibration.restart")))
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

	case eAlignmentCalibrationMenuState::failedVideoStartStreamRequest:
	{
		ImGui::TextWrapped("%s", locText("alignmentCalibration.failedVideoStream"));
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
