#pragma once

#include "IVideoDevice.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"

class RmlModel_USBVideoSourceComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_USBVideoSourceComponent();

	USBVideoSourceSystemPtr getUSBVideoSourceSystem() const;
	USBVideoSourceSystemConfigPtr getUSBVideoSourceSystemConfig() const;
	USBVideoSourceComponentPtr getUSBVideoSourceComponent() const;

	// -- RmlModel_MikanComponent --
	virtual bool init(Rml::Context* rmlContext) override;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;
	virtual bool setComponent(MikanComponentPtr component) override;
	virtual void onComponentPropertyChanged(
		IEntityAccessorPtr accessorPtr,
		const ConfigPropertyChangeSet& changedPropertySet) override;

protected:
	void refreshSettings();

private:
	RmlDataBinding_VideoSourceSettingPtr m_videoSourceSettings[(int)eVideoSettingType::COUNT];
	RmlDataBinding_USBDevicePathListPtr m_usbDevicePathList;
	RmlDataBinding_VideoModeListPtr m_videoModeNameList;
};