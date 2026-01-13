#include "CameraObjectSystem.h"

// -- CameraObjectSystemDefinition -----
CameraObjectSystemDefinition::CameraObjectSystemDefinition(const std::string& configName)
	: Super::MikanTypedObjectSystemDefinition(configName)
{
}

// -- CameraObjectSystem -----
CameraObjectSystem::CameraObjectSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}