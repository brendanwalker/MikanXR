#include "App.h"
#include "MikanObject.h"
#include "MikanPropertyDatabase.h"
#include "StageComponent.h"
#include "StageObjectSystem.h"
#include "ProjectConfig.h"

// -- StageObjectSystemDefinition -----
StageObjectSystemDefinition::StageObjectSystemDefinition(const std::string& configName,
														 IEntityIDAllocatorPtr idAllocator)
	: Super::MikanTypedObjectSystemDefinition(configName, idAllocator)
{
}

// -- StageObjectSystem -----
StageObjectSystem::StageObjectSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}
