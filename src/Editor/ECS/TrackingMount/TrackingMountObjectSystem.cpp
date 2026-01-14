#include "TrackingMountObjectSystem.h"

// -- TrackingMountObjectSystemDefinition -----
TrackingMountObjectSystemDefinition::TrackingMountObjectSystemDefinition(const std::string& configName)
	: Super::MikanTypedObjectSystemDefinition(configName)
{
}

// -- TrackingMountObjectSystem ----
TrackingMountObjectSystem::TrackingMountObjectSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}