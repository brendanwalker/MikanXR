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
#include <vector>

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp>

class GlmTransform;

using SceneMap = std::map<MikanSceneID, SceneComponentWeakPtr>;

class SceneObjectSystemConfig : public MikanObjectSystemDefinition
{
public:
	SceneObjectSystemConfig(const std::string& configName)
		: MikanObjectSystemDefinition(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_sceneListPropertyId;
	SceneComponentDefinitionPtr getSceneConfig(MikanSceneID sceneId) const;
	SceneComponentDefinitionPtr getSceneConfigByName(const std::string& sceneName) const;
	MikanSpatialAnchorID addNewScene(MikanStageID parentStageId);
	bool removeScene(MikanSceneID sceneId);
	const std::vector<SceneComponentDefinitionPtr>& getSceneList() const { return m_sceneList; }

	static const std::string k_currentSceneIdPropertyId;
	inline MikanSceneID getCurrentSceneId() const { return m_currentSceneId; }
	void setCurrentSceneId(MikanSceneID sceneId);

private:
	MikanSceneID m_nextSceneId = 0;
	MikanSceneID m_currentSceneId = -1;
	std::vector<SceneComponentDefinitionPtr> m_sceneList;
};

class SceneObjectSystem : public MikanObjectSystem
{
public:
	SceneObjectSystem(class ProjectManager* ownerObjectSystem) : MikanObjectSystem(ownerObjectSystem) {}
	static SceneObjectSystemPtr getSystem() { return s_sceneObjectSystem.lock(); }

	virtual bool init() override;
	virtual void dispose() override;

	virtual MikanObjectSystemDefinitionConstPtr getObjectSystemConfigConst() const override {
		return getSceneSystemConfigConst();
	}
	SceneObjectSystemConfigConstPtr getSceneSystemConfigConst() const;
	SceneObjectSystemConfigPtr getSceneSystemConfig();

	SceneComponentPtr getCurrentScene() const;
	void setCurrentScene(SceneComponentPtr scene);
	const SceneMap& getSceneMap() const { return m_sceneComponents; }
	SceneComponentPtr getSceneById(MikanSceneID sceneId) const;
	SceneComponentPtr getSceneByName(const std::string& sceneName) const;
	SceneComponentPtr addNewScene(MikanStageID parentStageId);
	bool removeScene(MikanSceneID sceneId);

	MulticastDelegate<void(SceneComponentPtr oldScene)> OnSceneDeactivated;
	MulticastDelegate<void(SceneComponentPtr newScene)> OnSceneActivated;

protected:
	SceneComponentPtr createSceneObject(SceneComponentDefinitionPtr sceneConfig);
	void disposeSceneObject(MikanSceneID sceneId);

	SceneMap m_sceneComponents;
	static SceneObjectSystemWeakPtr s_sceneObjectSystem;
};