#pragma once

#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "NetworkVideoSourceComponent.h"
#include "USBVideoSourceComponent.h"

class RmlModel_NetworkVideoSourceComponent : 
	public RmlModel_TypedMikanComponent<NetworkVideoSourceComponent>
{
};

class RmlModel_USBVideoSourceComponent : 
	public RmlModel_TypedMikanComponent<USBVideoSourceComponent>
{
public:
	RmlModel_USBVideoSourceComponent();

	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	VideoSourceSystemPtr getVideoSourceSystem() const;
	VideoSourceSystemConfigPtr getVideoSourceSystemConfig() const;
	USBVideoSourceSystemPtr getUSBVideoSourceSystem() const;
	USBVideoSourceComponentPtr getUSBVideoSourceComponent() const;

private:
	RmlDataBinding_USBDevicePathListPtr m_usbDevicePathList;
	RmlDataBinding_VideoModeListPtr m_videoModeNameList;
};