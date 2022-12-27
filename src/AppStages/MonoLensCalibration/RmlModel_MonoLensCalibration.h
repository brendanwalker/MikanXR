#pragma once

#include "Constants_MonoLensCalibration.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "VideoDisplayConstants.h"

class RmlModel_MonoLensCalibration : public RmlModel
{
public:
	bool init(Rml::Context* rmlContext);
	virtual void dispose() override;

	bool getBypassCalibrationFlag() const;
	void setBypassCalibrationFlag(const bool bNewFlag);

	eMonoLensCalibrationMenuState getMenuState() const;
	void setMenuState(eMonoLensCalibrationMenuState newState);

	float getCalibrationFraction() const;
	void setCalibrationFraction(const float newProgress);

	bool getCurrentImagePointsValid() const;
	void setCurrentImagePointsValid(const bool bNewImagePointsValid);

	float getReprojectionError() const;
	void setReprojectionError(const float newReprojectionError);

	void resetCalibrationState();

	SinglecastDelegate<void()> OnCancelEvent;
	SinglecastDelegate<void()> OnRestartEvent;
	SinglecastDelegate<void()> OnReturnEvent;

private:
	class MonoLensDistortionCalibrator* m_monoLensCalibrator;

	int m_menuState = 0;
	bool m_areCurrentImagePointsValid = false;
	float m_calibrationPercent = 0.f;
	float m_reprojectionError = 0.f;
	bool m_bypassCalibrationFlag = false;
};
