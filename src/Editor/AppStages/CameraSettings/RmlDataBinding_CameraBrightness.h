#pragma once

#include "Shared\RmlDataBinding.h"
#include "SinglecastDelegate.h"

class VideoSourceView;
typedef std::shared_ptr<VideoSourceView> VideoSourceViewPtr;

class RmlDataBinding_CameraBrightness : public RmlDataBinding
{
public:
	RmlDataBinding_CameraBrightness()= default;
	virtual ~RmlDataBinding_CameraBrightness();

	virtual bool init(Rml::DataModelConstructor constructor) override;
	virtual void dispose() override;

	int getBrightness() const;
	void setBrightness(int newBrightness);

	VideoSourceViewPtr getVideoSourceView() const;
	void setVideoSourceView(VideoSourceViewPtr videoSourceView);

	SinglecastDelegate<void(int)> OnBrightnessChanged;

protected:
	void handleBrightnessPercentChanged(float newPercentValue);

protected:
	VideoSourceViewPtr m_videoSourceView;
	int m_brightness = 0;
	int m_brightnessMin = 0;
	int m_brightnessMax = 0;
	int m_brightnessPercent= 0;
	bool m_brightnessValid= false;
};
