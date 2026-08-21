#include "LightEnvironmentSystem.h"
#include "MikanObject.h"

// -- LightEnvironmentSystemDefinition -----
LightEnvironmentSystemDefinition::LightEnvironmentSystemDefinition(const std::string& configName,
																   IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- LightEnvironmentSystem -----
LightEnvironmentSystem::LightEnvironmentSystem(ProjectManagerPtr ownerProjectManager)
	: Super::MikanTypedObjectSystem(ownerProjectManager)
{
}
