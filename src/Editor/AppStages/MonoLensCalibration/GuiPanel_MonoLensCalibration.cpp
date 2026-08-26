#include "MonoLensCalibration/GuiPanel_MonoLensCalibration.h"

#include "imgui.h"
#include "LocText.h"

void GuiPanel_MonoLensCalibration::setCurrentImagePointsValid(bool valid)
{
	if (m_areCurrentImagePointsStable && !valid)
	{
		setCurrentImagePointsStable(false);
	}
	m_areCurrentImagePointsValid= valid;
}

void GuiPanel_MonoLensCalibration::setCurrentImagePointsStable(bool stable)
{
	if (!stable)
		m_imagePointsStabilityTimer= 0.f;

	if (m_areCurrentImagePointsStable != stable)
	{
		m_areCurrentImagePointsStable= stable;
		if (OnImagePointStabilityChangedEvent)
			OnImagePointStabilityChangedEvent(m_areCurrentImagePointsStable);
	}
}

void GuiPanel_MonoLensCalibration::updateImagePointStabilityTimer(float deltaTime)
{
	if (m_areCurrentImagePointsValid && !m_areCurrentImagePointsStable)
	{
		m_imagePointsStabilityTimer+= deltaTime;
		if (m_imagePointsStabilityTimer >= k_imagePointStabilityDuration)
		{
			setCurrentImagePointsStable(true);
		}
	}
}

void GuiPanel_MonoLensCalibration::resetCalibrationState()
{
	m_calibrationPercent= 0.f;
	m_reprojectionError= 0.f;
	setCurrentImagePointsValid(false);
}

void GuiPanel_MonoLensCalibration::onGui()
{
	switch (m_menuState)
	{
	case eMonoLensCalibrationMenuState::capture:
	{
		ImGui::Text(locText("monoLensCalibration.patternDetectedFmt"), m_areCurrentImagePointsValid
																		   ? locText("monoLensCalibration.yes")
																		   : locText("monoLensCalibration.no"));
		ImGui::Text(locText("monoLensCalibration.patternStableFmt"), m_areCurrentImagePointsStable
																		 ? locText("monoLensCalibration.yes")
																		 : locText("monoLensCalibration.no"));
		ImGui::Spacing();
		ImGui::TextUnformatted(locText("monoLensCalibration.holdBoardSteady"));
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

	case eMonoLensCalibrationMenuState::processingCalibration:
	{
		ImGui::TextUnformatted(locText("monoLensCalibration.processingCalibration"));
	}
	break;

	case eMonoLensCalibrationMenuState::testCalibration:
	{
		ImGui::TextUnformatted(locText("monoLensCalibration.calibrationComplete"));
		ImGui::Text(locText("monoLensCalibration.reprojectionErrorFmt"), m_reprojectionError);
		ImGui::Spacing();
		if (ImGui::Button(locLabel("monoLensCalibration.restart")))
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

	case eMonoLensCalibrationMenuState::failedCalibration:
	{
		ImGui::TextWrapped("%s", locText("monoLensCalibration.calibrationFailed"));
		ImGui::Spacing();
		if (ImGui::Button(locLabel("monoLensCalibration.restart")))
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

	case eMonoLensCalibrationMenuState::failedVideoStartStreamRequest:
	{
		ImGui::TextWrapped("%s", locText("monoLensCalibration.failedVideoStream"));
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
