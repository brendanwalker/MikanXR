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

	inline static const std::string k_componentClassName = "MarkerTrackingVolumeComponent";
	virtual std::string getComponentClassName() const override { return k_componentClassName; }

	inline MarkerTrackingVolumeDefinitionPtr getMarkerTrackingVolumeDefinition() const
	{ return std::static_pointer_cast<MarkerTrackingVolumeDefinition>(m_definition); }

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<PropertyDescriptorConstPtr>& outDescriptors);

	// -- IRmlFunctionInterface ----
	static void getRmlFunctionDescriptors(std::vector<FunctionDescriptorConstPtr>& outDescriptors);
};