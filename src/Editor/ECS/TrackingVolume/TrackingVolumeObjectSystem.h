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

using TrackingVolumeMap = std::map<MikanTrackingVolumeID, TrackingVolumeComponentWeakPtr>;

class TrackingVolumeObjectSystemConfig : public CommonConfig
{
public:
	TrackingVolumeObjectSystemConfig(const std::string& configName)
		: CommonConfig(configName)
	{}

	static TrackingVolumeObjectSystemConfigPtr getSystemConfig();

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	bool canAddTrackingVolumeType(eTrackingVolumeType systemType) const;
	TrackingVolumeDefinitionConstPtr getTrackingVolumeDefinitionConst(MikanTrackingVolumeID systemId) const;
	TrackingVolumeDefinitionPtr getTrackingVolumeDefinition(MikanTrackingVolumeID systemId);
	eTrackingVolumeType getTrackingVolumeType(MikanTrackingVolumeID systemId) const;
	bool removeTrackingVolume(MikanTrackingVolumeID systemId);

	static const std::string k_markerTrackingVolumeListPropertyId;
	const std::vector<MarkerTrackingVolumeDefinitionPtr>& getMarkerTrackingVolumeList() const { return m_markerTrackingVolumeList; }
	MarkerTrackingVolumeDefinitionConstPtr getMarkerTrackingVolumeDefinitionConst(MikanTrackingVolumeID systemId) const;
	MarkerTrackingVolumeDefinitionPtr getMarkerTrackingVolumeDefinition(MikanTrackingVolumeID systemId);
	MikanTrackingVolumeID addMarkerTrakingSystem();
	bool removeMarkerTrackingVolume(MikanTrackingVolumeID systemId);

	static const std::string k_vrTrackingVolumeListPropertyId;
	const std::vector<VRTrackingVolumeDefinitionPtr>& getVRTrackingVolumeList() const { return m_vrTrackingVolumeList; }
	VRTrackingVolumeDefinitionConstPtr getVRTrackingVolumeDefinitionConst(MikanTrackingVolumeID systemId) const;
	VRTrackingVolumeDefinitionPtr getVRTrackingVolumeDefinition(MikanTrackingVolumeID systemId);
	MikanTrackingVolumeID addVRTrackingVolume(eTrackingRuntime trackingRuntime);
	bool removeVRTrackingVolume(MikanTrackingVolumeID systemId);

protected:
	std::vector<MarkerTrackingVolumeDefinitionPtr> m_markerTrackingVolumeList;
	std::vector<VRTrackingVolumeDefinitionPtr> m_vrTrackingVolumeList;
	MikanTrackingVolumeID m_nextTrackingVolumeId = 0;
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
	TrackingVolumeComponentPtr getTrackingVolumeById(MikanTrackingVolumeID trackingVolumeId) const;
	MarkerTrackingVolumeComponentPtr getMarkerTrackingVolumeById(MikanTrackingVolumeID trackingVolumeId) const;
	VRTrackingVolumeComponentPtr getVRTrackingVolumeById(MikanTrackingVolumeID trackingVolumeId) const;
	
	MarkerTrackingVolumeComponentPtr addNewMarkerTrackingVolume();
	VRTrackingVolumeComponentPtr addNewVRTrackingVolume(eTrackingRuntime trackingRuntime);
	bool removeTrackingVolume(MikanTrackingVolumeID trackingVolumeId);

protected:
	TrackingVolumeComponentPtr createTrackingVolumeObject(TrackingVolumeDefinitionPtr trackingVolumeConfig);
	void disposeTrackingVolumeObject(MikanTrackingVolumeID trackingVolumeId);

	TrackingVolumeMap m_trackingVolumeComponents;
};
