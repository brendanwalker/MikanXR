#pragma once

#include "ComponentFwd.h"
#include "MarkerTrackingVolumeComponent.h"
#include "MikanTypedObjectSystem.h"
#include "MikanTypeFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "ProjectConfigConstants.h"

#include <memory>
#include <string>
#include <vector>

using MarkerTrackingVolumeIdList = std::vector<MikanTrackingVolumeID>;

class MarkerTrackingVolumeSystemDefinition :
	public MikanTypedObjectSystemDefinition<MarkerTrackingVolumeComponent, MarkerTrackingVolumeDefinition, MikanTrackingVolumeID>
{
public:
	using Super = MikanTypedObjectSystemDefinition<MarkerTrackingVolumeComponent, MarkerTrackingVolumeDefinition, MikanTrackingVolumeID>;

	MarkerTrackingVolumeSystemDefinition(const std::string& configName, IEntityIDAllocatorPtr idAllocator);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);
};

class MarkerTrackingVolumeSystem :
	public MikanTypedObjectSystem<
		MarkerTrackingVolumeComponent, MarkerTrackingVolumeDefinition,
		MikanTrackingVolumeID,
		MarkerTrackingVolumeSystem, MarkerTrackingVolumeSystemDefinition>
{
public:
	using Super = MikanTypedObjectSystem<
		MarkerTrackingVolumeComponent, MarkerTrackingVolumeDefinition,
		MikanTrackingVolumeID,
		MarkerTrackingVolumeSystem, MarkerTrackingVolumeSystemDefinition>;

	MarkerTrackingVolumeSystem(ProjectManagerPtr ownerObjectSystemManager);

	inline static const std::string k_objectSystemClassName = "MarkerTrackingVolumeSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	MarkerTrackingVolumeIdList getTrackingVolumeIdList() const;
};
