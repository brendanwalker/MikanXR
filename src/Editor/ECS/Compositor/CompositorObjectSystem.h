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

class CompositorObjectSystemConfig : public CommonConfig
{
public:
	CompositorObjectSystemConfig(const std::string& configName)
		: CommonConfig(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	CompositorDefinitionPtr getCompositorConfig(MikanCompositorID compositorId) const;
	CompositorDefinitionPtr getCompositorConfigByName(const std::string& compositorName) const;

	static const std::string k_compositorListPropertyId;
	MikanCompositorID addNewCompositor(const std::string& compositorName);
	bool removeCompositor(MikanCompositorID compositorId);

private:
	MikanCompositorID m_nextCompositorId = 0;
	std::vector<CompositorDefinitionPtr> m_compositorList;
};

class CompositorObjectSystem : public MikanObjectSystem
{
public:
	CompositorObjectSystem(class ObjectSystemManager* ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}
	static CompositorObjectSystemPtr getSystem() { return s_compositorObjectSystem.lock(); }

	virtual bool init() override;
	virtual void dispose() override;

	CompositorObjectSystemConfigConstPtr getCompositorSystemConfigConst() const;
	CompositorObjectSystemConfigPtr getCompositorSystemConfig();

	const CompositorMap& getCompositorMap() const { return m_compositorComponents; }
	CompositorComponentPtr getCompositorById(MikanCompositorID compositorId) const;
	CompositorComponentPtr getCompositorByName(const std::string& compositorName) const;
	CompositorComponentPtr addNewCompositor(const std::string& compositorName);
	bool removeCompositor(MikanCompositorID compositorId);

	MulticastDelegate<void(CompositorComponentPtr oldCompositor)> OnCompositorDeactivated;
	MulticastDelegate<void(CompositorComponentPtr newCompositor)> OnCompositorActivated;

protected:
	CompositorComponentPtr createCompositorObject(CompositorDefinitionPtr compositorConfig);
	void disposeCompositorObject(MikanCompositorID compositorId);

private:
	CompositorMap m_compositorComponents;
	static CompositorObjectSystemWeakPtr s_compositorObjectSystem;
};
