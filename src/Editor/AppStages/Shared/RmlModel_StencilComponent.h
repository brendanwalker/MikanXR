#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "StencilComponent.h"

class RmlModel_StencilComponent : public RmlModel_TypedMikanComponent<StencilComponent>
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