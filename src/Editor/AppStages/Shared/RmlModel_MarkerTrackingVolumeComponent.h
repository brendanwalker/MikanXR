#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "MarkerTrackingVolumeComponent.h"

class RmlModel_MarkerTrackingVolumeComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_MarkerTrackingVolumeComponent();

	virtual bool init(Rml::Context* rmlContext) override;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;
	virtual bool setComponent(MikanComponentPtr component) override;

protected:
	MarkerObjectSystemPtr getMarkerObjectSystem() const;
	MarkerObjectSystemConfigPtr getMarkerObjectSystemConfig() const;
	TrackingMountObjectSystemPtr getTrackingMountObjectSystem() const;
	TrackingMountObjectSystemConfigPtr getTrackingMountObjectSystemConfig() const;
	MarkerTrackingVolumeComponentPtr getMarkerTrackingVolumeComponent() const;

private:
	RmlDataBinding_ComponentIdListPtr m_markerComponentIdList;
	MarkerObjectSystemWeakPtr m_markerObjectSystem;
	TrackingMountObjectSystemWeakPtr m_trackingMountObjectSystem;
};