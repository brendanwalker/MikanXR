#pragma once

#include "ObjectSystemConfigFwd.h"
#include "SinglecastDelegate.h"
#include "Shared/RmlModel.h"
#include "VideoDisplayConstants.h"
#include "Constants_CTOffsetCalibration.h"

class ProfileConfig;

class VideoSourceView;
typedef std::shared_ptr<const VideoSourceView> VideoSourceViewConstPtr;

class RmlModel_CTOffsetCameraSettings : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext,
		VideoSourceViewConstPtr videoSourceView,
		ProfileConfigConstPtr profileConfig);
	virtual void dispose() override;

	eCTOffsetCalibrationMenuState getMenuState() const;
	void setMenuState(eCTOffsetCalibrationMenuState newState);

	eCTOffsetCalibrationViewpointMode getViewpointMode() const;
	void setViewpointMode(eCTOffsetCalibrationViewpointMode newMode);

	int getBrightness() const;
	void setBrightness(int newBrightness);

	int getVRFrameDelay() const;
	void setVRFrameDelay(int newFrameDelay);

	SinglecastDelegate<void(eCTOffsetCalibrationViewpointMode)> OnViewpointModeChanged;
	SinglecastDelegate<void(int)> OnBrightnessChanged;
	SinglecastDelegate<void(int)> OnVRFrameDelayChanged;

private:
	// Model Variables
	Rml::String m_menuState;
	Rml::String m_viewpointMode;
	int m_vrFrameDelay= 0;
	int m_brightness= 0;
	int m_brightnessMin= 0;
	int m_brightnessMax= 0;
	int m_brightnessStep= 0;
};
