#pragma once

#include "TrackingVolumeComponent.h"
#include "MarkerTrackingVolumeComponent.h"

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

class MarkerTrackingVolumeComponent : public TrackingVolumeComponent
{
public:
	MarkerTrackingVolumeComponent(MikanObjectWeakPtr owner);

	inline MarkerTrackingVolumeDefinitionPtr getMarkerTrackingVolumeDefinition() const
	{ return std::static_pointer_cast<MarkerTrackingVolumeDefinition>(m_definition); }

	// -- IPropertyInterface ----
	static void getPropertyNamesStatic(std::vector<std::string>& outPropertyNames);
	virtual void getPropertyNames(std::vector<std::string>& outPropertyNames) const override;
};