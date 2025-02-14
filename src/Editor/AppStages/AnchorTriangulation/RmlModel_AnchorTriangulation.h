#pragma once

#include "Constants_AnchorTriangulation.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlModel_AnchorTriangulation : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext);
	virtual void dispose() override;

	bool getBypassCalibrationFlag() const;
	void setBypassCalibrationFlag(const bool bNewFlag);

	eAnchorTriangulationMenuState getMenuState() const;
	void setMenuState(eAnchorTriangulationMenuState newState);

	int getCapturedPointCount() const;
	void setCapturedPointCount(const int newCount);

	SinglecastDelegate<void()> OnOkEvent;
	SinglecastDelegate<void()> OnRedoEvent;
	SinglecastDelegate<void()> OnCancelEvent;
	
	Rml::String m_menuState;
private:
	Rml::String m_viewpointMode;
	int m_capturedPointCount = 0;
	bool m_bypassCalibrationFlag = false;
};
