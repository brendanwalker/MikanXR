#pragma once

#include "Shared/RmlModel.h"
#include "SinglecastDelegate.h"
#include "VideoDisplayConstants.h"

class VideoFrameDistortionView;
class VideoSourceView;
typedef std::shared_ptr<const VideoSourceView> VideoSourceViewConstPtr;

class RmlModel_MonoCameraSettings : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext, 
		VideoSourceViewConstPtr videoSourceView);
	virtual void update() override;

	eVideoDisplayMode getVideoDisplayMode() const;
	void setVideoDisplayMode(eVideoDisplayMode newMode);

	SinglecastDelegate<void(eVideoDisplayMode)> OnVideoDisplayModeChanged;
	SinglecastDelegate<void(int)> OnBrightnessChanged;

private:
	// Model Variables
	Rml::Vector<Rml::String> m_videoDisplayModes;
	int m_videoDisplayMode;
	int m_brightness;
	int m_brightnessMin;
	int m_brightnessMax;
	int m_brightnessStep;
};
