#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_ComponentList.h"

class RmlModel_CompositorComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_CompositorComponent();

	virtual bool init(Rml::Context* rmlContext) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	CameraObjectSystemPtr getCameraObjectSystem() const;
	CameraObjectSystemConfigPtr getCameraObjectSystemConfig() const;
	VideoSourceSystemPtr getVideoSourceSystem() const;
	VideoSourceSystemConfigPtr getVideoSourceSystemConfig() const;

private:
	RmlDataBinding_ComponentListPtr m_cameraIdList;
	RmlDataBinding_ComponentListPtr m_videoSourceIdList;
	CameraObjectSystemWeakPtr m_cameraObjectSystem;
	VideoSourceSystemWeakPtr m_videoSourceSystem;
};

using RmlModel_CompositorComponentPtr = std::shared_ptr<RmlModel_CompositorComponent>;