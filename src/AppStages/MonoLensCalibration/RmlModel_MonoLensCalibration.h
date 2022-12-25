#pragma once

#include "Constants_MonoLensCalibration.h"
#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "VideoDisplayConstants.h"

class RmlModel_MonoLensCalibration : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext,
		class MonoLensDistortionCalibrator* monoLensCalibrator);
	virtual void dispose() override;
	virtual void update() override;

	bool getBypassCalibrationFlag() const;
	eMonoLensCalibrationMenuState getMenuState() const;

	void setMenuState(eMonoLensCalibrationMenuState newState);
	void setCalibrationProgress(const float newProgress);
	void setCurrentImagePointsValid(const bool bNewImagePointsValid);
	void setReprojectionError(const float newReprojectionError);
	void setBypassCalibrationFlag(const bool bNewFlag);
	void resetCalibrationState();

	SinglecastDelegate<void()> OnRestartEvent;
	SinglecastDelegate<void()> OnGotoMainMenuEvent;

private:
	class MonoLensDistortionCalibrator* m_monoLensCalibrator;

	int m_menuState = 0;
	bool m_areCurrentImagePointsValid = false;
	float m_calibrationProgress = 0.f;
	float m_reprojectionError = 0.f;
	bool m_bypassCalibrationFlag = false;
};
