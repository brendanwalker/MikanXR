#include "App.h"
#include "MikanObject.h"
#include "MikanPropertyDatabase.h"
#include "SceneComponent.h"
#include "SceneObjectSystem.h"
#include "ProjectConfig.h"

// -- SceneObjectSystemConfig -----
const std::string SceneObjectSystemDefinition::k_scenePoolPropertyId= "scene_pool";
const std::string SceneObjectSystemDefinition::k_currentSceneIdPropertyId = "current_scene_id";

SceneObjectSystemDefinition::SceneObjectSystemDefinition(const std::string& configName)
	: MikanObjectSystemDefinition(configName)
{
	m_scenePoolDefinition = std::make_shared<ScenePoolDefinition>(k_scenePoolPropertyId);
	addChildConfig(m_scenePoolDefinition);
}

configuru::Config SceneObjectSystemDefinition::writeToJSON()
{
	configuru::Config pt = MikanObjectSystemDefinition::writeToJSON();

	pt[k_currentSceneIdPropertyId] = m_currentSceneId;
	pt[k_scenePoolPropertyId] = m_scenePoolDefinition->writeToJSON();

	return pt;
}

void SceneObjectSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanObjectSystemDefinition::readFromJSON(pt);

	m_currentSceneId =
		pt.get_or<MikanSceneID>(
			SceneObjectSystemDefinition::k_currentSceneIdPropertyId, m_currentSceneId);

	// Read the scene pool definition
	if (pt.has_key(k_scenePoolPropertyId))
	{
		m_scenePoolDefinition->readFromJSON(pt[k_scenePoolPropertyId]);
	}
}

void SceneObjectSystemDefinition::setCurrentSceneId(MikanSceneID sceneId)
{
	if (sceneId != m_currentSceneId)
	{
		m_currentSceneId = sceneId;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_currentSceneIdPropertyId));
	}
}

SceneComponentDefinitionPtr SceneObjectSystemDefinition::getSceneDefinition(MikanSceneID sceneId) const
{
	return m_scenePoolDefinition->getById(sceneId);
}

SceneComponentDefinitionPtr SceneObjectSystemDefinition::getSceneDefinitionByName(const std::string& sceneName) const
{
	return m_scenePoolDefinition->getByName(sceneName);
}

const std::vector<SceneComponentDefinitionPtr>& SceneObjectSystemDefinition::getSceneList() const
{
	return m_scenePoolDefinition->getAll();
}

SceneComponentDefinitionPtr SceneObjectSystemDefinition::allocateNewScene(
	MikanStageID parentStageId)
{
	MikanSceneID nextId = m_scenePoolDefinition->allocateNextId();
	const std::string sceneName = "Scene_" + std::to_string(nextId);

	return std::make_shared<SceneComponentDefinition>(nextId, parentStageId, sceneName);
}

bool SceneObjectSystemDefinition::addScene(
	SceneComponentDefinitionPtr sceneDefinitionPtr)
{
	return m_scenePoolDefinition->addDefinition(sceneDefinitionPtr);
}

bool SceneObjectSystemDefinition::removeScene(MikanSceneID sceneId)
{
	return m_scenePoolDefinition->removeDefinition(sceneId);
}

SceneObjectSystem::SceneObjectSystem(class ProjectManager* ownerObjectSystem)
	: MikanObjectSystem(ownerObjectSystem)
	, m_scenePool(
		this,
		[this](auto def) { return this->createSceneObject(def); }) // scene factory lambda
{
}

bool SceneObjectSystem::init()
{
	MikanObjectSystem::init();

	SceneObjectSystemDefinitionConstPtr sceneSystemConfig = getSceneSystemConfigConst();

	// Create scene objects from definitions
	m_scenePool.initializeFromDefinitions(sceneSystemConfig->m_scenePoolDefinition);

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

	MikanObjectSystem::dispose();
}

MikanComponentPtr SceneObjectSystem::getComponentById(int componentId) const
{
	return getSceneById(componentId);
}

SceneObjectSystemDefinitionConstPtr SceneObjectSystem::getSceneSystemConfigConst() const
{
	auto config= getProjectConfig();
	return config ? getProjectConfig()->sceneConfig : SceneObjectSystemDefinitionConstPtr();
}

SceneObjectSystemDefinitionPtr SceneObjectSystem::getSceneSystemDefinition()
{
	return std::const_pointer_cast<SceneObjectSystemDefinition>(getSceneSystemConfigConst());
}

SceneComponentPtr SceneObjectSystem::getCurrentScene() const
{
	SceneObjectSystemDefinitionConstPtr sceneSystemConfigPtr = getSceneSystemConfigConst();
	if (sceneSystemConfigPtr)
	{
		return getSceneById(sceneSystemConfigPtr->getCurrentSceneId());
	}
	return SceneComponentPtr();
}

void SceneObjectSystem::setCurrentScene(SceneComponentPtr newScene)
{
	SceneObjectSystemDefinitionPtr sceneSystemConfig = getSceneSystemDefinition();
	if (sceneSystemConfig)
	{
		MikanSceneID currentSceneId = sceneSystemConfig->getCurrentSceneId();
		MikanSceneID newSceneId = newScene ? newScene->getSceneId() : INVALID_MIKAN_ID;

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

const SceneObjectSystem::ScenePool::ComponentMap& SceneObjectSystem::getSceneMap() const
{
	return m_scenePool.getAll();
}

SceneComponentPtr SceneObjectSystem::getSceneById(MikanSceneID sceneId) const
{
	return m_scenePool.getById(sceneId);
}

SceneComponentPtr SceneObjectSystem::getSceneByName(const std::string& sceneName) const
{
	return m_scenePool.getByName(sceneName);
}

SceneComponentPtr SceneObjectSystem::addNewScene(
	MikanStageID parentStageId)
{
	SceneObjectSystemDefinitionPtr systemDefinition = getSceneSystemDefinition();

	return createSceneObject(systemDefinition->allocateNewScene(parentStageId));
}

bool SceneObjectSystem::removeScene(MikanSceneID sceneId)
{
	SceneObjectSystemDefinitionPtr sceneSystemConfig = getSceneSystemDefinition();

	return
		m_scenePool.dispose(sceneId) &&
		sceneSystemConfig->removeScene(sceneId);
}

SceneComponentPtr SceneObjectSystem::createSceneObject(SceneComponentDefinitionPtr sceneConfig)
{
	MikanObjectPtr sceneObject = newObject();
	sceneObject->setName(sceneConfig->getComponentName());

	// Add spatial scene component to the object
	SceneComponentPtr sceneComponentPtr = sceneObject->addComponent<SceneComponent>();
	sceneObject->setRootComponent(sceneComponentPtr);
	sceneComponentPtr->setDefinition(sceneConfig);

	// Init the object once all components are added
	sceneObject->init();

	return sceneComponentPtr;
}

void SceneObjectSystem::registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase)
{
	propertyDatabase->registerPropertiesForSystem<SceneObjectSystem>();
	propertyDatabase->registerPropertiesForComponent<SceneObjectSystem, SceneComponent>();
}