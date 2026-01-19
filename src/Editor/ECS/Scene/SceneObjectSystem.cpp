#include "App.h"
#include "MikanObject.h"
#include "MikanPropertyDatabase.h"
#include "SceneComponent.h"
#include "SceneObjectSystem.h"
#include "ProjectConfig.h"

// -- SceneObjectSystemDefinition -----
const std::string SceneObjectSystemDefinition::k_currentSceneIdPropertyId = "current_scene_id";

SceneObjectSystemDefinition::SceneObjectSystemDefinition(const std::string& configName)
	: Super::MikanTypedObjectSystemDefinition(configName)
{
}

configuru::Config SceneObjectSystemDefinition::writeToJSON()
{
	configuru::Config pt = Super::writeToJSON();

	pt[k_currentSceneIdPropertyId] = m_currentSceneId;

	return pt;
}

void SceneObjectSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	Super::readFromJSON(pt);

	m_currentSceneId =
		pt.get_or<MikanSceneID>(
			SceneObjectSystemDefinition::k_currentSceneIdPropertyId, m_currentSceneId);
}

void SceneObjectSystemDefinition::setCurrentSceneId(MikanSceneID sceneId)
{
	if (sceneId != m_currentSceneId)
	{
		m_currentSceneId = sceneId;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_currentSceneIdPropertyId));
	}
}

// -- SceneObjectSystem -----
SceneObjectSystem::SceneObjectSystem(ProjectManagerPtr ownerObjectSystem)
	: Super::MikanTypedObjectSystem(ownerObjectSystem)
{
}

bool SceneObjectSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
	Super::init(definitionPtr);

	SceneComponentPtr sceneComponent = getCurrentScene();
	if (sceneComponent)
	{
		sceneComponent->activateScene();
	}

	return true;
}

void SceneObjectSystem::dispose()
{
	SceneComponentPtr sceneComponent = getCurrentScene();
	if (sceneComponent)
	{
		sceneComponent->deactivateScene();
	}

	Super::dispose();
}

SceneComponentPtr SceneObjectSystem::getCurrentScene() const
{
	SceneObjectSystemDefinitionConstPtr sceneSystemConfigPtr = getTypedDefinitionConst();
	if (sceneSystemConfigPtr)
	{
		return getSceneById(sceneSystemConfigPtr->getCurrentSceneId());
	}
	return SceneComponentPtr();
}

void SceneObjectSystem::setCurrentScene(SceneComponentPtr newScene)
{
	setCurrentSceneById(newScene->getSceneId());
}

void SceneObjectSystem::setCurrentSceneById(MikanSceneID newSceneId)
{
	SceneObjectSystemDefinitionPtr sceneSystemConfig = getTypedDefinition();
	if (sceneSystemConfig)
	{
		MikanSceneID currentSceneId = sceneSystemConfig->getCurrentSceneId();
		if (currentSceneId != newSceneId)
		{
			SceneComponentPtr currentScene = getSceneById(currentSceneId);
			if (currentScene)
			{
				currentScene->deactivateScene();

				if (OnSceneDeactivated)
				{
					OnSceneDeactivated(currentScene);
				}
			}

			sceneSystemConfig->setCurrentSceneId(newSceneId);
			SceneComponentPtr newScene = getSceneById(newSceneId);
			if (newScene)
			{
				newScene->activateScene();

				if (OnSceneActivated)
				{
					OnSceneActivated(newScene);
				}
			}
		}
	}
}