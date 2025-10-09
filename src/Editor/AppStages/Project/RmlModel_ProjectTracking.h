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

class RmlModel_ProjectTracking : public RmlModel
{
public:
	RmlModel_ProjectTracking();

	bool init(
		Rml::Context* rmlContext, 
		ProjectConfigPtr projectConfig,
		TrackingVolumeObjectSystemPtr trackingVolumeSystem,
		TrackingMountObjectSystemPtr trackingMountSystem);
	virtual void dispose() override;

private:
	TrackingVolumeObjectSystemPtr getTrackingVolumeSystem();
	TrackingMountObjectSystemPtr getTrackingMountSystem();
	TrackingVolumeComponentPtr getSelectedTrackingVolume();
	MarkerTrackingVolumeComponentPtr getSelectedMarkerTrackingVolume();
	VRTrackingVolumeComponentPtr getSelectedVRTrackingVolume();
	TrackingMountComponentPtr getSelectedTrackingMount();

	void addNewSteamVRTrackingVolume(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewMarkerTrackingVolume(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeTrackingVolume(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void addNewTrackingMount(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void removeTrackingMountID(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void selectTrackingVolumeEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);
	void selectTrackingMountEntry(Rml::DataModelHandle handle, Rml::Event& /*ev*/, const Rml::VariantList& parameters);

	void setSelectedTrackingVolumeId(MikanTrackingVolumeID trackingVolumeId);
	void setSelectedTrackingMountId(MikanTrackingMountID trackingMountId);

	void trackingVolumeIdListChanged(bool bOwnerChanged);
	void trackingMountIdListChanged(bool bOwnerChanged);

	ProjectConfigWeakPtr m_projectConfig;
	TrackingVolumeObjectSystemWeakPtr m_trackingVolumeSystem;
	TrackingMountObjectSystemWeakPtr m_trackingMountSystem;

	RmlDataBinding_ComponentIdListPtr m_trackingVolumeIdList;
	RmlDataBinding_ComponentIdListPtr m_trackingMountIdList;
	RmlModel_VRTrackingVolumeComponentPtr m_selectedVRTrackingVolumeModel;
	RmlModel_MarkerTrackingVolumeComponentPtr m_selectedMarkerTrackingVolumeModel;
	RmlModel_TrackingMountComponentPtr m_selectedTrackingMountModel;

	int m_selectedTrackingVolumeId = -1; // MikanTrackingVolumeID
	int m_selectedTrackingMountId = -1; // MikanTrackingMountID
};