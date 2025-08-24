#include "MarkerTrackingSystemDefinition.h"


MarkerTrackingSystemDefinition::MarkerTrackingSystemDefinition()
	: TrackingSystemDefinition() 
{
}

MarkerTrackingSystemDefinition::MarkerTrackingSystemDefinition(
	MikanTrackingSystemID trackingSystemId,
	const std::string& trackingSystemName)
	: TrackingSystemDefinition(trackingSystemId, trackingSystemName) 
{
}

eTrackingSystemType MarkerTrackingSystemDefinition::getTrackingSystemType() const 
{
	return eTrackingSystemType::marker; 
}