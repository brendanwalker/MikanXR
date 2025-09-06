#pragma once

#include "CommonConfig.h"
#include "MarkerTrackingSystemDefinition.h"
#include "VRTrackingSystemDefinition.h"
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
	TrackingSystemDefinitionConstPtr getTrackingSystemDefinitionConst(MikanTrackingSystemID systemId) const;
	TrackingSystemDefinitionPtr getTrackingSystemDefinition(MikanTrackingSystemID systemId);
	eTrackingSystemType getTrackingSystemType(MikanTrackingSystemID systemId) const;
	bool removeTrackingSystem(MikanTrackingSystemID systemId);

	static const std::string k_markerTrackingSystemListPropertyId;
	const std::vector<MarkerTrackingSystemDefinitionPtr>& getMarkerTrackingSystemList() const { return m_markerTrackingSystemList; }
	MarkerTrackingSystemDefinitionConstPtr getMarkerTrackingSystemDefinitionConst(MikanTrackingSystemID systemId) const;
	MarkerTrackingSystemDefinitionPtr getMarkerTrackingSystemDefinition(MikanTrackingSystemID systemId);
	MikanTrackingSystemID addMarkerTrakingSystem();
	bool removeMarkerTrackingSystem(MikanTrackingSystemID systemId);

	static const std::string k_vrTrackingSystemListPropertyId;
	const std::vector<VRTrackingSystemDefinitionPtr>& getVRTrackingSystemList() const { return m_vrTrackingSystemList; }
	VRTrackingSystemDefinitionConstPtr getVRTrackingSystemDefinitionConst(MikanTrackingSystemID systemId) const;
	VRTrackingSystemDefinitionPtr getVRTrackingSystemDefinition(MikanTrackingSystemID systemId);
	MikanTrackingSystemID addVRTrackingSystem(eTrackingRuntime trackingRuntime);
	bool removeVRTrackingSystem(MikanTrackingSystemID systemId);

protected:
	std::vector<MarkerTrackingSystemDefinitionPtr> m_markerTrackingSystemList;
	std::vector<VRTrackingSystemDefinitionPtr> m_vrTrackingSystemList;
	MikanTrackingSystemID m_nextTrackingSystemId = 0;
};
