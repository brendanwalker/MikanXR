#pragma once

#include "Constants_ModelFastenerCalibration.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlModel_ModelFastenerCalibration : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext);
	virtual void dispose() override;

	bool getBypassCalibrationFlag() const;
	void setBypassCalibrationFlag(const bool bNewFlag);

	eModelFastenerCalibrationMenuState getMenuState() const;
	void setMenuState(eModelFastenerCalibrationMenuState newState);

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
