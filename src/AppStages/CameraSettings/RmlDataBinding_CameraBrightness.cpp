#include "RmlDataBinding_CameraBrightness.h"
#include "MathUtility.h"
#include "VideoSourceView.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

RmlDataBinding_CameraBrightness::~RmlDataBinding_CameraBrightness()
{
	dispose();
}

bool RmlDataBinding_CameraBrightness::init(Rml::DataModelConstructor constructor)
{
	if (!RmlDataBinding::init(constructor))
	{
		return false;
	}

	// Register Data Model Fields
	constructor.Bind("brightness_percent", &m_brightnessPercent);
	constructor.Bind("brightness_valid", &m_brightnessValid);

	// Bind data model callbacks	
	constructor.BindEventCallback(
		"brightness_changed",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
		if (ev.GetId() == Rml::EventId::Change)
		{
			const float newBrightnessPercent = ev.GetParameter<float>("value", 0.f);
			handleBrightnessPercentChanged(newBrightnessPercent);
		}
	});

	return true;
}

void RmlDataBinding_CameraBrightness::dispose()
{
	OnBrightnessChanged.Clear();
	RmlDataBinding::dispose();
}

int RmlDataBinding_CameraBrightness::getBrightness() const
{
	return m_brightness;
}

void RmlDataBinding_CameraBrightness::setBrightness(int newBrightness)
{
	if (newBrightness != m_brightness)
	{
		m_brightness = newBrightness;
		m_brightnessPercent= remap_int_to_int(m_brightnessMin, m_brightnessMax, 0, 100, m_brightness);

		m_modelHandle.DirtyVariable("brightness_percent");
	}
}


VideoSourceViewPtr RmlDataBinding_CameraBrightness::getVideoSourceView() const
{
	return m_videoSourceView;
}

void RmlDataBinding_CameraBrightness::setVideoSourceView(VideoSourceViewPtr videoSourceView)
{
	if (m_videoSourceView != videoSourceView)
	{
		if (videoSourceView != nullptr)
		{
			m_brightnessValid= true;
			m_brightnessMin = videoSourceView->getVideoPropertyConstraintMinValue(VideoPropertyType::Brightness);
			m_brightnessMax = videoSourceView->getVideoPropertyConstraintMaxValue(VideoPropertyType::Brightness);
			setBrightness(videoSourceView->getVideoProperty(VideoPropertyType::Brightness));
			m_modelHandle.DirtyVariable("brightness_valid");
		}
		else
		{
			m_brightnessValid= false;
			m_modelHandle.DirtyVariable("brightness_valid");
		}

		m_videoSourceView = videoSourceView;
	}
}

void RmlDataBinding_CameraBrightness::handleBrightnessPercentChanged(float newPercentValue)
{
	m_brightnessPercent = newPercentValue;
	m_brightness = remap_int_to_int(0, 100, m_brightnessMin, m_brightnessMax, m_brightnessPercent);

	if (m_videoSourceView)
	{
		m_videoSourceView->setVideoProperty(VideoPropertyType::Brightness, m_brightness, true);
	}

	if (OnBrightnessChanged)
	{
		OnBrightnessChanged(m_brightness);
	}
}