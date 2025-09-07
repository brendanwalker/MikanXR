#include "MarkerTrackingAPIDefinition.h"


MarkerTrackingAPIDefinition::MarkerTrackingAPIDefinition()
	: TrackingAPIDefinition() 
{
}

MarkerTrackingAPIDefinition::MarkerTrackingAPIDefinition(
	MikanTrackingSystemID trackingSystemId,
	const std::string& trackingSystemName)
	: TrackingAPIDefinition(trackingSystemId, trackingSystemName) 
{
}

eTrackingSystemType MarkerTrackingAPIDefinition::getTrackingSystemType() const 
{
	return eTrackingSystemType::marker; 
}