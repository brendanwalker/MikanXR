#pragma once

#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "ObjectFwd.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel.h"
#include "Shared/RmlDataBinding_ComponentList.h"
#include "Shared/RmlModel_PropertyInterface.h"
#include "SinglecastDelegate.h"

class RmlModel_ProjectTracking : public RmlModel
{
public:
	RmlModel_ProjectTracking();

	bool init(
		Rml::Context* rmlContext, 
		ProjectConfigPtr projectConfig,
		TrackingMountObjectSystemPtr trackingMountSystem);
	virtual void dispose() override;

private:
	TrackingVolumeObjectSystemConfigPtr getTrackingSystemsConfig();
	TrackingVolumeDefinitionPtr getSelectedTrackingSystem();
	MarkerTrackingVolumeDefinitionPtr getSelectedMarkerTrackingSystem();
	VRTrackingVolumeDefinitionPtr getSelectedVRTrackingSystem();
	TrackingMountComponentPtr getSelectedTrackingMount();

	void addNewSteamVRTrackingSystem(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewMarkerTrackingSystem(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeTrackingSystem(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewTrackingMount(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeTrackingMount(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void selectTrackingSystemEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void selectTrackingMountEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);

	void setSelectedTrackingSystemId(MikanTrackingSystemID trackingSystemId);
	void setSelectedTrackingMountId(MikanTrackingMountID trackingMountId);

	void trackingSystemIdListChanged(bool bOwnerChanged);
	void trackingMountIdListChanged(bool bOwnerChanged);

	ProjectConfigWeakPtr m_projectConfig;
	TrackingMountObjectSystemWeakPtr m_trackingMountSystem;

	RmlDataBinding_ComponentListPtr m_trackingSystemIdList;
	RmlDataBinding_ComponentListPtr m_trackingMountIdList;
	RmlModel_PropertyInterfacePtr m_selectedVRTrackingSystemModel;
	RmlModel_PropertyInterfacePtr m_selectedMarkerTrackingSystemModel;
	RmlModel_PropertyInterfacePtr m_selectedTrackingMountModel;

	int m_selectedTrackingSystemId = -1; // MikanTrackingSystemID
	int m_selectedTrackingMountId = -1; // MikanTrackingMountID
};