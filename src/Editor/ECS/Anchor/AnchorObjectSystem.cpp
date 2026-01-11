#include "App.h"
#include "AnchorObjectSystem.h"
#include "AnchorComponent.h"
#include "BoxColliderComponent.h"
#include "TransformComponent.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "MikanPropertyDatabase.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

// -- AnchorObjectSystemConfig -----
const std::string AnchorObjectSystemDefinition::k_anchorVRDevicePathPropertyId= "anchorVRDevicePath";
const std::string AnchorObjectSystemDefinition::k_anchorListPropertyId= "spatialAnchors";
const std::string AnchorObjectSystemDefinition::k_renderAnchorsPropertyId= "render_anchors";

configuru::Config AnchorObjectSystemDefinition::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["anchorVRDevicePath"] = anchorVRDevicePath;
	pt["nextAnchorId"] = nextAnchorId;
	pt["debugRenderAnchors"] = m_bDebugRenderAnchors;

	std::vector<configuru::Config> anchorConfigs;
	for (AnchorDefinitionPtr AnchorDefinitionPtr : spatialAnchorList)
	{
		anchorConfigs.push_back(AnchorDefinitionPtr->writeToJSON());
	}
	pt.insert_or_assign(std::string("spatialAnchors"), anchorConfigs);

	return pt;
}

void AnchorObjectSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	anchorVRDevicePath = pt.get_or<std::string>("anchorVRDevicePath", anchorVRDevicePath);
	nextAnchorId = pt.get_or<int>("nextAnchorId", nextAnchorId);
	m_bDebugRenderAnchors = pt.get_or<bool>("debugRenderAnchors", m_bDebugRenderAnchors);

	// Read in the spatial anchors
	spatialAnchorList.clear();
	if (pt.has_key("spatialAnchors"))
	{
		for (const configuru::Config& anchor_pt : pt["spatialAnchors"].as_array())
		{
			AnchorDefinitionPtr AnchorDefinitionPtr = std::make_shared<AnchorDefinition>();

			AnchorDefinitionPtr->readFromJSON(anchor_pt);
			spatialAnchorList.push_back(AnchorDefinitionPtr);

			addChildConfig(AnchorDefinitionPtr);
		}
	}
}

bool AnchorObjectSystemDefinition::canAddAnchor() const
{
	return true;
}

AnchorDefinitionPtr AnchorObjectSystemDefinition::getSpatialAnchorConfig(MikanSpatialAnchorID anchorId) const
{
	auto it = std::find_if(
		spatialAnchorList.begin(), spatialAnchorList.end(),
		[anchorId](AnchorDefinitionPtr configPtr) {
			return configPtr->getAnchorId() == anchorId;
		});

	if (it != spatialAnchorList.end())
	{
		return AnchorDefinitionPtr(*it);
	}

	return AnchorDefinitionPtr();
}

AnchorDefinitionPtr AnchorObjectSystemDefinition::getSpatialAnchorConfigByName(const std::string& anchorName) const
{
	auto it = std::find_if(
		spatialAnchorList.begin(), spatialAnchorList.end(),
		[anchorName](AnchorDefinitionPtr configPtr) {
			return configPtr->getComponentName() == anchorName;
		});

	if (it != spatialAnchorList.end())
	{
		return AnchorDefinitionPtr(*it);
	}

	return AnchorDefinitionPtr();
}

MikanSpatialAnchorID AnchorObjectSystemDefinition::addNewAnchor(MikanSceneID ownerSceneId)
{
	const MikanTransform anchorXform = glm_transform_to_MikanTransform(glm::mat4(1.f));
	const std::string newAnchorName = StringUtils::stringify("Anchor ", nextAnchorId);

	return addNewAnchor(ownerSceneId, newAnchorName, anchorXform);
}

MikanSpatialAnchorID AnchorObjectSystemDefinition::addNewAnchor(
	MikanSceneID ownerSceneId,
	const std::string& anchorName, 
	const MikanTransform& xform)
{
	if (!canAddAnchor())
		return INVALID_MIKAN_ID;

	AnchorDefinitionPtr AnchorDefinitionPtr = std::make_shared<AnchorDefinition>(nextAnchorId, anchorName, xform);
	nextAnchorId++;

	spatialAnchorList.push_back(AnchorDefinitionPtr);
	addChildConfig(AnchorDefinitionPtr);

	notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_anchorListPropertyId));

	return AnchorDefinitionPtr->getAnchorId();
}

bool AnchorObjectSystemDefinition::removeAnchor(MikanSpatialAnchorID anchorId)
{
	auto it = std::find_if(
		spatialAnchorList.begin(), spatialAnchorList.end(),
		[anchorId](AnchorDefinitionPtr configPtr) {
		return configPtr->getAnchorId() == anchorId;
	});

	if (it != spatialAnchorList.end())
	{
		removeChildConfig(*it);

		spatialAnchorList.erase(it);
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_anchorListPropertyId));

		return true;
	}

	return false;
}

void AnchorObjectSystemDefinition::setRenderAnchorsFlag(bool flag)
{
	if (m_bDebugRenderAnchors != flag)
	{
		m_bDebugRenderAnchors = flag;
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_renderAnchorsPropertyId));
	}
}

// -- AnchorObjectSystem -----
bool AnchorObjectSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
	MikanObjectSystem::init(definitionPtr);

	AnchorObjectSystemDefinitionConstPtr anchorSystemConfig = getAnchorSystemConfigConst();

	for (AnchorDefinitionPtr anchorConfig : anchorSystemConfig->spatialAnchorList)
	{
		createAnchorObject(anchorConfig);
	}

	return true;
}

void AnchorObjectSystem::dispose()
{
	m_anchorComponents.clear();

	MikanObjectSystem::dispose();
}

MikanComponentPtr AnchorObjectSystem::getComponentById(int componentId) const
{
	return getSpatialAnchorById(componentId);
}

void AnchorObjectSystem::deleteObjectConfig(MikanObjectPtr objectPtr)
{
	AnchorComponentPtr anchorComponent= objectPtr->getComponentOfType<AnchorComponent>();
	if (anchorComponent != nullptr)
	{
		removeAnchor(anchorComponent->getAnchorDefinition()->getAnchorId());
	}
}

AnchorComponentPtr AnchorObjectSystem::getSpatialAnchorById(MikanSpatialAnchorID anchorId) const
{
	auto iter= m_anchorComponents.find(anchorId);
	if (iter != m_anchorComponents.end())
	{
		return iter->second.lock();
	}

	return AnchorComponentPtr();
}

AnchorComponentPtr AnchorObjectSystem::getSpatialAnchorByName(const std::string& anchorName) const
{
	for (auto it = m_anchorComponents.begin(); it != m_anchorComponents.end(); it++)
	{
		AnchorComponentPtr componentPtr= it->second.lock();

		if (componentPtr && componentPtr->getName() == anchorName)
		{
			return componentPtr;
		}
	}

	return AnchorComponentPtr();
}

bool AnchorObjectSystem::getSpatialAnchorWorldTransform(MikanSpatialAnchorID anchorId, glm::mat4& outXform) const
{
	AnchorComponentPtr anchorPtr= getSpatialAnchorById(anchorId);
	if (anchorPtr)
	{
		outXform= anchorPtr->getWorldTransform();
		return true;
	}

	return false;
}

AnchorComponentPtr AnchorObjectSystem::addNewAnchor(MikanStageID ownerStageId)
{
	AnchorObjectSystemDefinitionPtr anchorSystemConfig = getAnchorSystemConfig();

	MikanSpatialAnchorID anchorId= anchorSystemConfig->addNewAnchor(ownerStageId);
	if (anchorId != INVALID_MIKAN_ID)
	{		
		AnchorDefinitionPtr anchorConfig= anchorSystemConfig->getSpatialAnchorConfig(anchorId);
		assert(anchorConfig != nullptr);

		return createAnchorObject(anchorConfig);
	}

	return AnchorComponentPtr();
}

AnchorComponentPtr AnchorObjectSystem::addNewAnchor(
	MikanStageID ownerStageId,
	const std::string& anchorName, 
	const class GlmTransform& xform)
{
	AnchorObjectSystemDefinitionPtr anchorSystemConfig = getAnchorSystemConfig();

	MikanSpatialAnchorID anchorId = anchorSystemConfig->addNewAnchor(
		ownerStageId,
		anchorName, 
		glm_transform_to_MikanTransform(xform));
	if (anchorId != INVALID_MIKAN_ID)
	{
		AnchorDefinitionPtr anchorConfig = anchorSystemConfig->getSpatialAnchorConfig(anchorId);
		assert(anchorConfig != nullptr);

		return createAnchorObject(anchorConfig);
	}

	return AnchorComponentPtr();
}

bool AnchorObjectSystem::removeAnchor(MikanSpatialAnchorID anchorId)
{
	getAnchorSystemConfig()->removeAnchor(anchorId);
	disposeAnchorObject(anchorId);

	return false;
}

AnchorComponentPtr AnchorObjectSystem::createAnchorObject(AnchorDefinitionPtr anchorConfig)
{
	AnchorObjectSystemDefinitionConstPtr anchorSystemConfig = getAnchorSystemConfigConst();
	MikanObjectPtr anchorObject= newObject();
	anchorObject->setName(anchorConfig->getComponentName());

	// Add spatial anchor component to the object
	AnchorComponentPtr anchorComponentPtr= anchorObject->addComponent<AnchorComponent>();
	anchorObject->setRootComponent(anchorComponentPtr);
	anchorComponentPtr->setDefinition(anchorConfig);
	m_anchorComponents.insert({anchorConfig->getAnchorId(), anchorComponentPtr});

	// Add a selection component
	anchorObject->addComponent<SelectionComponent>();

	// Attach a box collider to quad stencil component
	const float size= 0.1f;
	BoxColliderComponentPtr boxColliderPtr = anchorObject->addComponent<BoxColliderComponent>();
	boxColliderPtr->setHalfExtents(glm::vec3(size * 0.5f, size * 0.5f, size * 0.5f));
	boxColliderPtr->setRelativeTransform(GlmTransform(glm::vec3(size * 0.5f, size * 0.5f, size * 0.5f)));
	boxColliderPtr->attachToComponent(anchorComponentPtr);

	// Init the object once all components are added
	anchorObject->init();

	return anchorComponentPtr;
}

void AnchorObjectSystem::disposeAnchorObject(MikanSpatialAnchorID anchorId)
{
	auto it= m_anchorComponents.find(anchorId);
	if (it != m_anchorComponents.end())
	{
		AnchorComponentPtr anchorComponentPtr= it->second.lock();

		// Remove for component list
		m_anchorComponents.erase(it);

		// Free the corresponding object
		deleteObject(anchorComponentPtr->getOwnerObject());
	}
}

AnchorObjectSystemDefinitionConstPtr AnchorObjectSystem::getAnchorSystemConfigConst() const
{
	return getProjectConfig()->anchorConfig;
}

AnchorObjectSystemDefinitionPtr AnchorObjectSystem::getAnchorSystemConfig()
{
	return std::const_pointer_cast<AnchorObjectSystemDefinition>(getAnchorSystemConfigConst());
}

void AnchorObjectSystem::registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase)
{
	propertyDatabase->registerPropertiesForSystem<AnchorObjectSystem>();
	propertyDatabase->registerPropertiesForComponent<AnchorObjectSystem, AnchorComponent>();
}