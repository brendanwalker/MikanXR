#include "App.h"
#include "SceneObjectSystem.h"
#include "CameraComponent.h"
#include "SceneComponent.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

// -- SceneObjectSystemConfig -----
const std::string SceneObjectSystemConfig::k_cameraListPropertyId= "cameras";

configuru::Config SceneObjectSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["nextCameraId"] = nextCameraId;

	std::vector<configuru::Config> cameraConfigs;
	for (CameraDefinitionPtr cameraDefinition : cameraList)
	{
		cameraConfigs.push_back(cameraDefinition->writeToJSON());
	}
	pt.insert_or_assign(std::string("Cameras"), cameraConfigs);

	return pt;
}

void SceneObjectSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	nextCameraId = pt.get_or<int>("nextCameraId", nextCameraId);

	// Read in the cameras
	cameraList.clear();
	if (pt.has_key("cameras"))
	{
		for (const configuru::Config& cameraConfig : pt["cameras"].as_array())
		{
			CameraDefinitionPtr cameraDefinition = std::make_shared<CameraDefinition>();

			cameraDefinition->readFromJSON(cameraConfig);
			cameraList.push_back(cameraDefinition);

			addChildConfig(cameraDefinition);
		}
	}
}

CameraDefinitionPtr SceneObjectSystemConfig::getCameraConfig(MikanCameraID cameraId) const
{
	auto it = std::find_if(
		cameraList.begin(), cameraList.end(),
		[cameraId](CameraDefinitionPtr configPtr) {
			return configPtr->getCameraId() == cameraId;
		});

	if (it != cameraList.end())
	{
		return CameraDefinitionPtr(*it);
	}

	return CameraDefinitionPtr();
}

CameraDefinitionPtr SceneObjectSystemConfig::getCameraConfigByName(const std::string& cameraName) const
{
	auto it = std::find_if(
		cameraList.begin(), cameraList.end(),
		[cameraName](CameraDefinitionPtr configPtr) {
			return configPtr->getComponentName() == cameraName;
		});

	if (it != cameraList.end())
	{
		return CameraDefinitionPtr(*it);
	}

	return CameraDefinitionPtr();
}

MikanCameraID SceneObjectSystemConfig::addNewCamera(
	const std::string& cameraName, 
	const struct MikanTransform& xform)
{
	CameraDefinitionPtr cameraDefinition = std::make_shared<CameraDefinition>(nextCameraId, cameraName, xform);
	nextCameraId++;

	cameraList.push_back(cameraDefinition);
	addChildConfig(cameraDefinition);

	markDirty(ConfigPropertyChangeSet().addPropertyName(k_cameraListPropertyId));

	return cameraDefinition->getCameraId();
}

bool SceneObjectSystemConfig::removeCamera(MikanCameraID cameraId)
{
	auto it = std::find_if(
		cameraList.begin(), cameraList.end(),
		[cameraId](CameraDefinitionPtr configPtr) {
		return configPtr->getCameraId() == cameraId;
	});

	if (it != cameraList.end())
	{
		removeChildConfig(*it);

		cameraList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_cameraListPropertyId));

		return true;
	}

	return false;
}

// -- SceneObjectSystem -----
SceneObjectSystemWeakPtr SceneObjectSystem::s_SceneObjectSystem;

bool SceneObjectSystem::init()
{
	MikanObjectSystem::init();

	SceneObjectSystemConfigConstPtr anchorSystemConfig = getSceneSystemConfigConst();

	for (CameraDefinitionPtr cameraConfig : anchorSystemConfig->cameraList)
	{
		createCameraObject(cameraConfig);
	}

	s_SceneObjectSystem = std::static_pointer_cast<SceneObjectSystem>(shared_from_this());
	return true;
}

void SceneObjectSystem::dispose()
{
	s_SceneObjectSystem.reset();
	m_cameraComponents.clear();

	MikanObjectSystem::dispose();
}

void SceneObjectSystem::deleteObjectConfig(MikanObjectPtr objectPtr)
{
	CameraComponentPtr anchorComponent= objectPtr->getComponentOfType<CameraComponent>();
	if (anchorComponent != nullptr)
	{
		removeCamera(anchorComponent->getCameraDefinition()->getCameraId());
	}
}

CameraComponentPtr SceneObjectSystem::getCameraById(MikanCameraID cameraId) const
{
	auto iter= m_cameraComponents.find(cameraId);
	if (iter != m_cameraComponents.end())
	{
		return iter->second.lock();
	}

	return CameraComponentPtr();
}

CameraComponentPtr SceneObjectSystem::getCameraByName(const std::string& cameraName) const
{
	for (auto it = m_cameraComponents.begin(); it != m_cameraComponents.end(); it++)
	{
		CameraComponentPtr componentPtr= it->second.lock();

		if (componentPtr && componentPtr->getName() == cameraName)
		{
			return componentPtr;
		}
	}

	return CameraComponentPtr();
}

CameraComponentPtr SceneObjectSystem::addNewCamera(const std::string& cameraName, const GlmTransform& xform)
{
	SceneObjectSystemConfigPtr anchorSystemConfig = getSceneSystemConfig();

	MikanCameraID cameraId= anchorSystemConfig->addNewCamera(cameraName, glm_transform_to_MikanTransform(xform));
	if (cameraId != INVALID_MIKAN_ID)
	{		
		CameraDefinitionPtr cameraConfig= anchorSystemConfig->getCameraConfig(cameraId);
		assert(cameraConfig != nullptr);

		return createCameraObject(cameraConfig);
	}

	return CameraComponentPtr();
}

bool SceneObjectSystem::removeCamera(MikanCameraID cameraId)
{
	getSceneSystemConfig()->removeCamera(cameraId);
	disposeCameraObject(cameraId);

	return false;
}

CameraComponentPtr SceneObjectSystem::createCameraObject(CameraDefinitionPtr cameraConfig)
{
	SceneObjectSystemConfigConstPtr anchorSystemConfig = getSceneSystemConfigConst();
	MikanObjectPtr cameraObject= newObject();
	cameraObject->setName(cameraConfig->getComponentName());

	// Add camera component to the object
	CameraComponentPtr cameraComponent= cameraObject->addComponent<CameraComponent>();
	cameraObject->setRootComponent(cameraComponent);
	cameraComponent->setDefinition(cameraConfig);
	m_cameraComponents.insert({cameraConfig->getCameraId(), cameraComponent});

	// Init the object once all components are added
	cameraObject->init();

	return cameraComponent;
}

void SceneObjectSystem::disposeCameraObject(MikanCameraID cameraId)
{
	auto it= m_cameraComponents.find(cameraId);
	if (it != m_cameraComponents.end())
	{
		CameraComponentPtr cameraComponent= it->second.lock();

		// Remove for component list
		m_cameraComponents.erase(it);

		// Free the corresponding object
		deleteObject(cameraComponent->getOwnerObject());
	}
}

SceneObjectSystemConfigConstPtr SceneObjectSystem::getSceneSystemConfigConst() const
{
	return App::getInstance()->getProfileConfig()->sceneConfig;
}

SceneObjectSystemConfigPtr SceneObjectSystem::getSceneSystemConfig()
{
	return std::const_pointer_cast<SceneObjectSystemConfig>(getSceneSystemConfigConst());
}