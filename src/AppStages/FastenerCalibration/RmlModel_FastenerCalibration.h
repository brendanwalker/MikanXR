#pragma once

#include "Constants_FastenerCalibration.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlModel_FastenerCalibration : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext);
	virtual void dispose() override;

	bool getBypassCalibrationFlag() const;
	void setBypassCalibrationFlag(const bool bNewFlag);

	eFastenerCalibrationMenuState getMenuState() const;
	void setMenuState(eFastenerCalibrationMenuState newState);

	int getCapturedPointCount() const;
	void setCapturedPointCount(const int newCount);

	SinglecastDelegate<void()> OnOkEvent;
	SinglecastDelegate<void()> OnRedoEvent;
	SinglecastDelegate<void()> OnCancelEvent;
	
private:
	Rml::String m_menuState;
	Rml::String m_viewpointMode;
	int m_capturedPointCount = 0;
	bool m_bypassCalibrationFlag = false;
};
