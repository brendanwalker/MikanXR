#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "CompositorComponent.h"

class RmlModel_CompositorComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_CompositorComponent();

	virtual bool init(class AppStage* ownerAppStage) override;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	CameraObjectSystemPtr getCameraObjectSystem() const;
	CameraObjectSystemDefinitionPtr getCameraObjectSystemConfig() const;
	CompositorComponentPtr getCompositorComponent() const;

private:
	RmlDataBinding_ComponentIdListPtr m_cameraIdList;
	RmlDataBinding_ComponentIdListPtr m_textureSourceIdList;
	CameraObjectSystemWeakPtr m_cameraObjectSystem;
};

using RmlModel_CompositorComponentPtr = std::shared_ptr<RmlModel_CompositorComponent>;