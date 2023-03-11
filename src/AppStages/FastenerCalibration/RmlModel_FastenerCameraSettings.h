#pragma once

#pragma once

#include "SinglecastDelegate.h"
#include "Shared/RmlModel.h"
#include "VideoDisplayConstants.h"
#include "Constants_FastenerCalibration.h"

class ProfileConfig;

class VideoSourceView;
typedef std::shared_ptr<const VideoSourceView> VideoSourceViewConstPtr;

class RmlModel_FastenerCameraSettings : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext,
		VideoSourceViewConstPtr videoSourceView,
		const ProfileConfig* profileConfig);
	virtual void dispose() override;

	eFastenerCalibrationMenuState getMenuState() const;
	void setMenuState(eFastenerCalibrationMenuState newState);

	eFastenerCalibrationViewpointMode getViewpointMode() const;
	void setViewpointMode(eFastenerCalibrationViewpointMode newMode);

	int getBrightness() const;
	void setBrightness(int newBrightness);

	SinglecastDelegate<void(eFastenerCalibrationViewpointMode)> OnViewpointModeChanged;
	SinglecastDelegate<void(int)> OnBrightnessChanged;

private:
	// Model Variables
	Rml::String m_menuState;
	Rml::String m_viewpointMode;
	int m_brightness= 0;
	int m_brightnessMin= 0;
	int m_brightnessMax= 0;
	int m_brightnessStep= 0;
};
