#include "RmlDataBinding_VideoSourceSetting.h"
#include "MathUtility.h"
#include "MulticastDelegate.h"
#include "Shared/RmlModel_VideoSourceComponent.h"
#include "USBVideoSourceComponent.h"
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
	class RmlModel_USBVideoSourceComponent* ownerComponentModel,
	eVideoSettingType videoSettingType)
	: RmlDataBinding()
	, m_ownerComponentModel(ownerComponentModel)
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

float RmlDataBinding_VideoSourceSetting::getPropertyPercentValue() const
{
	return m_propertyPercentValue;
}

void RmlDataBinding_VideoSourceSetting::setPropertyPercentValue(float newPercentValue)
{
	if (newPercentValue != m_propertyPercentValue)
	{
		m_propertyPercentValue = newPercentValue;
		m_modelHandle.DirtyVariable(m_propertyPercentValueName);
	}
}

void RmlDataBinding_VideoSourceSetting::refreshDataBinding()
{
	bool propertyValid = false;
	auto videoSourceComponent = m_ownerComponentModel->getUSBVideoSourceComponent();

	if (float fractionValue= 0.f;
		videoSourceComponent != nullptr &&
		videoSourceComponent->getVideoSetting(m_videoSettingType, fractionValue))
	{
		const float percentValue = clampf(fractionValue * 100.f, 0.f, 100.f);

		setPropertyPercentValue(percentValue);
		propertyValid = true;
	}
	else
	{
		propertyValid = false;
	}

	if (m_propertyIdValid != propertyValid)
	{
		m_propertyIdValid = propertyValid;
		m_modelHandle.DirtyVariable(m_propertyValidName);
	}
}

void RmlDataBinding_VideoSourceSetting::handlePercentValueChanged(float newPercentValue)
{
	if (fabsf(newPercentValue - m_propertyPercentValue) > 0.001f)
	{
		m_propertyPercentValue = newPercentValue;

		auto videoSourceComponent = m_ownerComponentModel->getUSBVideoSourceComponent();
		if (videoSourceComponent != nullptr)
		{
			const float fractionValue = clampf01(newPercentValue / 100.f);

			videoSourceComponent->setVideoSetting(m_videoSettingType, fractionValue);
		}
	}
}