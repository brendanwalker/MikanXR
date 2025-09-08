#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "TrackingVolumeComponent.h"
#include "MarkerTrackingVolumeComponent.h"
#include "VRTrackingVolumeComponent.h"
#include "MikanObjectSystem.h"
#include "MikanTypeFwd.h"
#include "MulticastDelegate.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "ProjectConfigConstants.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

using TrackingVolumeMap = std::map<MikanTrackingSystemID, TrackingVolumeComponentWeakPtr>;

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

class TrackingVolumeObjectSystem : public MikanObjectSystem
{
public:
	TrackingVolumeObjectSystem(class ObjectSystemManager* ownerObjectSystemManager) : MikanObjectSystem(ownerObjectSystemManager) {}

	virtual bool init() override;
	virtual void dispose() override;
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) override;

	TrackingVolumeObjectSystemConfigConstPtr getTrackingVolumeSystemConfigConst() const;
	TrackingVolumeObjectSystemConfigPtr getTrackingVolumeSystemConfig();

	const TrackingVolumeMap& getTrackingVolumeMap() const { return m_trackingVolumeComponents; }
	TrackingVolumeComponentPtr getTrackingVolumeById(MikanTrackingSystemID trackingSystemId) const;
	MarkerTrackingVolumeComponentPtr getMarkerTrackingVolumeById(MikanTrackingSystemID trackingSystemId) const;
	VRTrackingVolumeComponentPtr getVRTrackingVolumeById(MikanTrackingSystemID trackingSystemId) const;
	
	MarkerTrackingVolumeComponentPtr addNewMarkerTrackingVolume();
	VRTrackingVolumeComponentPtr addNewVRTrackingVolume(eTrackingRuntime trackingRuntime);
	bool removeTrackingSystem(MikanTrackingSystemID trackingSystemId);

protected:
	TrackingVolumeComponentPtr createTrackingVolumeObject(TrackingVolumeDefinitionPtr trackingVolumeConfig);
	void disposeTrackingVolumeObject(MikanTrackingSystemID trackingSystemId);

	TrackingVolumeMap m_trackingVolumeComponents;
};
