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

using CompositorMap = std::map<MikanCompositorID, CompositorComponentWeakPtr>;

class CompositorObjectSystemConfig : public MikanObjectSystemDefinition
{
public:
	CompositorObjectSystemConfig(const std::string& configName)
		: MikanObjectSystemDefinition(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	CompositorDefinitionPtr getCompositorConfig(MikanCompositorID compositorId) const;
	CompositorDefinitionPtr getCompositorConfigByName(const std::string& compositorName) const;

	static const std::string k_compositorListPropertyId;
	MikanCompositorID addNewCompositor(MikanStageID ownerStageId);
	bool removeCompositor(MikanCompositorID compositorId);
	const std::vector<CompositorDefinitionPtr>& getCompositorList() const { return m_compositorList; }

private:
	MikanCompositorID m_nextCompositorId = 0;
	std::vector<CompositorDefinitionPtr> m_compositorList;
};

class CompositorObjectSystem : public MikanObjectSystem
{
public:
	CompositorObjectSystem(class ProjectManager* ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}

	inline static const std::string k_objectSystemClassName = "CompositorObjectSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	virtual bool init() override;
	virtual void dispose() override;

	virtual MikanObjectSystemDefinitionConstPtr getObjectSystemConfigConst() const override {
		return getCompositorSystemConfigConst();
	}
	CompositorObjectSystemConfigConstPtr getCompositorSystemConfigConst() const;
	CompositorObjectSystemConfigPtr getCompositorSystemConfig();

	virtual MikanComponentPtr getComponentById(int componentId) const override;

	const CompositorMap& getCompositorMap() const { return m_compositorComponents; }
	CompositorComponentPtr getCompositorById(MikanCompositorID compositorId) const;
	CompositorComponentPtr getCompositorByName(const std::string& compositorName) const;
	std::vector<MikanCompositorID> getCompositorIdListForStage(MikanStageID stageId) const;
	CompositorComponentPtr addNewCompositor(MikanStageID ownerStageId);
	bool removeCompositor(MikanCompositorID compositorId);

	void setActiveCompositors(const std::vector<MikanCompositorID>& activeCompositorIdList);
	MulticastDelegate<void(CompositorComponentPtr oldCompositor)> OnCompositorDeactivated;
	MulticastDelegate<void(CompositorComponentPtr newCompositor)> OnCompositorActivated;

	virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override;

protected:
	CompositorComponentPtr createCompositorObject(CompositorDefinitionPtr compositorConfig);
	void disposeCompositorObject(MikanCompositorID compositorId);

private:
	CompositorMap m_compositorComponents;
};
