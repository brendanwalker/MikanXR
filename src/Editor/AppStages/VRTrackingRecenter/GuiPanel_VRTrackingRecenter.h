#pragma once

#include "Constants_VRTrackingRecenter.h"
#include "Shared/GuiPanel.h"

#include <functional>

class AppStage;

class GuiPanel_VRTrackingRecenter : public GuiPanel
{
public:
	GuiPanel_VRTrackingRecenter(AppStage* ownerAppStage) : GuiPanel(ownerAppStage) {}
	virtual void onGui() override;

	eVRTrackingRecenterMenuState getMenuState() const { return m_menuState; }
	void setMenuState(eVRTrackingRecenterMenuState newState) { m_menuState = newState; }

	void setCalibrationFraction(float fraction) { m_calibrationPercent = fraction * 100.f; }

	bool getCurrentMarkerValid() const { return m_isCurrentMarkerValid; }
	void setCurrentMarkerValid(bool valid);

	bool getCurrentMarkerStable() const { return m_isCurrentMarkerStable; }
	void setCurrentMarkerStable(bool stable);
	void updateMarkerStabilityTimer(float deltaTime);

	std::function<void()> OnBeginEvent;
	std::function<void()> OnRestartEvent;
	std::function<void()> OnCancelEvent;
	std::function<void()> OnReturnEvent;
	std::function<void(bool)> OnMarkerStabilityChangedEvent;

private:
	static constexpr float k_markerStabilityDuration = 1.0f;

	eVRTrackingRecenterMenuState m_menuState = eVRTrackingRecenterMenuState::inactive;
	float m_calibrationPercent = 0.f;
	bool m_isCurrentMarkerValid = false;
	bool m_isCurrentMarkerStable = false;
	float m_markerStabilityTimer = 0.f;
};
