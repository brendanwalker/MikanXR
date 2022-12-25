#pragma once

#include "Shared/RmlModel.h"
#include "VideoDisplayConstants.h"

class VideoFrameDistortionView;
class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class RmlModel_MonoCameraSettings : public RmlModel
{
public:
	bool init(
		Rml::Context* rmlContext, 
		VideoSourceViewPtr videoSourceView, 
		VideoFrameDistortionView* monoDistortionView);
	virtual void update() override;

	void setVideoDisplayMode(eVideoDisplayMode newMode);

private:
	// Data Binding Targets
	VideoSourceViewPtr m_videoSourceView;
	VideoFrameDistortionView* m_monoDistortionView;

	// Model Variables
	Rml::Vector<Rml::String> m_videoDisplayModes;
	int m_videoDisplayMode;
	int m_brightness;
	int m_brightnessMin;
	int m_brightnessMax;
	int m_brightnessStep;
};
