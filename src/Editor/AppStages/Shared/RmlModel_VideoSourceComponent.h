#pragma once

#include "IVideoDevice.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"

class RmlModel_NetworkVideoSourceComponent : public RmlModel_MikanComponent
{
public:
	virtual bool init(Rml::Context* rmlContext) override;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;

protected:
	NetworkVideoSourceComponentPtr getNetworkVideoSourceComponent() const;
};

class RmlModel_USBVideoSourceComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_USBVideoSourceComponent();

	virtual bool init(Rml::Context* rmlContext) override;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	VideoSourceSystemPtr getVideoSourceSystem() const;
	VideoSourceSystemConfigPtr getVideoSourceSystemConfig() const;
	USBVideoSourceSystemPtr getUSBVideoSourceSystem() const;
	USBVideoSourceComponentPtr getUSBVideoSourceComponent() const;

	void refreshSettings();

private:
	RmlDataBinding_VideoSourceSettingPtr m_videoSourceSettings[(int)eVideoSettingType::COUNT];
	RmlDataBinding_USBDevicePathListPtr m_usbDevicePathList;
	RmlDataBinding_VideoModeListPtr m_videoModeNameList;
};