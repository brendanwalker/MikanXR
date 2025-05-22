#include "App.h"
#include "MikanObject.h"
#include "SceneComponent.h"
#include "SceneObjectSystem.h"
#include "ProjectConfig.h"

// -- SceneObjectSystemConfig -----
const std::string SceneObjectSystemConfig::k_sceneListPropertyId= "sceneList";

configuru::Config SceneObjectSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	return pt;
}

void SceneObjectSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);
}

// -- SceneObjectSystemConfig -----
SceneComponentDefinitionPtr SceneObjectSystemConfig::getSceneConfig(MikanSpatialAnchorID sceneId) const
{
	auto it = std::find_if(
		sceneList.begin(), sceneList.end(),
		[sceneId](SceneComponentDefinitionPtr configPtr) {
			return configPtr->getSceneId() == sceneId;
		});

	if (it != sceneList.end())
	{
		return SceneComponentDefinitionPtr(*it);
	}

	return SceneComponentDefinitionPtr();
}

SceneComponentDefinitionPtr SceneObjectSystemConfig::getSceneConfigByName(const std::string& sceneName) const
{
	auto it = std::find_if(
		sceneList.begin(), sceneList.end(),
		[sceneName](SceneComponentDefinitionPtr configPtr) {
			return configPtr->getComponentName() == sceneName;
		});

	if (it != sceneList.end())
	{
		return SceneComponentDefinitionPtr(*it);
	}

	return SceneComponentDefinitionPtr();
}

MikanSpatialAnchorID SceneObjectSystemConfig::addNewScene(
	const std::string& sceneName,
	MikanStageID parentStageId)
{
	auto sceneDefinitionPtr = 
		std::make_shared<SceneComponentDefinition>(
			nextSceneId, parentStageId, sceneName);
	nextSceneId++;

	sceneList.push_back(sceneDefinitionPtr);
	addChildConfig(sceneDefinitionPtr);

	markDirty(ConfigPropertyChangeSet().addPropertyName(k_sceneListPropertyId));

	return sceneDefinitionPtr->getSceneId();
}

bool SceneObjectSystemConfig::removeScene(MikanSceneID sceneId)
{
	auto it = std::find_if(
		sceneList.begin(), sceneList.end(),
		[sceneId](SceneComponentDefinitionPtr configPtr) {
			return configPtr->getSceneId() == sceneId;
		});

	if (it != sceneList.end())
	{
		removeChildConfig(*it);

		sceneList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_sceneListPropertyId));

		return true;
	}

	return false;
}

// -- SceneObjectSystem -----
SceneObjectSystemWeakPtr SceneObjectSystem::s_sceneObjectSystem;

bool SceneObjectSystem::init()
{
	MikanObjectSystem::init();

	SceneObjectSystemConfigConstPtr sceneSystemConfig = getSceneSystemConfigConst();

	s_sceneObjectSystem = std::static_pointer_cast<SceneObjectSystem>(shared_from_this());
	return true;
}

void SceneObjectSystem::dispose()
{
	s_sceneObjectSystem.reset();

	MikanObjectSystem::dispose();
}

SceneObjectSystemConfigConstPtr SceneObjectSystem::getSceneSystemConfigConst() const
{
	return App::getInstance()->getProfileConfig()->sceneConfig;
}

SceneObjectSystemConfigPtr SceneObjectSystem::getSceneSystemConfig()
{
	return std::const_pointer_cast<SceneObjectSystemConfig>(getSceneSystemConfigConst());
}

SceneComponentPtr SceneObjectSystem::getSceneById(MikanSceneID sceneId) const
{
	auto iter = m_sceneComponents.find(sceneId);
	if (iter != m_sceneComponents.end())
	{
		return iter->second.lock();
	}

	return SceneComponentPtr();
}

SceneComponentPtr SceneObjectSystem::getSceneByName(const std::string& sceneName) const
{
	for (auto it = m_sceneComponents.begin(); it != m_sceneComponents.end(); it++)
	{
		SceneComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getName() == sceneName)
		{
			return componentPtr;
		}
	}

	return SceneComponentPtr();
}

SceneComponentPtr SceneObjectSystem::addNewScene(
	const std::string& sceneName, 
	MikanStageID parentStageId)
{
	SceneObjectSystemConfigPtr sceneSystemConfig = getSceneSystemConfig();

	MikanSceneID sceneId = sceneSystemConfig->addNewScene(sceneName, parentStageId);
	if (sceneId != INVALID_MIKAN_ID)
	{
		SceneComponentDefinitionPtr sceneConfig = sceneSystemConfig->getSceneConfig(sceneId);
		assert(sceneConfig != nullptr);

		return createSceneObject(sceneConfig);
	}

	return SceneComponentPtr();
}

bool SceneObjectSystem::removeScene(MikanSceneID sceneId)
{
	bool bValidScene= getSceneSystemConfig()->removeScene(sceneId);
	disposeSceneObject(sceneId);

	return bValidScene;
}

SceneComponentPtr SceneObjectSystem::createSceneObject(SceneComponentDefinitionPtr sceneConfig)
{
	SceneObjectSystemConfigConstPtr sceneSystemConfig = getSceneSystemConfigConst();
	MikanObjectPtr sceneObject = newObject();
	sceneObject->setName(sceneConfig->getComponentName());

	// Add spatial scene component to the object
	SceneComponentPtr sceneComponentPtr = sceneObject->addComponent<SceneComponent>();
	sceneObject->setRootComponent(sceneComponentPtr);
	sceneComponentPtr->setDefinition(sceneConfig);
	m_sceneComponents.insert({sceneConfig->getSceneId(), sceneComponentPtr});

	// Init the object once all components are added
	sceneObject->init();

	return sceneComponentPtr;
}

void SceneObjectSystem::disposeSceneObject(MikanSceneID sceneId)
{
	auto it = m_sceneComponents.find(sceneId);
	if (it != m_sceneComponents.end())
	{
		SceneComponentPtr sceneComponentPtr = it->second.lock();

		// Remove for component list
		m_sceneComponents.erase(it);

		// Free the corresponding object
		deleteObject(sceneComponentPtr->getOwnerObject());
	}
}
