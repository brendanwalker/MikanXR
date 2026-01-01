#pragma once

#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "ObjectFwd.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "Shared/RmlModel_PropertyInterface.h"
#include "SinglecastDelegate.h"

class RmlModel_ProjectMarkers : public RmlModel
{
public:
	RmlModel_ProjectMarkers();

	bool init(class ProjectRmlModelContext* context);
	virtual void dispose() override;

private:
	MarkerObjectSystemPtr getMarkerSystem();
	MarkerComponentPtr getSelectedMarker();

	void addNewMarker(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeMarker(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void selectMarkerEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);

	void setSelectedMarkerId(MikanMarkerID markerId);

	void markerIdListChanged(bool bOwnerChanged);

	class ProjectRmlModelContext* m_projectRmlModelContext = nullptr;
	MarkerObjectSystemWeakPtr m_markerSystem;

	RmlDataBinding_ComponentIdListPtr m_markerIdList;

	int m_selectedMarkerId = -1; // MikanMarkerID
};