#include "App.h"
#include "AnchorObjectSystem.h"
#include "AnchorComponent.h"
#include "SceneComponent.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "ProfileConfig.h"
#include "StringUtils.h"

// -- AnchorObjectSystemConfig -----
configuru::Config AnchorObjectSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["anchorVRDevicePath"] = anchorVRDevicePath;
	pt["nextAnchorId"] = nextAnchorId;
	pt["debugRenderAnchors"] = debugRenderAnchors;

	std::vector<configuru::Config> anchorConfigs;
	for (AnchorConfigPtr anchorConfigPtr : spatialAnchorList)
	{
		anchorConfigs.push_back(anchorConfigPtr->writeToJSON());
	}
	pt.insert_or_assign(std::string("spatialAnchors"), anchorConfigs);

	return pt;
}

void AnchorObjectSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	anchorVRDevicePath = pt.get_or<std::string>("anchorVRDevicePath", anchorVRDevicePath);
	nextAnchorId = pt.get_or<int>("nextAnchorId", nextAnchorId);
	debugRenderAnchors = pt.get_or<bool>("debugRenderAnchors", debugRenderAnchors);

	// Read in the spatial anchors
	spatialAnchorList.clear();
	if (pt.has_key("spatialAnchors"))
	{
		for (const configuru::Config& anchor_pt : pt["spatialAnchors"].as_array())
		{
			AnchorConfigPtr anchorConfigPtr = std::make_shared<AnchorConfig>();
			anchorConfigPtr->readFromJSON(anchor_pt);
			spatialAnchorList.push_back(anchorConfigPtr);
		}
	}

	// Special case: Origin spatial anchor
	AnchorConfigPtr originAnchorInfo = getSpatialAnchorInfoByName(ORIGIN_SPATIAL_ANCHOR_NAME);
	if (originAnchorInfo)
	{
		originAnchorId = originAnchorInfo->getAnchorId();
	}
	else
	{
		const MikanMatrix4f originXform = glm_mat4_to_MikanMatrix4f(glm::mat4(1.f));

		originAnchorId = nextAnchorId;
		addNewAnchor(ORIGIN_SPATIAL_ANCHOR_NAME, originXform);
	}
}

bool AnchorObjectSystemConfig::canAddAnchor() const
{
	return (spatialAnchorList.size() < MAX_MIKAN_SPATIAL_ANCHORS);
}

AnchorConfigPtr AnchorObjectSystemConfig::getSpatialAnchorInfo(MikanSpatialAnchorID anchorId) const
{
	auto it = std::find_if(
		spatialAnchorList.begin(), spatialAnchorList.end(),
		[anchorId](AnchorConfigPtr configPtr) {
		return configPtr->getAnchorId() == anchorId;
	});

	if (it != spatialAnchorList.end())
	{
		return AnchorConfigPtr(*it);
	}

	return AnchorConfigPtr();
}

AnchorConfigPtr AnchorObjectSystemConfig::getSpatialAnchorInfoByName(const std::string& anchorName) const
{
	auto it = std::find_if(
		spatialAnchorList.begin(), spatialAnchorList.end(),
		[anchorName](AnchorConfigPtr configPtr) {
		return strncmp(configPtr->getAnchorInfo().anchor_name, anchorName.c_str(), MAX_MIKAN_ANCHOR_NAME_LEN) == 0;
	});

	if (it != spatialAnchorList.end())
	{
		return AnchorConfigPtr(*it);
	}

	return AnchorConfigPtr();
}


MikanSpatialAnchorID AnchorObjectSystemConfig::addNewAnchor(const std::string& anchorName, const MikanMatrix4f& xform)
{
	if (!canAddAnchor())
		return INVALID_MIKAN_ID;

	AnchorConfigPtr anchorConfigPtr = std::make_shared<AnchorConfig>(nextAnchorId, anchorName, xform);
	nextAnchorId++;

	spatialAnchorList.push_back(anchorConfigPtr);
	markDirty();

	return anchorConfigPtr->getAnchorId();
}

bool AnchorObjectSystemConfig::removeAnchor(MikanSpatialAnchorID anchorId)
{
	auto it = std::find_if(
		spatialAnchorList.begin(), spatialAnchorList.end(),
		[anchorId](AnchorConfigPtr configPtr) {
		return configPtr->getAnchorId() == anchorId;
	});

	if (it != spatialAnchorList.end() &&
		(*it)->getAnchorId() != originAnchorId)
	{
		spatialAnchorList.erase(it);
		markDirty();

		return true;
	}

	return false;
}

// -- AnchorObjectSystem -----
AnchorObjectSystem* AnchorObjectSystem::s_anchorObjectSystem= nullptr;

AnchorObjectSystem::AnchorObjectSystem()
{
	s_anchorObjectSystem = this;
}

AnchorObjectSystem::~AnchorObjectSystem()
{
	s_anchorObjectSystem= nullptr;
}

void AnchorObjectSystem::init()
{
	MikanObjectSystem::init();

	AnchorObjectSystemConfigConstPtr anchorConfig = getAnchorConfigConst();
	for (AnchorConfigPtr anchorConfig : anchorConfig->spatialAnchorList)
	{
		createAnchorObject(anchorConfig);
	}
}

void AnchorObjectSystem::dispose()
{
	m_anchorComponents.clear();
	MikanObjectSystem::dispose();
}

AnchorComponentWeakPtr AnchorObjectSystem::getSpatialAnchorById(MikanSpatialAnchorID anchorId) const
{
	auto iter= m_anchorComponents.find(anchorId);
	if (iter != m_anchorComponents.end())
	{
		return iter->second;
	}

	return AnchorComponentWeakPtr();
}

AnchorComponentWeakPtr AnchorObjectSystem::getSpatialAnchorByName(const std::string& anchorName) const
{
	for (auto it = m_anchorComponents.begin(); it != m_anchorComponents.end(); it++)
	{
		AnchorComponentPtr componentPtr= it->second.lock();

		if (componentPtr && componentPtr->getAnchorName() == anchorName)
		{
			return componentPtr;
		}
	}

	return AnchorComponentWeakPtr();
}

AnchorComponentPtr AnchorObjectSystem::addNewAnchor(const std::string& anchorName, const glm::mat4& xform)
{
	AnchorObjectSystemConfig& anchorConfig = getAnchorConfig();

	MikanSpatialAnchorID anchorId= anchorConfig.addNewAnchor(anchorName, glm_mat4_to_MikanMatrix4f(xform));
	if (anchorId != INVALID_MIKAN_ID)
	{
		const MikanSpatialAnchorInfo* anchorInfo= anchorConfig.getSpatialAnchorInfo(anchorId);
		assert(anchorInfo != nullptr);
	}

	return AnchorComponentPtr();
}

bool AnchorObjectSystem::removeAnchor(MikanSpatialAnchorID anchorId)
{
	getAnchorConfig().removeAnchor(anchorId);
	disposeAnchorObject(anchorId);

	return false;
}

AnchorComponentPtr AnchorObjectSystem::createAnchorObject(AnchorConfigPtr anchorConfig)
{
	MikanObjectPtr anchorObject= newObject().lock();

	// Add a scene component to the anchor
	SceneComponentPtr sceneComponentPtr= anchorObject->addComponent<SceneComponent>();
	anchorObject->setRootComponent(sceneComponentPtr);
	// TODO add a IGlSceneRenderable to the scene component to draw the anchor

	// Add spatial anchor component to the object
	AnchorComponentPtr anchorComponentPtr= anchorObject->addComponent<AnchorComponent>();
	anchorComponentPtr->setSpatialAnchor(anchorInfo);
	m_anchorComponents.insert({anchorInfo.anchor_id, anchorComponentPtr});

	// TODO: Add a collider component 

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

AnchorObjectSystemConfigConstPtr AnchorObjectSystem::getAnchorConfigConst() const
{
	return App::getInstance()->getProfileConfig()->anchorConfig;
}

AnchorObjectSystemConfigPtr AnchorObjectSystem::getAnchorConfig()
{
	return std::const_pointer_cast<AnchorObjectSystemConfig>(getAnchorConfigConst());
}