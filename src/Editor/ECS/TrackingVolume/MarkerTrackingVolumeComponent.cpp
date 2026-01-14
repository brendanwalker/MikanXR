#include "MarkerTrackingVolumeComponent.h"

// -- MarkerTrackingVolumeDefinition -----
MarkerTrackingVolumeDefinition::MarkerTrackingVolumeDefinition()
	: TrackingVolumeDefinition()
{
}

MarkerTrackingVolumeDefinition::MarkerTrackingVolumeDefinition(
	MikanTrackingVolumeID trackingVolumeId)
	: TrackingVolumeDefinition(trackingVolumeId)
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
