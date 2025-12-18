#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "AnchorComponent.h"

class RmlModel_AnchorComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_AnchorComponent();

	virtual bool init(Rml::Context* rmlContext) override;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	AnchorObjectSystemPtr getAnchorObjectSystem() const;
	AnchorObjectSystemConfigPtr getAnchorObjectSystemConfig() const;

private:
	RmlDataBinding_ComponentIdListPtr m_stencilComponentIdList;
	AnchorObjectSystemWeakPtr m_anchorObjectSystem;
};