#pragma once

#include "TrackingVolumeComponent.h"
#include "MarkerTrackingVolumeComponent.h"

class MarkerTrackingVolumeDefinition :
	public TrackingVolumeDefinition
{
public:
	MarkerTrackingVolumeDefinition();
	MarkerTrackingVolumeDefinition(
		MikanTrackingVolumeID trackingVolumeId,
		const std::string& trackingVolumeName);

	virtual eTrackingVolumeType getTrackingVolumeType() const override;
};

class MarkerTrackingVolumeComponent : public TrackingVolumeComponent
{
public:
	MarkerTrackingVolumeComponent(MikanObjectWeakPtr owner);

	inline MarkerTrackingVolumeDefinitionPtr getMarkerTrackingVolumeDefinition() const
	{ return std::static_pointer_cast<MarkerTrackingVolumeDefinition>(m_definition); }

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors);

	// -- IRmlFunctionInterface ----
	static void getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors);
};