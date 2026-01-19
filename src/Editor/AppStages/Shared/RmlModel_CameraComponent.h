#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "CameraComponent.h"

class RmlModel_CameraComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_CameraComponent();

	virtual bool init(class AppStage* ownerAppStage) override;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	VRTrackingVolumeDefinitionPtr getOwnerVRTrackingVolume() const;
	CameraComponentPtr getCameraComponent() const;

private:
	RmlDataBinding_ComponentIdListPtr m_trackingMountIdList;
	RmlDataBinding_ComponentIdListPtr m_textureSourceIdList;
};