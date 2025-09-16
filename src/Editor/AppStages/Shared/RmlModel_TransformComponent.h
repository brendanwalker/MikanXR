#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_ComponentList.h"

class RmlModel_TransformComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_TransformComponent();

	virtual bool init(Rml::Context* rmlContext) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	AnchorObjectSystemPtr getAnchorObjectSystem() const;
	AnchorObjectSystemConfigPtr getAnchorObjectSystemConfig() const;
	StencilObjectSystemPtr getStencilObjectSystem() const;
	StencilObjectSystemConfigPtr getStencilObjectSystemConfig() const;

private:
	RmlDataBinding_ComponentListPtr m_transformComponentIdList;
	AnchorObjectSystemWeakPtr m_anchorObjectSystem;
	StencilObjectSystemWeakPtr m_stencilObjectSystem;
};

using RmlModel_TransformComponentPtr = std::shared_ptr<RmlModel_TransformComponent>;