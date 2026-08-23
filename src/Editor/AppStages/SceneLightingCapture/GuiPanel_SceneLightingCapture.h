#pragma once

#include "Constants_SceneLightingCapture.h"
#include "SceneLightingEstimator.h"
#include "Shared/GuiPanel.h"

#include <functional>
#include <string>

class GuiPanel_SceneLightingCapture : public GuiPanel
{
public:
	GuiPanel_SceneLightingCapture(class AppStage* ownerAppStage)
		: GuiPanel(ownerAppStage)
	{
	}
	virtual void onGui() override;

	eSceneLightingCaptureMenuState getMenuState() const { return m_menuState; }
	void setMenuState(eSceneLightingCaptureMenuState newState) { m_menuState= newState; }

	void setProbeName(const std::string& name) { m_probeName= name; }
	void setExecutionProvider(const std::string& provider) { m_executionProvider= provider; }
	void setFailureReason(const std::string& reason) { m_failureReason= reason; }

	/// Populated after a successful estimate so the operator can judge it
	/// before committing.
	void setEstimateSummary(float directionality, const std::string& keyDirection, const std::string& ambient,
							int sampleCount, float negativeSolidAngleFraction)
	{
		m_directionality= directionality;
		m_keyDirection= keyDirection;
		m_ambient= ambient;
		m_sampleCount= sampleCount;
		m_negativeSolidAngleFraction= negativeSolidAngleFraction;
	}

	/// Pushed every frame while an estimate runs so the progress readout tracks
	/// the worker.
	void setEstimateProgress(eSceneLightingEstimatePhase phase, float fraction, float elapsedSeconds, bool bCancelling)
	{
		m_estimatePhase= phase;
		m_estimateFraction= fraction;
		m_estimateElapsedSeconds= elapsedSeconds;
		m_bCancellingEstimate= bCancelling;
	}

	std::function<void()> OnCaptureEvent;
	std::function<void()> OnCancelCaptureEvent;
	std::function<void()> OnApplyEvent;
	std::function<void()> OnRedoEvent;
	std::function<void()> OnCancelEvent;
	std::function<void()> OnOkEvent;

private:
	eSceneLightingCaptureMenuState m_menuState= eSceneLightingCaptureMenuState::inactive;
	std::string m_probeName;
	std::string m_executionProvider;
	std::string m_failureReason;

	float m_directionality= 0.f;
	std::string m_keyDirection;
	std::string m_ambient;
	int m_sampleCount= 0;
	float m_negativeSolidAngleFraction= 0.f;

	eSceneLightingEstimatePhase m_estimatePhase= eSceneLightingEstimatePhase::idle;
	float m_estimateFraction= 0.f;
	float m_estimateElapsedSeconds= 0.f;
	bool m_bCancellingEstimate= false;
};
