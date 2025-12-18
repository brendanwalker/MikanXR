#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "TrackingMountComponent.h"

class RmlModel_TrackingMountComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_TrackingMountComponent();

	virtual bool init(Rml::Context* rmlContext) override;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	VRObjectSystemPtr getVRObjectSystem() const;
	VRObjectSystemConfigPtr getVRObjectSystemConfig() const;
	VRDeviceComponentPtr getVRDeviceComponent() const;
	TrackingMountComponentPtr getTrackingMountComponent() const;

private:
	RmlDataBinding_VRDevicePathListPtr m_vrDevicePathList;
	RmlDataBinding_SocketNameListPtr m_socketNameList;
	VRObjectSystemWeakPtr m_vrObjectSystem;
};

using RmlModel_TrackingMountComponentPtr = std::shared_ptr<RmlModel_TrackingMountComponent>;