#pragma once

#include "CommonConfig.h"
#include "MarkerTrackingVolumeDefinition.h"
#include "VRTrackingVolumeDefinition.h"
#include <vector>

class TrackingVolumeObjectSystemConfig : public CommonConfig
{
public:
	TrackingVolumeObjectSystemConfig(const std::string& configName)
		: CommonConfig(configName)
	{}

	static TrackingVolumeObjectSystemConfigPtr getSystemConfig();

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	bool canAddTrackingSystemType(eTrackingSystemType systemType) const;
	TrackingVolumeDefinitionConstPtr getTrackingVolumeDefinitionConst(MikanTrackingSystemID systemId) const;
	TrackingVolumeDefinitionPtr getTrackingVolumeDefinition(MikanTrackingSystemID systemId);
	eTrackingSystemType getTrackingSystemType(MikanTrackingSystemID systemId) const;
	bool removeTrackingSystem(MikanTrackingSystemID systemId);

	static const std::string k_markerTrackingSystemListPropertyId;
	const std::vector<MarkerTrackingVolumeDefinitionPtr>& getMarkerTrackingVolumeList() const { return m_markerTrackingVolumeList; }
	MarkerTrackingVolumeDefinitionConstPtr getMarkerTrackingVolumeDefinitionConst(MikanTrackingSystemID systemId) const;
	MarkerTrackingVolumeDefinitionPtr getMarkerTrackingVolumeDefinition(MikanTrackingSystemID systemId);
	MikanTrackingSystemID addMarkerTrakingSystem();
	bool removeMarkerTrackingSystem(MikanTrackingSystemID systemId);

	static const std::string k_vrTrackingSystemListPropertyId;
	const std::vector<VRTrackingVolumeDefinitionPtr>& getVRTrackingVolumeList() const { return m_vrTrackingVolumeList; }
	VRTrackingVolumeDefinitionConstPtr getVRTrackingVolumeDefinitionConst(MikanTrackingSystemID systemId) const;
	VRTrackingVolumeDefinitionPtr getVRTrackingVolumeDefinition(MikanTrackingSystemID systemId);
	MikanTrackingSystemID addVRTrackingSystem(eTrackingRuntime trackingRuntime);
	bool removeVRTrackingSystem(MikanTrackingSystemID systemId);

protected:
	std::vector<MarkerTrackingVolumeDefinitionPtr> m_markerTrackingVolumeList;
	std::vector<VRTrackingVolumeDefinitionPtr> m_vrTrackingVolumeList;
	MikanTrackingSystemID m_nextTrackingSystemId = 0;
};
