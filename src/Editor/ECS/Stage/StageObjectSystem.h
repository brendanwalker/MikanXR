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

class StageObjectSystemConfig : public MikanObjectSystemDefinition
{
public:
	StageObjectSystemConfig(const std::string& configName)
		: MikanObjectSystemDefinition(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_stageListPropertyId;
	StageComponentDefinitionPtr getStageConfig(MikanStageID stageId) const;
	StageComponentDefinitionPtr getStageConfigByName(const std::string& stageName) const;
	MikanSpatialAnchorID addNewStage();
	MikanSpatialAnchorID addNewStage(const std::string& stageName);
	bool removeStage(MikanStageID sceneId);
	const std::vector<StageComponentDefinitionPtr>& getStageList() const { return m_stageList; }

private:
	MikanStageID m_nextStageId = 0;
	std::vector<StageComponentDefinitionPtr> m_stageList;
};

class StageObjectSystem : public MikanObjectSystem
{
public:
	StageObjectSystem(class ProjectManager* ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}
	static StageObjectSystemPtr getSystem() { return s_sceneObjectSystem.lock(); }

	virtual bool init() override;
	virtual void dispose() override;

	virtual MikanObjectSystemDefinitionConstPtr getObjectSystemConfigConst() const override {
		return getStageSystemConfigConst();
	}
	StageObjectSystemConfigConstPtr getStageSystemConfigConst() const;
	StageObjectSystemConfigPtr getStageSystemConfig();

	const StageMap& getStageMap() const { return m_stageComponents; }
	StageComponentPtr getStageById(MikanStageID stageId) const;
	StageComponentPtr getStageByName(const std::string& stageName) const;
	StageComponentPtr addNewStage();
	bool removeStage(MikanStageID stageId);

protected:
	StageComponentPtr createStageObject(StageComponentDefinitionPtr sceneConfig);
	void disposeStageObject(MikanStageID sceneId);

	StageMap m_stageComponents;
	static StageObjectSystemWeakPtr s_sceneObjectSystem;
};