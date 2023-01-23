#pragma once

#pragma once

#include "SinglecastDelegate.h"
#include "Shared/RmlModel.h"
#include "VideoDisplayConstants.h"
#include "Constants_AlignmentCalibration.h"

class ProfileConfig;

class VideoSourceView;
typedef std::shared_ptr<const VideoSourceView> VideoSourceViewConstPtr;

class RmlModel_AlignmentCameraSettings : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext,
		VideoSourceViewConstPtr videoSourceView,
		const ProfileConfig* profileConfig);
	virtual void dispose() override;

	eAlignmentCalibrationMenuState getMenuState() const;
	void setMenuState(eAlignmentCalibrationMenuState newState);

	eAlignmentCalibrationViewpointMode getViewpointMode() const;
	void setViewpointMode(eAlignmentCalibrationViewpointMode newMode);

	int getBrightness() const;
	void setBrightness(int newBrightness);

	int getVRFrameDelay() const;
	void setVRFrameDelay(int newFrameDelay);

	SinglecastDelegate<void(eAlignmentCalibrationViewpointMode)> OnViewpointModeChanged;
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
