#pragma once

#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "ObjectFwd.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "Shared/RmlModel.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "Shared/RmlModel_EntityAccessor.h"
#include "SinglecastDelegate.h"

class RmlModel_ProjectTracking : public RmlModel
{
public:
	RmlModel_ProjectTracking();

	bool init(class ProjectRmlModelContext* context);
	virtual void dispose() override;

private:
	TrackingMountObjectSystemPtr getTrackingMountSystem();
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

	class ProjectRmlModelContext* m_projectRmlModelContext = nullptr;
	TrackingMountObjectSystemWeakPtr m_trackingMountSystem;

	RmlDataBinding_ComponentIdListPtr m_trackingVolumeIdList;
	RmlDataBinding_ComponentIdListPtr m_trackingMountIdList;

	bool m_isVRTrackingVolume = false;
	int m_selectedTrackingVolumeId = -1; // MikanTrackingVolumeID
	int m_selectedTrackingMountId = -1; // MikanTrackingMountID
};