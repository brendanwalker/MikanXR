#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "MikanObjectSystem.h"
#include "MikanTypedObjectSystemDefinition.h"
#include "MikanTypedObjectSystem.h"
#include "MulticastDelegate.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "SceneFwd.h"
#include "StageComponent.h"

#include <map>
#include <memory>
#include <string>

class StageObjectSystemDefinition :
	public MikanTypedObjectSystemDefinition<StageComponentDefinition, MikanStageID>
{
public:
	using Super = MikanTypedObjectSystemDefinition<StageComponentDefinition, MikanStageID>;

	StageObjectSystemDefinition(const std::string& configName);

	static const std::string k_stagePoolPropertyId;
};

class StageObjectSystem :
	public MikanTypedObjectSystem<
		StageComponent, StageComponentDefinition,
		MikanStageID,
		StageObjectSystem, StageObjectSystemDefinition>
{
public:
	using Super = MikanTypedObjectSystem<
		StageComponent, StageComponentDefinition,
		MikanStageID,
		StageObjectSystem, StageObjectSystemDefinition>;

	StageObjectSystem(ProjectManagerPtr ownerObjectSystem);

	inline static const std::string k_objectSystemClassName = "StageObjectSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	inline StageComponentPtr getStageById(MikanStageID stageId) const {
		return Super::getTypedComponentById(stageId);
	}
	inline StageComponentPtr getStageByName(const std::string& stageName) const {
		return Super::getTypedComponentByName(stageName);
	}
};