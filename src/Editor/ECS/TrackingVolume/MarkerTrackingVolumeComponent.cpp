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

// -- IRmlPropertyInterface ----
void MarkerTrackingVolumeComponent::getRmlPropertyDescriptors(
	std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors)
{
	TrackingVolumeComponent::getRmlPropertyDescriptors(outDescriptors);
}

void MarkerTrackingVolumeComponent::getRmlFunctionDescriptors(
	std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors)
{
	TrackingVolumeComponent::getRmlFunctionDescriptors(outDescriptors);
}
