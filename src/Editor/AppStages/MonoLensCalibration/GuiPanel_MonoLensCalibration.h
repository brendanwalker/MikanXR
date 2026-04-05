#pragma once

#include "Constants_MonoLensCalibration.h"
#include "Shared/GuiPanel.h"

#include <functional>

class AppStage;

class GuiPanel_MonoLensCalibration : public GuiPanel
{
public:
	GuiPanel_MonoLensCalibration(AppStage* ownerAppStage) : GuiPanel(ownerAppStage) {}
	virtual void onGui() override;

	bool getBypassCalibrationFlag() const { return m_bypassCalibrationFlag; }
	void setBypassCalibrationFlag(bool flag) { m_bypassCalibrationFlag = flag; }

	eMonoLensCalibrationMenuState getMenuState() const { return m_menuState; }
	void setMenuState(eMonoLensCalibrationMenuState newState) { m_menuState = newState; }

	void setCalibrationFraction(float fraction) { m_calibrationPercent = fraction * 100.f; }

	bool getCurrentImagePointsValid() const { return m_areCurrentImagePointsValid; }
	void setCurrentImagePointsValid(bool valid);

	bool getCurrentImagePointsStable() const { return m_areCurrentImagePointsStable; }
	void setCurrentImagePointsStable(bool stable);
	void updateImagePointStabilityTimer(float deltaTime);

	float getReprojectionError() const { return m_reprojectionError; }
	void setReprojectionError(float error) { m_reprojectionError = error; }

	void resetCalibrationState();

	std::function<void()> OnCancelEvent;
	std::function<void()> OnRestartEvent;
	std::function<void()> OnReturnEvent;
	std::function<void(bool)> OnImagePointStabilityChangedEvent;

private:
	static constexpr float k_imagePointStabilityDuration = 1.0f;

	eMonoLensCalibrationMenuState m_menuState = eMonoLensCalibrationMenuState::inactive;
	bool m_areCurrentImagePointsValid = false;
	bool m_areCurrentImagePointsStable = false;
	float m_imagePointsStabilityTimer = 0.f;
	float m_calibrationPercent = 0.f;
	float m_reprojectionError = 0.f;
	bool m_bypassCalibrationFlag = false;
};
