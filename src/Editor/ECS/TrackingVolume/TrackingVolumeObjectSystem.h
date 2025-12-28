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
using TrackingVolumeIdList = std::vector<MikanTrackingVolumeID>;

class TrackingVolumeObjectSystemConfig : public MikanObjectSystemDefinition
{
public:
	TrackingVolumeObjectSystemConfig(const std::string& configName)
		: MikanObjectSystemDefinition(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

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
	TrackingVolumeObjectSystem(class ProjectManager* ownerObjectSystemManager) : MikanObjectSystem(ownerObjectSystemManager) {}

	inline static const std::string k_objectSystemClassName = "TrackingVolumeObjectSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	virtual bool init() override;
	virtual void dispose() override;
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) override;

	virtual MikanObjectSystemDefinitionConstPtr getObjectSystemConfigConst() const override {
		return getTrackingVolumeSystemConfigConst();
	}
	TrackingVolumeObjectSystemConfigConstPtr getTrackingVolumeSystemConfigConst() const;
	TrackingVolumeObjectSystemConfigPtr getTrackingVolumeSystemConfig();

	const TrackingVolumeMap& getTrackingVolumeMap() const { return m_trackingVolumeComponents; }
	TrackingVolumeIdList getTrackingVolumeIdList() const;
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
