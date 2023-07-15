#pragma once

#include "Constants_CTOffsetCalibration.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlModel_CTOffsetCalibration : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext);
	virtual void dispose() override;

	bool getBypassCalibrationFlag() const;
	void setBypassCalibrationFlag(const bool bNewFlag);

	eCTOffsetCalibrationMenuState getMenuState() const;
	void setMenuState(eCTOffsetCalibrationMenuState newState);

	float getCalibrationFraction() const;
	void setCalibrationFraction(const float newFraction);

	SinglecastDelegate<void()> OnContinueEvent;
	SinglecastDelegate<void()> OnCancelEvent;
	SinglecastDelegate<void()> OnCaptureEvent;
	SinglecastDelegate<void()> OnRestartEvent;

private:
	Rml::String m_menuState;
	Rml::String m_viewpointMode;
	float m_calibrationPercent = 0.f;
	bool m_bypassCalibrationFlag = false;
};
