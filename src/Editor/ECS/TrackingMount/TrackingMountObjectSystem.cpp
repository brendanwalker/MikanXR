#include "TrackingMountObjectSystem.h"

// -- TrackingMountObjectSystemDefinition -----
TrackingMountObjectSystemDefinition::TrackingMountObjectSystemDefinition(const std::string& configName,
																		 IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- TrackingMountObjectSystem ----
TrackingMountObjectSystem::TrackingMountObjectSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}