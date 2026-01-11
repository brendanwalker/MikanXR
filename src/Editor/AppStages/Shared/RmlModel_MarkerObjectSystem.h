#pragma once

#include "Shared/RmlModel_MikanObjectSystem.h"
#include "Shared/RmlDataBinding_Fwd.h"

class RmlModel_MarkerObjectSystem : public RmlModel_MikanObjectSystem
{
public:
	RmlModel_MarkerObjectSystem();

	virtual bool init(Rml::Context* rmlContext) override;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor);

protected:
	MarkerObjectSystemPtr getMarkerObjectSystem() const;
	MarkerObjectSystemDefinitionPtr getMarkerObjectSystemDefinition() const;
};
