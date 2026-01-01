#include "RmlDataBinding_VideoSourceSetting.h"
#include "MathUtility.h"
#include "VideoSourceComponent.h"

#include <RmlUi/Core/DataModelHandle.h>
#include <RmlUi/Core/Core.h>
#include <RmlUi/Core/Context.h>

const std::string RmlDataBinding_VideoSourceSetting::k_videoSettingTypeStrings[(int)eVideoSettingType::COUNT] = {
	"brightness",
	"contrast",
	"hue",
	"saturation",
	"sharpness",
	"gamma",
	"white_balance",
	"red_balance",
	"green_balance",
	"blue_balance",
	"gain",
	"pan",
	"tilt",
	"roll",
	"zoom",
	"exposure",
	"iris",
	"focus",
};

RmlDataBinding_VideoSourceSetting::RmlDataBinding_VideoSourceSetting(
	eVideoSettingType videoSettingType)
	: RmlDataBinding()
	, m_videoSettingType(videoSettingType)
{
}

bool RmlDataBinding_VideoSourceSetting::init(Rml::DataModelConstructor constructor)
{
	if (!RmlDataBinding::init(constructor))
	{
		return false;
	}

	const Rml::String propertyName = k_videoSettingTypeStrings[(int)m_videoSettingType];
	m_propertyPercentValueName = propertyName + "_percent";
	m_propertyValidName = propertyName + "_valid";

	// Register Data Model Fields
	constructor.Bind(m_propertyPercentValueName, &m_propertyPercentValue);
	constructor.Bind(m_propertyValidName, &m_propertyIdValid);

	// Bind data model callbacks	
	constructor.BindEventCallback(
		propertyName+"_changed",
		[this](Rml::DataModelHandle model, Rml::Event& ev, const Rml::VariantList& arguments) {
		if (ev.GetId() == Rml::EventId::Change)
		{
			const float newPercentValue = ev.GetParameter<float>("value", 0.f);

			handlePercentValueChanged(newPercentValue);
		}
	});

	return true;
}

int RmlDataBinding_VideoSourceSetting::getPropertyIntValue() const
{
	return m_propertyIntValue;
}

void RmlDataBinding_VideoSourceSetting::setPropertyIntValue(int newIntValue)
{
	if (newIntValue != m_propertyIntValue)
	{
		m_propertyIntValue = newIntValue;
		m_propertyPercentValue= 
			remap_int_to_int(
				m_propertyIntMinValue, m_propertyIntMaxValue, 
				0, 100, 
				m_propertyIntValue);

		m_modelHandle.DirtyVariable(m_propertyPercentValueName);
	}
}

VideoSourceComponentPtr RmlDataBinding_VideoSourceSetting::getVideoSourceComponent() const
{
	return m_videoSourceComponent;
}

void RmlDataBinding_VideoSourceSetting::setVideoSourceComponent(VideoSourceComponentPtr videoSourceComponent)
{
	if (m_videoSourceComponent != videoSourceComponent)
	{
		m_videoSourceComponent = videoSourceComponent;

		refreshDataBinding();
	}
}

void RmlDataBinding_VideoSourceSetting::refreshDataBinding()
{
	VideoSettingConstraint constraint;

	if (m_videoSourceComponent != nullptr &&
		m_videoSourceComponent->getVideoSettingConstraint(m_videoSettingType, constraint))
	{
		m_propertyIdValid = true;
		m_propertyIntMinValue = constraint.min_value;
		m_propertyIntMaxValue = constraint.max_value;

		const int currentIntValue = m_videoSourceComponent->getVideoSetting(m_videoSettingType);
		setPropertyIntValue(currentIntValue);
	}
	else
	{
		m_propertyIdValid = false;
	}

	m_modelHandle.DirtyVariable(m_propertyValidName);
}

void RmlDataBinding_VideoSourceSetting::handlePercentValueChanged(float newPercentValue)
{
	const int propertyIntValue = 
		remap_int_to_int(
			0, 100, 
			m_propertyIntMinValue, m_propertyIntMaxValue, 
			newPercentValue);

	if (propertyIntValue != m_propertyIntValue)
	{
		m_propertyIntValue = propertyIntValue;
		m_propertyPercentValue = newPercentValue;

		if (m_videoSourceComponent != nullptr)
		{
			m_videoSourceComponent->setVideoSetting(m_videoSettingType, m_propertyIntValue);
		}
	}
}