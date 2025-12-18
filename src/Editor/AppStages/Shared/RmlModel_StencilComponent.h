#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "StencilComponent.h"

// Abstract base class for Rml data models for stencil components
class RmlModel_StencilComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_StencilComponent();

	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	AnchorObjectSystemPtr getAnchorObjectSystem() const;
	AnchorObjectSystemConfigPtr getAnchorObjectSystemConfig() const;
	StencilComponentPtr getStencilComponent() const;

private:
	RmlDataBinding_ComponentIdListPtr m_anchorComponentIdList;
	AnchorObjectSystemWeakPtr m_stencilObjectSystem;
};

class RmlModel_QuadStencilComponent : public RmlModel_StencilComponent
{
public:
	RmlModel_QuadStencilComponent()= default;

	virtual bool init(Rml::Context* rmlContext) override;
};

class RmlModel_BoxStencilComponent : public RmlModel_StencilComponent
{
public:
	RmlModel_BoxStencilComponent() = default;

	virtual bool init(Rml::Context* rmlContext) override;
};

class RmlModel_ModelStencilComponent : public RmlModel_StencilComponent
{
public:
	RmlModel_ModelStencilComponent() = default;

	virtual bool init(Rml::Context* rmlContext) override;
};