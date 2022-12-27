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

	SinglecastDelegate<void()> OnBeginEvent;
	SinglecastDelegate<void()> OnRestartEvent;
	SinglecastDelegate<void()> OnCancelEvent;
	SinglecastDelegate<void()> OnReturnEvent;

private:
	int m_menuState = 0;
	float m_calibrationPercent = 0.f;
	bool m_bypassCalibrationFlag = false;
};
