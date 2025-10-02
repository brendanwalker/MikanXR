#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"

class RmlModel_CameraComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_CameraComponent();

	virtual bool init(Rml::Context* rmlContext) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	VRTrackingVolumeDefinitionPtr getOwnerVRTrackingVolume() const;
	VideoSourceSystemPtr getVideoSourceSystem() const;
	VideoSourceSystemConfigPtr getVideoSourceSystemConfig() const;
	CameraComponentPtr getCameraComponent() const;

private:
	RmlDataBinding_ComponentIdListPtr m_trackingMountIdList;
	RmlDataBinding_ComponentIdListPtr m_videoSourceIdList;
	VideoSourceSystemWeakPtr m_videoSourceSystem;
};