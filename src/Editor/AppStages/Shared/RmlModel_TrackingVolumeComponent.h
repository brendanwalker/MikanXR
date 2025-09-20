#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_ComponentList.h"

class RmlModel_TrackingVolumeComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_TrackingVolumeComponent();

	virtual bool init(Rml::Context* rmlContext) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	MarkerObjectSystemPtr getMarkerObjectSystem() const;
	MarkerObjectSystemConfigPtr getMarkerObjectSystemConfig() const;
	TrackingMountObjectSystemPtr getTrackingMountObjectSystem() const;
	TrackingMountObjectSystemConfigPtr getTrackingMountObjectSystemConfig() const;

private:
	RmlDataBinding_ComponentListPtr m_markerComponentIdList;
	RmlDataBinding_ComponentListPtr m_trackingMountIdList;
	MarkerObjectSystemWeakPtr m_markerObjectSystem;
	TrackingMountObjectSystemWeakPtr m_trackingMountObjectSystem;
};

using RmlModel_TrackingVolumeComponentPtr = std::shared_ptr<RmlModel_TrackingVolumeComponent>;