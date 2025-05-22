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

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp>

class GlmTransform;

using SceneMap = std::map<MikanSceneID, SceneComponentWeakPtr>;

class SceneObjectSystemConfig : public CommonConfig
{
public:
	SceneObjectSystemConfig(const std::string& configName)
		: CommonConfig(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	SceneComponentDefinitionPtr getSceneConfig(MikanSpatialAnchorID sceneId) const;
	SceneComponentDefinitionPtr getSceneConfigByName(const std::string& sceneName) const;
	MikanSpatialAnchorID addNewScene(const std::string& sceneName, MikanStageID parentStageId);
	bool removeScene(MikanSceneID sceneId);

	static const std::string k_sceneListPropertyId;
	std::vector<SceneComponentDefinitionPtr> sceneList;

	MikanSceneID nextSceneId = 0;
};

class SceneObjectSystem : public MikanObjectSystem
{
public:
	static SceneObjectSystemPtr getSystem() { return s_sceneObjectSystem.lock(); }

	virtual bool init() override;
	virtual void dispose() override;

	SceneObjectSystemConfigConstPtr getSceneSystemConfigConst() const;
	SceneObjectSystemConfigPtr getSceneSystemConfig();

	const SceneMap& getSceneMap() const { return m_sceneComponents; }
	SceneComponentPtr getSceneById(MikanSceneID sceneId) const;
	SceneComponentPtr getSceneByName(const std::string& sceneName) const;
	SceneComponentPtr addNewScene(const std::string& sceneName, MikanStageID parentStageId);
	bool removeScene(MikanSceneID sceneId);


protected:
	SceneComponentPtr createSceneObject(SceneComponentDefinitionPtr sceneConfig);
	void disposeSceneObject(MikanSceneID sceneId);

	SceneMap m_sceneComponents;
	static SceneObjectSystemWeakPtr s_sceneObjectSystem;
};