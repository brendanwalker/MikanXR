#include "CameraObjectSystem.h"

// -- CameraObjectSystemDefinition -----
CameraObjectSystemDefinition::CameraObjectSystemDefinition(
	const std::string& configName, IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- CameraObjectSystem -----
CameraObjectSystem::CameraObjectSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}