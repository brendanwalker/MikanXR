#include "MarkerTrackingVolumeComponent.h"

// -- MarkerTrackingVolumeDefinition -----
MarkerTrackingVolumeDefinition::MarkerTrackingVolumeDefinition()
	: TrackingVolumeDefinition()
{
}

MarkerTrackingVolumeDefinition::MarkerTrackingVolumeDefinition(
	MikanTrackingVolumeID trackingVolumeId,
	const std::string& trackingVolumeName)
	: TrackingVolumeDefinition(trackingVolumeId, trackingVolumeName)
{
}

eTrackingVolumeType MarkerTrackingVolumeDefinition::getTrackingVolumeType() const
{
	return eTrackingVolumeType::marker;
}

// -- MarkerTrackingVolumeComponent -----
MarkerTrackingVolumeComponent::MarkerTrackingVolumeComponent(MikanObjectWeakPtr owner)
	: TrackingVolumeComponent(owner)
{
}

// -- IPropertyInterface ----
void MarkerTrackingVolumeComponent::getPropertyNamesStatic(std::vector<std::string>& outPropertyNames)
{
	TrackingVolumeComponent::getPropertyNamesStatic(outPropertyNames);
}

void MarkerTrackingVolumeComponent::getPropertyNames(std::vector<std::string>& outPropertyNames) const
{
	getPropertyNamesStatic(outPropertyNames);
}