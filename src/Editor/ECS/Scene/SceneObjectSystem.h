#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MikanTypeFwd.h"
#include "MikanObjectSystem.h"
#include "MikanTypedObjectPoolDefinition.h"
#include "MikanTypedObjectPool.h"
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

class SceneObjectSystemDefinition : public MikanObjectSystemDefinition
{
public:
	using ScenePoolDefinition = MikanTypedObjectPoolDefinition<SceneComponentDefinition, MikanSceneID>;
	using ScenePoolDefinitionPtr = std::shared_ptr<ScenePoolDefinition>;

	SceneObjectSystemDefinition(const std::string& configName);

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_scenePoolPropertyId;
	SceneComponentDefinitionPtr getSceneDefinition(MikanSceneID sceneId) const;
	SceneComponentDefinitionPtr getSceneDefinitionByName(const std::string& sceneName) const;
	SceneComponentDefinitionPtr allocateNewScene(MikanStageID parentStageId);
	bool addScene(SceneComponentDefinitionPtr sceneDefinitionPtr);
	bool removeScene(MikanSceneID sceneId);
	const std::vector<SceneComponentDefinitionPtr>& getSceneList() const;

	static const std::string k_currentSceneIdPropertyId;
	inline MikanSceneID getCurrentSceneId() const { return m_currentSceneId; }
	void setCurrentSceneId(MikanSceneID sceneId);

	ScenePoolDefinitionPtr m_scenePoolDefinition;

private:
	MikanSceneID m_currentSceneId = -1;
};

class SceneObjectSystem : public MikanObjectSystem
{
public:
	using ScenePool = MikanTypedObjectPool<SceneComponent, SceneComponentDefinition, MikanSceneID>;

	SceneObjectSystem(class ProjectManager* ownerObjectSystem);

	inline static const std::string k_objectSystemClassName = "SceneObjectSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	virtual bool init() override;
	virtual void dispose() override;

	virtual MikanObjectSystemDefinitionConstPtr getObjectSystemConfigConst() const override {
		return getSceneSystemConfigConst();
	}
	SceneObjectSystemDefinitionConstPtr getSceneSystemConfigConst() const;
	SceneObjectSystemDefinitionPtr getSceneSystemDefinition();

	virtual MikanComponentPtr getComponentById(int componentId) const override;

	SceneComponentPtr getCurrentScene() const;
	void setCurrentScene(SceneComponentPtr scene);
	const ScenePool::ComponentMap& getSceneMap() const;
	SceneComponentPtr getSceneById(MikanSceneID sceneId) const;
	SceneComponentPtr getSceneByName(const std::string& sceneName) const;
	SceneComponentPtr addNewScene(MikanStageID parentStageId);
	bool removeScene(MikanSceneID sceneId);

	MulticastDelegate<void(SceneComponentPtr oldScene)> OnSceneDeactivated;
	MulticastDelegate<void(SceneComponentPtr newScene)> OnSceneActivated;

	virtual void registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase) override;

protected:
	SceneComponentPtr createSceneObject(SceneComponentDefinitionPtr sceneConfig);
	ScenePool m_scenePool;
};