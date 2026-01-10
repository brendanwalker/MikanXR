#pragma once

#include "ComponentFwd.h"
#include "MarkerTrackingVolumeComponent.h"
#include "MikanObjectSystem.h"
#include "MikanTypeFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "ProjectConfigConstants.h"

#include <map>
#include <memory>
#include <string>
#include <vector>

using MarkerTrackingVolumeMap = std::map<MikanTrackingVolumeID, MarkerTrackingVolumeComponentWeakPtr>;
using MarkerTrackingVolumeIdList = std::vector<MikanTrackingVolumeID>;

class MarkerTrackingVolumeSystemDefinition : public MikanObjectSystemDefinition
{
public:
	MarkerTrackingVolumeSystemDefinition(const std::string& configName)
		: MikanObjectSystemDefinition(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_markerTrackingVolumeListPropertyId;
	const std::vector<MarkerTrackingVolumeDefinitionPtr>& getMarkerTrackingVolumeList() const { return m_markerTrackingVolumeList; }
	MarkerTrackingVolumeDefinitionConstPtr getMarkerTrackingVolumeDefinitionConst(MikanTrackingVolumeID trackingVolumeId) const;
	MarkerTrackingVolumeDefinitionPtr getMarkerTrackingVolumeDefinition(MikanTrackingVolumeID trackingVolumeId);
	MikanTrackingVolumeID addMarkerTrackingVolume();
	bool removeMarkerTrackingVolume(MikanTrackingVolumeID trackingVolumeId);

protected:
	std::vector<MarkerTrackingVolumeDefinitionPtr> m_markerTrackingVolumeList;
	MikanTrackingVolumeID m_nextTrackingVolumeId = 0;
};

class MarkerTrackingVolumeSystem : public MikanObjectSystem
{
public:
	MarkerTrackingVolumeSystem(ProjectManagerPtr ownerObjectSystemManager) : MikanObjectSystem(ownerObjectSystemManager) {}

	inline static const std::string k_objectSystemClassName = "MarkerTrackingVolumeSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr) override;
	virtual void dispose() override;

	MarkerTrackingVolumeSystemDefinitionConstPtr getMarkerTrackingVolumeSystemConfigConst() const;
	MarkerTrackingVolumeSystemDefinitionPtr getMarkerTrackingVolumeSystemConfig();

	virtual MikanComponentPtr getComponentById(int componentId) const override;

	const MarkerTrackingVolumeMap& getMarkerTrackingVolumeMap() const { return m_markerTrackingVolumeComponents; }
	MarkerTrackingVolumeIdList getTrackingVolumeIdList() const;
	MarkerTrackingVolumeComponentPtr getMarkerTrackingVolumeById(MikanTrackingVolumeID trackingVolumeId) const;

	MarkerTrackingVolumeComponentPtr addNewMarkerTrackingVolume();
	bool removeMarkerTrackingVolume(MikanTrackingVolumeID trackingVolumeId);

	virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override;

protected:
	MarkerTrackingVolumeComponentPtr createMarkerTrackingVolumeObject(MarkerTrackingVolumeDefinitionPtr trackingVolumeConfig);
	void disposeMarkerTrackingVolumeObject(MikanTrackingVolumeID trackingVolumeId);

	MarkerTrackingVolumeMap m_markerTrackingVolumeComponents;
};
