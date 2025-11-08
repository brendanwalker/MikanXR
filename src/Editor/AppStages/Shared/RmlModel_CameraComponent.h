#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "CameraComponent.h"

class RmlModel_CameraComponent : public RmlModel_TypedMikanComponent<CameraComponent>
{
public:
	RmlModel_CameraComponent();

	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	VRTrackingVolumeDefinitionPtr getOwnerVRTrackingVolume() const;
	VideoSourceSystemPtr getVideoSourceSystem() const;
	VideoSourceSystemConfigPtr getVideoSourceSystemConfig() const;
	CameraComponentPtr getCameraComponent() const;

private:
	RmlDataBinding_ComponentIdListPtr m_trackingMountIdList;
	RmlDataBinding_ComponentIdListPtr m_textureSourceIdList;
	VideoSourceSystemWeakPtr m_videoSourceSystem;
};