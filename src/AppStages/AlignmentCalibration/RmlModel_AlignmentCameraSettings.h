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
	virtual void update() override;

	bool getShowVideoDisplayMode() const;
	void setShowVideoDisplayMode(bool bNewFlag);

	eVideoDisplayMode getVideoDisplayMode() const;
	void setVideoDisplayMode(eVideoDisplayMode newMode);

	eAlignmentCalibrationViewpointMode getViewpointMode() const;
	void setViewpointMode(eAlignmentCalibrationViewpointMode newMode);

	int getVRFrameDelay() const;
	void setVRFrameDelay(int newFrameDelay);

	SinglecastDelegate<void(eVideoDisplayMode)> OnVideoDisplayModeChanged;
	SinglecastDelegate<void(eAlignmentCalibrationViewpointMode)> OnViewpointModeChanged;
	SinglecastDelegate<void(int)> OnBrightnessChanged;
	SinglecastDelegate<void(int)> OnVRFrameDelayChanged;

private:
	// Model Variables
	bool m_bShowVideoDisplayMode= false;
	Rml::Vector<Rml::String> m_videoDisplayModes;
	int m_videoDisplayMode;
	Rml::Vector<Rml::String> m_viewpointModes;
	int m_vrFrameDelay;
	int m_viewpointMode;
	int m_brightness;
	int m_brightnessMin;
	int m_brightnessMax;
	int m_brightnessStep;
};
