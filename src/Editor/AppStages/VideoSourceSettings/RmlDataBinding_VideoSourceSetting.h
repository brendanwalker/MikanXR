#pragma once

#include "ComponentFwd.h"
#include "IVideoDevice.h"
#include "Shared\RmlDataBinding.h"

class RmlDataBinding_VideoSourceSetting : public RmlDataBinding
{
public:
	RmlDataBinding_VideoSourceSetting(eVideoSettingType videoSettingType);

	virtual bool init(Rml::DataModelConstructor constructor) override;

	int getPropertyIntValue() const;
	void setPropertyIntValue(int newValue);

	VideoSourceComponentPtr getVideoSourceComponent() const;
	void setVideoSourceComponent(VideoSourceComponentPtr videoSourceComponent);
	void refreshDataBinding();

protected:
	void handlePercentValueChanged(float newPercentValue);

private:
	static const std::string k_videoSettingTypeStrings[(int)eVideoSettingType::COUNT];

	eVideoSettingType m_videoSettingType= eVideoSettingType::INVALID;
	Rml::String m_propertyPercentValueName;
	Rml::String m_propertyValidName;

	VideoSourceComponentPtr m_videoSourceComponent;
	float m_propertyPercentValue = 0.f;
	int m_propertyIntValue = 0;
	int m_propertyIntMinValue = 0;
	int m_propertyIntMaxValue = 0;
	bool m_propertyIdValid= false;
};
