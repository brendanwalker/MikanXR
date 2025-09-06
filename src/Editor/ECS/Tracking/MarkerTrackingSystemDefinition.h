#pragma once

#include "TrackingSystemDefinition.h"

class MarkerTrackingSystemDefinition : 
	public TrackingSystemDefinition
{
public:
	MarkerTrackingSystemDefinition();
	MarkerTrackingSystemDefinition(
		MikanTrackingSystemID trackingSystemId,
		const std::string& trackingSystemName);

	virtual eTrackingSystemType getTrackingSystemType() const override;
};
