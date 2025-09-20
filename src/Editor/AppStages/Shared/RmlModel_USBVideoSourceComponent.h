#pragma once

#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"

class RmlModel_USBVideoSourceComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_USBVideoSourceComponent();

	virtual bool init(Rml::Context* rmlContext) override;
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