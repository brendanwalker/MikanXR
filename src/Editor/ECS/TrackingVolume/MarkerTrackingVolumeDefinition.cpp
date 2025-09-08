#include "MarkerTrackingVolumeDefinition.h"


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