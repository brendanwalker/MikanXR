#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "VRTrackingVolumeComponent.h"

class RmlModel_VRTrackingVolumeComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_VRTrackingVolumeComponent();

	virtual bool init(Rml::Context* rmlContext) override;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	MarkerObjectSystemPtr getMarkerObjectSystem() const;
	MarkerObjectSystemConfigPtr getMarkerObjectSystemConfig() const;
	TrackingMountObjectSystemPtr getTrackingMountObjectSystem() const;
	TrackingMountObjectSystemConfigPtr getTrackingMountObjectSystemConfig() const;
	VRTrackingVolumeComponentPtr getVRTrackingVolumeComponent() const;

private:
	RmlDataBinding_ComponentIdListPtr m_markerComponentIdList;
	RmlDataBinding_ComponentIdListPtr m_trackingMountIdList;
	MarkerObjectSystemWeakPtr m_markerObjectSystem;
	TrackingMountObjectSystemWeakPtr m_trackingMountObjectSystem;
};