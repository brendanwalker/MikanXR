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
	std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	TrackingVolumeComponent::getRmlPropertyDescriptors(outDescriptors);
}

void MarkerTrackingVolumeComponent::getRmlFunctionDescriptors(
	std::vector<FunctionDescriptorConstPtr>& outDescriptors)
{
	TrackingVolumeComponent::getRmlFunctionDescriptors(outDescriptors);
}
