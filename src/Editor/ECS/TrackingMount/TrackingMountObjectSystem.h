#pragma once

#include "TrackingMountComponent.h"
#include "MikanTypedObjectSystem.h"

class TrackingMountObjectSystemDefinition : public MikanTypedObjectSystemDefinition<TrackingMountComponent, TrackingMountDefinition, MikanTrackingMountID>
{
public:
	using Super= MikanTypedObjectSystemDefinition<TrackingMountComponent, TrackingMountDefinition, MikanTrackingMountID>;

	TrackingMountObjectSystemDefinition(const std::string& configName, IEntityIDAllocatorPtr idAllocator);
};

class TrackingMountObjectSystem : public MikanTypedObjectSystem<
									  TrackingMountComponent, TrackingMountDefinition,
									  MikanTrackingMountID,
									  TrackingMountObjectSystem, TrackingMountObjectSystemDefinition>
{
public:
	using Super= MikanTypedObjectSystem<
		TrackingMountComponent, TrackingMountDefinition,
		MikanTrackingMountID,
		TrackingMountObjectSystem, TrackingMountObjectSystemDefinition>;

	TrackingMountObjectSystem(ProjectManagerPtr ownerObjectSystem);

	inline static const std::string k_objectSystemClassName= "TrackingMountObjectSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }
};