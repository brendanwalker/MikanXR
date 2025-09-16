#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_ComponentList.h"

class RmlModel_StencilComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_StencilComponent();

	virtual bool init(Rml::Context* rmlContext) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	StencilObjectSystemPtr getStencilObjectSystem() const;
	StencilObjectSystemConfigPtr getStencilObjectSystemConfig() const;

private:
	RmlDataBinding_ComponentListPtr m_stencilComponentIdList;
	StencilObjectSystemWeakPtr m_stencilObjectSystem;
};

using RmlModel_StencilComponentPtr = std::shared_ptr<RmlModel_StencilComponent>;