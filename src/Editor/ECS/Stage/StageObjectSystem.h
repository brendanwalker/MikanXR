#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "MikanObjectSystem.h"
#include "MulticastDelegate.h"
#include "ObjectSystemFwd.h"
#include "ObjectSystemConfigFwd.h"
#include "SceneFwd.h"

#include <map>
#include <memory>
#include <string>

using StageMap = std::map<MikanStageID, StageComponentWeakPtr>;

class StageObjectSystemConfig : public CommonConfig
{
public:
	StageObjectSystemConfig(const std::string& configName)
		: CommonConfig(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	StageComponentDefinitionPtr getStageConfig(MikanStageID stageId) const;
	StageComponentDefinitionPtr getStageConfigByName(const std::string& stageName) const;
	MikanSpatialAnchorID addNewStage(const std::string& stageName);
	bool removeStage(MikanStageID sceneId);

	static const std::string k_stageListPropertyId;

private:
	MikanStageID m_nextStageId = 0;
	std::vector<StageComponentDefinitionPtr> m_stageList;
};

class StageObjectSystem : public MikanObjectSystem
{
public:
	StageObjectSystem(class ObjectSystemManager* ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}
	static StageObjectSystemPtr getSystem() { return s_sceneObjectSystem.lock(); }

	virtual bool init() override;
	virtual void dispose() override;

	StageObjectSystemConfigConstPtr getStageSystemConfigConst() const;
	StageObjectSystemConfigPtr getStageSystemConfig();

	const StageMap& getStageMap() const { return m_stageComponents; }
	StageComponentPtr getStageById(MikanStageID stageId) const;
	StageComponentPtr getStageByName(const std::string& stageName) const;
	StageComponentPtr addNewStage(const std::string& stageName);
	bool removeStage(MikanStageID stageId);

protected:
	StageComponentPtr createStageObject(StageComponentDefinitionPtr sceneConfig);
	void disposeStageObject(MikanStageID sceneId);

	StageMap m_stageComponents;
	static StageObjectSystemWeakPtr s_sceneObjectSystem;
};