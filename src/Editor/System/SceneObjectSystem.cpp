#include "App.h"
#include "SceneObjectSystem.h"
#include "ProjectConfig.h"

// -- SceneObjectSystemConfig -----
configuru::Config SceneObjectSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	return pt;
}

void SceneObjectSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);
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