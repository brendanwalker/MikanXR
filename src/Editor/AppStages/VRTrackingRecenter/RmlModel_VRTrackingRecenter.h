#pragma once

#include "Constants_VRTrackingRecenter.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"

class RmlModel_VRTrackingRecenter : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext);
	virtual void dispose() override;

	eVRTrackingRecenterMenuState getMenuState() const;
	void setMenuState(eVRTrackingRecenterMenuState newState);

	float getCalibrationFraction() const;
	void setCalibrationFraction(const float newFraction);

	bool getCurrentMarkerValid() const;
	void setCurrentMarkerValid(const bool bNewChessboardValid);

	void updateMarkerStabilityTimer(float deltaTime);
	void setCurrentMarkerStable(const bool bNewChessboardStability);
	bool getCurrentMarkerStable() const;

	SinglecastDelegate<void()> OnBeginEvent;
	SinglecastDelegate<void()> OnRestartEvent;
	SinglecastDelegate<void()> OnCancelEvent;
	SinglecastDelegate<void()> OnReturnEvent;
	SinglecastDelegate<void(bool)> OnMarkerStabilityChangedEvent;

private:
	Rml::String m_menuState;
	float m_calibrationPercent = 0.f;
	bool m_isCurrentMarkerValid = false;
	bool m_isCurrentMarkerStable = false;
	float m_markerStabilityTimer = 0.f;
};
