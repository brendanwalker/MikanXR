#include "App.h"
#include "MikanObject.h"
#include "MikanPropertyDatabase.h"
#include "StageComponent.h"
#include "StageObjectSystem.h"
#include "ProjectConfig.h"

// -- StageObjectSystemDefinition -----
const std::string StageObjectSystemDefinition::k_stagePoolPropertyId = "stage_pool";

StageObjectSystemDefinition::StageObjectSystemDefinition(const std::string& configName)
	: Super::MikanTypedObjectSystemDefinition(configName, k_stagePoolPropertyId)
{
}

// -- StageObjectSystem -----
StageObjectSystem::StageObjectSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}
