#pragma once

#include "Constants_AlignmentCalibration.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlModel_AlignmentCalibration : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext);
	virtual void dispose() override;

	bool getBypassCalibrationFlag() const;
	void setBypassCalibrationFlag(const bool bNewFlag);

	eAlignmentCalibrationMenuState getMenuState() const;
	void setMenuState(eAlignmentCalibrationMenuState newState);

	float getCalibrationFraction() const;
	void setCalibrationFraction(const float newFraction);

	bool getCurrentChessboardValid() const;
	void setCurrentChessboardValid(const bool bNewChessboardValid);

	void updateChessboardStabilityTimer(float deltaTime);
	void setCurrentChessboardStable(const bool bNewChessboardStability);
	bool getCurrentChessboardStable() const;

	SinglecastDelegate<void()> OnBeginEvent;
	SinglecastDelegate<void()> OnRestartEvent;
	SinglecastDelegate<void()> OnCancelEvent;
	SinglecastDelegate<void()> OnReturnEvent;
	SinglecastDelegate<void(bool)> OnChessboardStabilityChangedEvent;

private:
	Rml::String m_menuState;
	Rml::String m_viewpointMode;
	bool m_isCurrentChessboardValid = false;
	bool m_isCurrentChessboardStable = false;
	float m_chessboardStabilityTimer = 0.f;
	float m_calibrationPercent = 0.f;
	bool m_bypassCalibrationFlag = false;
};
