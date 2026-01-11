#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "SinglecastDelegate.h"

class RmlModel_MarkerComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_MarkerComponent();

	virtual bool init(Rml::Context* rmlContext) override;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;
	virtual bool setComponent(MikanComponentPtr component) override;

	SinglecastDelegate<void(int arucoId)> OnMarkerSelected;

protected:
	MarkerObjectSystemPtr getMarkerObjectSystem() const;
	MarkerObjectSystemDefinitionPtr getMarkerObjectSystemDefinition() const;
	MarkerComponentPtr getMarkerComponent() const;

private:
	RmlDataBinding_ArucoIdListPtr m_arucoIdList;
	MarkerObjectSystemWeakPtr m_markerObjectSystem;
};