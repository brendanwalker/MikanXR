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

	bool getCurrentImagePointsValid() const;
	void setCurrentImagePointsValid(const bool bNewImagePointsValid);

	SinglecastDelegate<void()> OnBeginEvent;
	SinglecastDelegate<void()> OnRestartEvent;
	SinglecastDelegate<void()> OnCancelEvent;
	SinglecastDelegate<void()> OnReturnEvent;

private:
	Rml::String m_menuState;
	Rml::String m_viewpointMode;
	bool m_areCurrentImagePointsValid = false;
	bool m_bypassCalibrationFlag = false;
};
