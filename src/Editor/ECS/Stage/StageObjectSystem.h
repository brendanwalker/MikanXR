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
	MikanStageID addNewStage();
	MikanStageID addNewStage(const std::string& stageName);
	bool removeStage(MikanStageID sceneId);
	const std::vector<StageComponentDefinitionPtr>& getStageList() const { return m_stageList; }

private:
	MikanStageID m_nextStageId = 0;
	std::vector<StageComponentDefinitionPtr> m_stageList;
};

class StageObjectSystem : public MikanObjectSystem
{
public:
	StageObjectSystem(ProjectManagerPtr ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}

	inline static const std::string k_objectSystemClassName = "StageObjectSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	virtual bool init(MikanObjectSystemDefinitionPtr definitionPtr) override;
	virtual void dispose() override;

	StageObjectSystemConfigConstPtr getStageSystemConfigConst() const;
	StageObjectSystemConfigPtr getStageSystemConfig();

	virtual MikanComponentPtr getComponentById(int componentId) const override;

	const StageMap& getStageMap() const { return m_stageComponents; }
	StageComponentPtr getStageById(MikanStageID stageId) const;
	StageComponentPtr getStageByName(const std::string& stageName) const;
	StageComponentPtr addNewStage();
	bool removeStage(MikanStageID stageId);

	virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override;

protected:
	StageComponentPtr createStageObject(StageComponentDefinitionPtr sceneConfig);
	void disposeStageObject(MikanStageID sceneId);

	StageMap m_stageComponents;
};