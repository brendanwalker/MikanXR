#include "MarkerTrackingVolumeComponent.h"

// -- MarkerTrackingVolumeDefinition -----
MarkerTrackingVolumeDefinition::MarkerTrackingVolumeDefinition()
	: TrackingVolumeDefinition()
{
}

MarkerTrackingVolumeDefinition::MarkerTrackingVolumeDefinition(
	MikanTrackingSystemID trackingSystemId,
	const std::string& trackingSystemName)
	: TrackingVolumeDefinition(trackingSystemId, trackingSystemName)
{
}

eTrackingSystemType MarkerTrackingVolumeDefinition::getTrackingSystemType() const
{
	return eTrackingSystemType::marker;
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