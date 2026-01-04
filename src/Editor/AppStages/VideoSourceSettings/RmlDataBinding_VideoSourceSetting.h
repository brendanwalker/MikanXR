#pragma once

#include "ComponentFwd.h"
#include "CommonConfigFwd.h"
#include "IVideoDevice.h"
#include "Shared\RmlDataBinding.h"

class RmlDataBinding_VideoSourceSetting : public RmlDataBinding
{
public:
	RmlDataBinding_VideoSourceSetting(
		class RmlModel_USBVideoSourceComponent* ownerComponentModel,
		eVideoSettingType videoSettingType);

	virtual bool init(Rml::DataModelConstructor constructor) override;

	float getPropertyPercentValue() const;
	void setPropertyPercentValue(float newValue);

	void refreshDataBinding();

protected:
	void handlePercentValueChanged(float newPercentValue);

private:
	static const std::string k_videoSettingTypeStrings[(int)eVideoSettingType::COUNT];

	eVideoSettingType m_videoSettingType= eVideoSettingType::INVALID;
	Rml::String m_propertyPercentValueName;
	Rml::String m_propertyValidName;

	class RmlModel_USBVideoSourceComponent* m_ownerComponentModel;
	float m_propertyPercentValue = 0.f;
	bool m_propertyIdValid= false;
};
