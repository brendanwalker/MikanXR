#include "App.h"
#include "MikanObject.h"
#include "StageComponent.h"
#include "StageObjectSystem.h"
#include "ProjectConfig.h"

// -- StageObjectSystemConfig -----
const std::string StageObjectSystemConfig::k_stageListPropertyId= "stageList";

configuru::Config StageObjectSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	return pt;
}

void StageObjectSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);
}

// -- StageObjectSystemConfig -----
StageComponentDefinitionPtr StageObjectSystemConfig::getStageConfig(MikanStageID stageId) const
{
	auto it = std::find_if(
		stageList.begin(), stageList.end(),
		[stageId](StageComponentDefinitionPtr configPtr) {
			return configPtr->getStageId() == stageId;
		});

	if (it != stageList.end())
	{
		return StageComponentDefinitionPtr(*it);
	}

	return StageComponentDefinitionPtr();
}

StageComponentDefinitionPtr StageObjectSystemConfig::getStageConfigByName(const std::string& stageName) const
{
	auto it = std::find_if(
		stageList.begin(), stageList.end(),
		[stageName](StageComponentDefinitionPtr configPtr) {
			return configPtr->getComponentName() == stageName;
		});

	if (it != stageList.end())
	{
		return StageComponentDefinitionPtr(*it);
	}

	return StageComponentDefinitionPtr();
}

MikanSpatialAnchorID StageObjectSystemConfig::addNewStage(
	const std::string& stageName)
{
	auto stageDefinitionPtr = 
		std::make_shared<StageComponentDefinition>(
			nextStageId, stageName);
	nextStageId++;

	stageList.push_back(stageDefinitionPtr);
	addChildConfig(stageDefinitionPtr);

	markDirty(ConfigPropertyChangeSet().addPropertyName(k_stageListPropertyId));

	return stageDefinitionPtr->getStageId();
}

bool StageObjectSystemConfig::removeStage(MikanStageID stageId)
{
	auto it = std::find_if(
		stageList.begin(), stageList.end(),
		[stageId](StageComponentDefinitionPtr configPtr) {
			return configPtr->getStageId() == stageId;
		});

	if (it != stageList.end())
	{
		removeChildConfig(*it);

		stageList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_stageListPropertyId));

		return true;
	}

	return false;
}

// -- StageObjectSystem -----
StageObjectSystemWeakPtr StageObjectSystem::s_sceneObjectSystem;

bool StageObjectSystem::init()
{
	MikanObjectSystem::init();

	StageObjectSystemConfigConstPtr sceneSystemConfig = getStageSystemConfigConst();

	s_sceneObjectSystem = std::static_pointer_cast<StageObjectSystem>(shared_from_this());
	return true;
}

void StageObjectSystem::dispose()
{
	s_sceneObjectSystem.reset();

	MikanObjectSystem::dispose();
}

StageObjectSystemConfigConstPtr StageObjectSystem::getStageSystemConfigConst() const
{
	return App::getInstance()->getProfileConfig()->stageConfig;
}

StageObjectSystemConfigPtr StageObjectSystem::getStageSystemConfig()
{
	return std::const_pointer_cast<StageObjectSystemConfig>(getStageSystemConfigConst());
}

StageComponentPtr StageObjectSystem::getStageById(MikanStageID sceneId) const
{
	auto iter = m_stageComponents.find(sceneId);
	if (iter != m_stageComponents.end())
	{
		return iter->second.lock();
	}

	return StageComponentPtr();
}

StageComponentPtr StageObjectSystem::getStageByName(const std::string& sceneName) const
{
	for (auto it = m_stageComponents.begin(); it != m_stageComponents.end(); it++)
	{
		StageComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getName() == sceneName)
		{
			return componentPtr;
		}
	}

	return StageComponentPtr();
}

StageComponentPtr StageObjectSystem::addNewStage(
	const std::string& stageName)
{
	StageObjectSystemConfigPtr stageSystemConfig = getStageSystemConfig();

	MikanStageID stageId = stageSystemConfig->addNewStage(stageName);
	if (stageId != INVALID_MIKAN_ID)
	{
		StageComponentDefinitionPtr stageConfig = stageSystemConfig->getStageConfig(stageId);
		assert(stageConfig != nullptr);

		return createStageObject(stageConfig);
	}

	return StageComponentPtr();
}

bool StageObjectSystem::removeStage(MikanStageID stageId)
{
	bool bValidStage= getStageSystemConfig()->removeStage(stageId);
	disposeStageObject(stageId);

	return bValidStage;
}

StageComponentPtr StageObjectSystem::createStageObject(StageComponentDefinitionPtr stageConfig)
{
	StageObjectSystemConfigConstPtr sceneSystemConfig = getStageSystemConfigConst();
	MikanObjectPtr stageObject = newObject();
	stageObject->setName(stageConfig->getComponentName());

	// Add scene component to the object
	StageComponentPtr stageComponentPtr = stageObject->addComponent<StageComponent>();
	stageObject->setRootComponent(stageComponentPtr);
	stageComponentPtr->setDefinition(stageConfig);
	m_stageComponents.insert({stageConfig->getStageId(), stageComponentPtr});

	// Init the object once all components are added
	stageObject->init();

	return stageComponentPtr;
}

void StageObjectSystem::disposeStageObject(MikanStageID stageId)
{
	auto it = m_stageComponents.find(stageId);
	if (it != m_stageComponents.end())
	{
		StageComponentPtr sceneComponentPtr = it->second.lock();

		// Remove for component list
		m_stageComponents.erase(it);

		// Free the corresponding object
		deleteObject(sceneComponentPtr->getOwnerObject());
	}
}
