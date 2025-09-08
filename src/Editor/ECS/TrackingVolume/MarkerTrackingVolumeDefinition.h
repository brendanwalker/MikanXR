#pragma once

#include "TrackingVolumeDefinition.h"

class MarkerTrackingVolumeDefinition : 
	public TrackingVolumeDefinition
{
public:
	MarkerTrackingVolumeDefinition();
	MarkerTrackingVolumeDefinition(
		MikanTrackingSystemID trackingSystemId,
		const std::string& trackingSystemName);

	virtual eTrackingSystemType getTrackingSystemType() const override;
};