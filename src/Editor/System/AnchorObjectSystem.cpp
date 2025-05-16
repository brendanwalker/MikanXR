#include "App.h"
#include "AnchorObjectSystem.h"
#include "AnchorComponent.h"
#include "BoxColliderComponent.h"
#include "SceneComponent.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

// -- AnchorObjectSystemConfig -----
const std::string AnchorObjectSystemConfig::k_anchorVRDevicePathPropertyId= "anchorVRDevicePath";
const std::string AnchorObjectSystemConfig::k_anchorListPropertyId= "spatialAnchors";
const std::string AnchorObjectSystemConfig::k_renderAnchorsPropertyId= "render_anchors";

configuru::Config AnchorObjectSystemConfig::writeToJSON()
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

void AnchorObjectSystemConfig::readFromJSON(const configuru::Config& pt)
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

bool AnchorObjectSystemConfig::canAddAnchor() const
{
	return true;
}

AnchorDefinitionPtr AnchorObjectSystemConfig::getSpatialAnchorConfig(MikanSpatialAnchorID anchorId) const
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

AnchorDefinitionPtr AnchorObjectSystemConfig::getSpatialAnchorConfigByName(const std::string& anchorName) const
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

MikanSpatialAnchorID AnchorObjectSystemConfig::addNewAnchor(const std::string& anchorName, const MikanTransform& xform)
{
	if (!canAddAnchor())
		return INVALID_MIKAN_ID;

	AnchorDefinitionPtr AnchorDefinitionPtr = std::make_shared<AnchorDefinition>(nextAnchorId, anchorName, xform);
	nextAnchorId++;

	spatialAnchorList.push_back(AnchorDefinitionPtr);
	addChildConfig(AnchorDefinitionPtr);

	markDirty(ConfigPropertyChangeSet().addPropertyName(k_anchorListPropertyId));

	return AnchorDefinitionPtr->getAnchorId();
}

bool AnchorObjectSystemConfig::removeAnchor(MikanSpatialAnchorID anchorId)
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
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_anchorListPropertyId));

		return true;
	}

	return false;
}

void AnchorObjectSystemConfig::setRenderAnchorsFlag(bool flag)
{
	if (m_bDebugRenderAnchors != flag)
	{
		m_bDebugRenderAnchors = flag;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_renderAnchorsPropertyId));
	}
}

// -- AnchorObjectSystem -----
AnchorObjectSystemWeakPtr AnchorObjectSystem::s_anchorObjectSystem;

bool AnchorObjectSystem::init()
{
	MikanObjectSystem::init();

	AnchorObjectSystemConfigConstPtr anchorSystemConfig = getAnchorSystemConfigConst();

	for (AnchorDefinitionPtr anchorConfig : anchorSystemConfig->spatialAnchorList)
	{
		createAnchorObject(anchorConfig);
	}

	s_anchorObjectSystem = std::static_pointer_cast<AnchorObjectSystem>(shared_from_this());
	return true;
}

void AnchorObjectSystem::dispose()
{
	s_anchorObjectSystem.reset();
	m_anchorComponents.clear();

	MikanObjectSystem::dispose();
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

AnchorComponentPtr AnchorObjectSystem::addNewAnchor(const std::string& anchorName, const GlmTransform& xform)
{
	AnchorObjectSystemConfigPtr anchorSystemConfig = getAnchorSystemConfig();

	MikanSpatialAnchorID anchorId= anchorSystemConfig->addNewAnchor(anchorName, glm_transform_to_MikanTransform(xform));
	if (anchorId != INVALID_MIKAN_ID)
	{		
		AnchorDefinitionPtr anchorConfig= anchorSystemConfig->getSpatialAnchorConfig(anchorId);
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
	AnchorObjectSystemConfigConstPtr anchorSystemConfig = getAnchorSystemConfigConst();
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

AnchorObjectSystemConfigConstPtr AnchorObjectSystem::getAnchorSystemConfigConst() const
{
	return App::getInstance()->getProfileConfig()->anchorConfig;
}

AnchorObjectSystemConfigPtr AnchorObjectSystem::getAnchorSystemConfig()
{
	return std::const_pointer_cast<AnchorObjectSystemConfig>(getAnchorSystemConfigConst());
}