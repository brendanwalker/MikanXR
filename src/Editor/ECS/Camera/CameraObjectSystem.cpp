#include "App.h"
#include "CameraObjectSystem.h"
#include "CompositorObjectSystem.h"
#include "CameraComponent.h"
#include "CompositorComponent.h"
#include "TransformComponent.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "MikanPropertyDatabase.h"
#include "ProjectConfig.h"
#include "StringUtils.h"
#include "SceneFwd.h"

// -- CameraObjectSystemConfig -----
const std::string CameraObjectSystemConfig::k_cameraListPropertyId= "camera_ids";

configuru::Config CameraObjectSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["m_nextCameraId"] = m_nextCameraId;

	std::vector<configuru::Config> cameraConfigs;
	for (CameraDefinitionPtr cameraDefinition : cameraList)
	{
		cameraConfigs.push_back(cameraDefinition->writeToJSON());
	}
	pt.insert_or_assign(std::string("Cameras"), cameraConfigs);

	return pt;
}

void CameraObjectSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_nextCameraId = pt.get_or<int>("m_nextCameraId", m_nextCameraId);

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

CameraDefinitionPtr CameraObjectSystemConfig::getCameraConfig(MikanCameraID cameraId) const
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

CameraDefinitionPtr CameraObjectSystemConfig::getCameraConfigByName(const std::string& cameraName) const
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

MikanCameraID CameraObjectSystemConfig::addNewCamera(MikanStageID ownerStageId)
{
	const std::string cameraName = "Camera" + std::to_string(m_nextCameraId);
	const MikanTransform xform= glm_transform_to_MikanTransform(GlmTransform());

	return addNewCamera(cameraName, xform, ownerStageId);
}

MikanCameraID CameraObjectSystemConfig::addNewCamera(
	const std::string& cameraName, 
	const struct MikanTransform& xform,
	MikanStageID ownerStageId)
{
	CameraDefinitionPtr cameraDefinition = 
		std::make_shared<CameraDefinition>(
			cameraName, xform, m_nextCameraId, ownerStageId);
	m_nextCameraId++;

	cameraList.push_back(cameraDefinition);
	addChildConfig(cameraDefinition);

	notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_cameraListPropertyId));

	return cameraDefinition->getCameraId();
}

bool CameraObjectSystemConfig::removeCamera(MikanCameraID cameraId)
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
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_cameraListPropertyId));

		return true;
	}

	return false;
}

// -- CameraObjectSystem -----
CameraObjectSystemWeakPtr CameraObjectSystem::s_cameraObjectSystem;

bool CameraObjectSystem::init()
{
	MikanObjectSystem::init();

	CameraObjectSystemConfigConstPtr cameraSystemConfig = getCameraSystemConfigConst();

	for (CameraDefinitionPtr cameraConfig : cameraSystemConfig->getCameraList())
	{
		createCameraObject(cameraConfig);
	}

	s_cameraObjectSystem = std::static_pointer_cast<CameraObjectSystem>(shared_from_this());
	return true;
}

void CameraObjectSystem::dispose()
{
	s_cameraObjectSystem.reset();
	m_cameraComponents.clear();

	MikanObjectSystem::dispose();
}

MikanComponentPtr CameraObjectSystem::getComponentById(int componentId) const
{
	return getCameraById(componentId);
}

void CameraObjectSystem::deleteObjectConfig(MikanObjectPtr objectPtr)
{
	CameraComponentPtr anchorComponent= objectPtr->getComponentOfType<CameraComponent>();
	if (anchorComponent != nullptr)
	{
		removeCamera(anchorComponent->getCameraDefinition()->getCameraId());
	}
}

CameraComponentPtr CameraObjectSystem::getCameraById(MikanCameraID cameraId) const
{
	auto iter= m_cameraComponents.find(cameraId);
	if (iter != m_cameraComponents.end())
	{
		return iter->second.lock();
	}

	return CameraComponentPtr();
}

CameraComponentPtr CameraObjectSystem::getCameraByName(const std::string& cameraName) const
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

CameraComponentPtr CameraObjectSystem::addNewCamera(const MikanStageID ownerStageId)
{
	CameraObjectSystemConfigPtr cameraSystemConfig = getCameraSystemConfig();

	MikanCameraID cameraId= 
		cameraSystemConfig->addNewCamera(ownerStageId);
	if (cameraId != INVALID_MIKAN_ID)
	{		
		CameraDefinitionPtr cameraConfig= cameraSystemConfig->getCameraConfig(cameraId);
		assert(cameraConfig != nullptr);

		return createCameraObject(cameraConfig);
	}

	return CameraComponentPtr();
}

bool CameraObjectSystem::removeCamera(MikanCameraID cameraId)
{
	if (getCameraSystemConfig()->removeCamera(cameraId))
	{
		disposeCameraObject(cameraId);
		return true;
	}

	return false;
}

CameraComponentPtr CameraObjectSystem::createCameraObject(CameraDefinitionPtr cameraConfig)
{
	CameraObjectSystemConfigConstPtr cameraSystemConfig = getCameraSystemConfigConst();
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

void CameraObjectSystem::disposeCameraObject(MikanCameraID cameraId)
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

CameraObjectSystemConfigConstPtr CameraObjectSystem::getCameraSystemConfigConst() const
{
	return getProjectConfig()->cameraConfig;
}

CameraObjectSystemConfigPtr CameraObjectSystem::getCameraSystemConfig()
{
	return std::const_pointer_cast<CameraObjectSystemConfig>(getCameraSystemConfigConst());
}

std::vector<MikanCameraID> CameraObjectSystem::getAllCameraIds() const
{
	std::vector<MikanCameraID> cameraIds;
	for (const auto& pair : m_cameraComponents)
	{
		cameraIds.push_back(pair.first);
	}
	return cameraIds;
}

void CameraObjectSystem::registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase)
{
	propertyDatabase->registerPropertiesForSystem<CameraObjectSystem>();
	propertyDatabase->registerPropertiesForComponent<CameraObjectSystem, CameraComponent>();
}