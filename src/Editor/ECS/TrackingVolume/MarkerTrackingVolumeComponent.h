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

	// -- IPropertyInterface ----
	static void getPropertyNamesStatic(std::vector<std::string>& outPropertyNames);
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;

	// -- IFunctionInterface ----
	static void getFunctionNamesStatic(std::vector<std::string>& outPropertyNames)
	{ TrackingVolumeComponent::getFunctionNamesStatic(outPropertyNames); }
};