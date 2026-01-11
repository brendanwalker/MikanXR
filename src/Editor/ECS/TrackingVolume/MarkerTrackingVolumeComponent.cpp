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
void MarkerTrackingVolumeComponent::getPropertyDescriptors(
	std::vector<PropertyDescriptorConstPtr>& outDescriptors)
{
	TrackingVolumeComponent::getPropertyDescriptors(outDescriptors);
}

void MarkerTrackingVolumeComponent::getFunctionDescriptors(
	std::vector<FunctionDescriptorConstPtr>& outDescriptors)
{
	TrackingVolumeComponent::getFunctionDescriptors(outDescriptors);
}
