#pragma once

#include "Constants_DepthMeshCapture.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlModel_DepthMeshCapture : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext);
	virtual void dispose() override;

	bool getBypassCalibrationFlag() const;
	void setBypassCalibrationFlag(const bool bNewFlag);

	eDepthMeshCaptureMenuState getMenuState() const;
	void setMenuState(eDepthMeshCaptureMenuState newState);

	float getCalibrationFraction() const;
	void setCalibrationFraction(const float newFraction);

	SinglecastDelegate<void()> OnBeginEvent;
	SinglecastDelegate<void()> OnRestartEvent;
	SinglecastDelegate<void()> OnCancelEvent;
	SinglecastDelegate<void()> OnReturnEvent;

private:
	Rml::String m_menuState;
	Rml::String m_viewpointMode;
	float m_calibrationPercent = 0.f;
	bool m_bypassCalibrationFlag = false;
};
