#include "RmlDataBinding_CameraBrightness.h"
#include "MathUtility.h"
#include "VideoSourceComponent.h"

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


VideoSourceComponentPtr RmlDataBinding_CameraBrightness::getVideoSourceComponent() const
{
	return m_videoSourceComponent;
}

void RmlDataBinding_CameraBrightness::setVideoSourceComponent(VideoSourceComponentPtr videoSourceComponent)
{
	if (m_videoSourceComponent != videoSourceComponent)
	{
		VideoSettingConstraint constraint;
		if (videoSourceComponent != nullptr &&
			videoSourceComponent->getVideoSettingConstraint(eVideoSettingType::Brightness, constraint))
		{
			m_brightnessValid= true;
			m_brightnessMin = constraint.min_value;
			m_brightnessMax = constraint.max_value;
			setBrightness(videoSourceComponent->getVideoSetting(eVideoSettingType::Brightness));
			m_modelHandle.DirtyVariable("brightness_valid");
		}
		else
		{
			m_brightnessValid= false;
			m_modelHandle.DirtyVariable("brightness_valid");
		}

		m_videoSourceComponent = videoSourceComponent;
	}
}

void RmlDataBinding_CameraBrightness::handleBrightnessPercentChanged(float newPercentValue)
{
	m_brightnessPercent = newPercentValue;
	m_brightness = remap_int_to_int(0, 100, m_brightnessMin, m_brightnessMax, m_brightnessPercent);

	if (m_videoSourceComponent)
	{
		m_videoSourceComponent->setVideoSetting(eVideoSettingType::Brightness, m_brightness);
	}

	if (OnBrightnessChanged)
	{
		OnBrightnessChanged(m_brightness);
	}
}