#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_ComponentList.h"

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

private:
	RmlDataBinding_ComponentListPtr m_trackingMountIdList;
	RmlDataBinding_ComponentListPtr m_videoSourceIdList;
	VideoSourceSystemWeakPtr m_videoSourceSystem;
};

using RmlModel_CameraComponentPtr = std::shared_ptr<RmlModel_CameraComponent>;