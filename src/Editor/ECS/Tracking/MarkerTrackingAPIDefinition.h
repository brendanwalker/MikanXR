#pragma once

#include "TrackingAPIDefinition.h"

class MarkerTrackingAPIDefinition : 
	public TrackingAPIDefinition
{
public:
	MarkerTrackingAPIDefinition();
	MarkerTrackingAPIDefinition(
		MikanTrackingSystemID trackingSystemId,
		const std::string& trackingSystemName);

	virtual eTrackingSystemType getTrackingSystemType() const override;
};