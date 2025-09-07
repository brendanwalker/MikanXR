#pragma once

#include "CommonConfig.h"
#include "MarkerTrackingAPIDefinition.h"
#include "VRTrackingAPIDefinition.h"
#include <vector>

class TrackingSystemsConfig : public CommonConfig
{
public:
	TrackingSystemsConfig(const std::string& configName)
		: CommonConfig(configName)
	{}

	static TrackingSystemsConfigPtr getSystemConfig();

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	bool canAddTrackingSystemType(eTrackingSystemType systemType) const;
	TrackingAPIDefinitionConstPtr getTrackingAPIDefinitionConst(MikanTrackingSystemID systemId) const;
	TrackingAPIDefinitionPtr getTrackingAPIDefinition(MikanTrackingSystemID systemId);
	eTrackingSystemType getTrackingSystemType(MikanTrackingSystemID systemId) const;
	bool removeTrackingSystem(MikanTrackingSystemID systemId);

	static const std::string k_markerTrackingSystemListPropertyId;
	const std::vector<MarkerTrackingAPIDefinitionPtr>& getMarkerTrackingAPIList() const { return m_markerTrackingAPIList; }
	MarkerTrackingAPIDefinitionConstPtr getMarkerTrackingAPIDefinitionConst(MikanTrackingSystemID systemId) const;
	MarkerTrackingAPIDefinitionPtr getMarkerTrackingAPIDefinition(MikanTrackingSystemID systemId);
	MikanTrackingSystemID addMarkerTrakingSystem();
	bool removeMarkerTrackingSystem(MikanTrackingSystemID systemId);

	static const std::string k_vrTrackingSystemListPropertyId;
	const std::vector<VRTrackingAPIDefinitionPtr>& getVRTrackingAPIList() const { return m_vrTrackingAPIList; }
	VRTrackingAPIDefinitionConstPtr getVRTrackingAPIDefinitionConst(MikanTrackingSystemID systemId) const;
	VRTrackingAPIDefinitionPtr getVRTrackingAPIDefinition(MikanTrackingSystemID systemId);
	MikanTrackingSystemID addVRTrackingSystem(eTrackingRuntime trackingRuntime);
	bool removeVRTrackingSystem(MikanTrackingSystemID systemId);

protected:
	std::vector<MarkerTrackingAPIDefinitionPtr> m_markerTrackingAPIList;
	std::vector<VRTrackingAPIDefinitionPtr> m_vrTrackingAPIList;
	MikanTrackingSystemID m_nextTrackingSystemId = 0;
};
