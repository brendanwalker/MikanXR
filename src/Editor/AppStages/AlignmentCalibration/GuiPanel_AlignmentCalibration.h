#pragma once

#include "Constants_AlignmentCalibration.h"
#include "Shared/GuiPanel.h"

#include <functional>

class AppStage;

class GuiPanel_AlignmentCalibration : public GuiPanel
{
public:
	bool init(AppStage* ownerAppStage);
	virtual void onGui() override;

	bool getBypassCalibrationFlag() const { return m_bypassCalibrationFlag; }
	void setBypassCalibrationFlag(bool flag) { m_bypassCalibrationFlag = flag; }

	eAlignmentCalibrationMenuState getMenuState() const { return m_menuState; }
	void setMenuState(eAlignmentCalibrationMenuState newState) { m_menuState = newState; }

	void setCalibrationFraction(float fraction) { m_calibrationPercent = fraction * 100.f; }

	bool getCurrentChessboardValid() const { return m_isCurrentChessboardValid; }
	void setCurrentChessboardValid(bool valid);

	bool getCurrentChessboardStable() const { return m_isCurrentChessboardStable; }
	void setCurrentChessboardStable(bool stable);
	void updateChessboardStabilityTimer(float deltaTime);

	std::function<void()> OnBeginEvent;
	std::function<void()> OnRestartEvent;
	std::function<void()> OnCancelEvent;
	std::function<void()> OnReturnEvent;
	std::function<void(bool)> OnChessboardStabilityChangedEvent;

private:
	static constexpr float k_chessboardStabilityDuration = 1.0f;

	eAlignmentCalibrationMenuState m_menuState = eAlignmentCalibrationMenuState::inactive;
	float m_calibrationPercent = 0.f;
	bool m_bypassCalibrationFlag = false;
	bool m_isCurrentChessboardValid = false;
	bool m_isCurrentChessboardStable = false;
	float m_chessboardStabilityTimer = 0.f;
};
