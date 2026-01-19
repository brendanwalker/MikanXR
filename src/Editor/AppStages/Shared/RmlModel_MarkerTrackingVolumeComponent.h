#pragma once

#include "Shared/RmlModel_MikanComponent.h"
#include "Shared/RmlDataBinding_Fwd.h"
#include "MarkerTrackingVolumeComponent.h"

class RmlModel_MarkerTrackingVolumeComponent : public RmlModel_MikanComponent
{
public:
	RmlModel_MarkerTrackingVolumeComponent();

	virtual bool init(class AppStage* ownerAppStage) override;
	virtual bool onConstruct(Rml::DataModelConstructor& constructor) override;

protected:
	MarkerObjectSystemPtr getMarkerObjectSystem() const;
	MarkerObjectSystemDefinitionPtr getMarkerObjectSystemDefinition() const;
	TrackingMountObjectSystemPtr getTrackingMountObjectSystem() const;
	TrackingMountObjectSystemDefinitionPtr getTrackingMountObjectSystemConfig() const;
	MarkerTrackingVolumeComponentPtr getMarkerTrackingVolumeComponent() const;

private:
	RmlDataBinding_ComponentIdListPtr m_markerComponentIdList;
	MarkerObjectSystemWeakPtr m_markerObjectSystem;
	TrackingMountObjectSystemWeakPtr m_trackingMountObjectSystem;
};