#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "CompositorComponent.h"

class RmlModel_CompositorComponent : public RmlModel_TypedMikanComponent<CompositorComponent>
{
public:
	RmlModel_CompositorComponent();

	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	CameraObjectSystemPtr getCameraObjectSystem() const;
	CameraObjectSystemConfigPtr getCameraObjectSystemConfig() const;
	CompositorComponentPtr getCompositorComponent() const;

private:
	RmlDataBinding_ComponentIdListPtr m_cameraIdList;
	RmlDataBinding_ComponentIdListPtr m_textureSourceIdList;
	CameraObjectSystemWeakPtr m_cameraObjectSystem;
};

using RmlModel_CompositorComponentPtr = std::shared_ptr<RmlModel_CompositorComponent>;