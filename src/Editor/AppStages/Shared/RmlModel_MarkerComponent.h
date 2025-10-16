#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"

class RmlModel_MarkerComponent : public RmlModel_TypedMikanComponent<MarkerComponent>
{
public:
	RmlModel_MarkerComponent();

	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	MarkerObjectSystemPtr getMarkerObjectSystem() const;
	MarkerObjectSystemConfigPtr getMarkerObjectSystemConfig() const;
	MarkerComponentPtr getMarkerComponent() const;

private:
	RmlDataBinding_ArucoIdListPtr m_arucoIdList;
	MarkerObjectSystemWeakPtr m_markerObjectSystem;
};