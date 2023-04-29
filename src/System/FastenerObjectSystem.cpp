#include "App.h"
#include "AnchorComponent.h"
#include "AnchorObjectSystem.h"
#include "FastenerObjectSystem.h"
#include "FastenerComponent.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "ProfileConfig.h"
#include "SceneComponent.h"
#include "StencilComponent.h"
#include "StencilObjectSystem.h"
#include "StringUtils.h"

// -- FastenerObjectSystemConfig -----
configuru::Config FastenerObjectSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["nextFastenerId"] = nextFastenerId;
	pt["debugRenderFasteners"] = debugRenderFasteners;

	std::vector<configuru::Config> FastenerConfigs;
	for (FastenerConfigPtr FastenerConfigPtr : spatialFastenerList)
	{
		FastenerConfigs.push_back(FastenerConfigPtr->writeToJSON());
	}
	pt.insert_or_assign(std::string("spatialFasteners"), FastenerConfigs);

	return pt;
}

void FastenerObjectSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	nextFastenerId = pt.get_or<int>("nextFastenerId", nextFastenerId);
	debugRenderFasteners = pt.get_or<bool>("debugRenderFasteners", debugRenderFasteners);

	// Read in the spatial fasteners
	spatialFastenerList.clear();
	if (pt.has_key("spatialFasteners"))
	{
		for (const configuru::Config& Fastener_pt : pt["spatialFasteners"].as_array())
		{
			FastenerConfigPtr FastenerConfigPtr = std::make_shared<FastenerConfig>();
			FastenerConfigPtr->readFromJSON(Fastener_pt);
			spatialFastenerList.push_back(FastenerConfigPtr);
		}
	}
}

bool FastenerObjectSystemConfig::canAddFastener() const
{
	return (spatialFastenerList.size() < MAX_MIKAN_SPATIAL_FASTENERS);
}

FastenerConfigPtr FastenerObjectSystemConfig::getSpatialFastenerConfig(MikanSpatialFastenerID fastenerId) const
{
	auto it = std::find_if(
		spatialFastenerList.begin(), spatialFastenerList.end(),
		[fastenerId](FastenerConfigPtr configPtr) {
		return configPtr->getFastenerId() == fastenerId;
	});

	if (it != spatialFastenerList.end())
	{
		return FastenerConfigPtr(*it);
	}

	return FastenerConfigPtr();
}

FastenerConfigPtr FastenerObjectSystemConfig::getSpatialFastenerConfigByName(const std::string& fastenerName) const
{
	auto it = std::find_if(
		spatialFastenerList.begin(), spatialFastenerList.end(),
		[fastenerName](FastenerConfigPtr configPtr) {
		return strncmp(configPtr->getFastenerInfo().fastener_name, fastenerName.c_str(), MAX_MIKAN_FASTENER_NAME_LEN) == 0;
	});

	if (it != spatialFastenerList.end())
	{
		return FastenerConfigPtr(*it);
	}

	return FastenerConfigPtr();
}


MikanSpatialFastenerID FastenerObjectSystemConfig::addNewFastener(const MikanSpatialFastenerInfo& fastenerInfo)
{
	if (!canAddFastener())
		return INVALID_MIKAN_ID;

	FastenerConfigPtr fastenerConfig = std::make_shared<FastenerConfig>(nextFastenerId, fastenerInfo);
	nextFastenerId++;

	spatialFastenerList.push_back(fastenerConfig);
	markDirty();

	return fastenerConfig->getFastenerId();
}

bool FastenerObjectSystemConfig::removeFastener(MikanSpatialFastenerID fastenerId)
{
	auto it = std::find_if(
		spatialFastenerList.begin(), spatialFastenerList.end(),
		[fastenerId](FastenerConfigPtr configPtr) {
		return configPtr->getFastenerId() == fastenerId;
	});

	if (it != spatialFastenerList.end() &&
		(*it)->getFastenerId() != originFastenerId)
	{
		spatialFastenerList.erase(it);
		markDirty();

		return true;
	}

	return false;
}

// -- FastenerObjectSystem -----
FastenerObjectSystemWeakPtr FastenerObjectSystem::s_fastenerObjectSystem;

FastenerObjectSystem::FastenerObjectSystem()
{
	s_fastenerObjectSystem = std::static_pointer_cast<FastenerObjectSystem>(shared_from_this());
}

FastenerObjectSystem::~FastenerObjectSystem()
{
	s_fastenerObjectSystem.reset();
}

void FastenerObjectSystem::init()
{
	MikanObjectSystem::init();

	FastenerObjectSystemConfigConstPtr FastenerConfig = getFastenerSystemConfigConst();
	for (FastenerConfigPtr fastenerConfig : FastenerConfig->spatialFastenerList)
	{
		createFastenerObject(fastenerConfig);
	}
}

void FastenerObjectSystem::dispose()
{
	m_fastenerComponents.clear();
	MikanObjectSystem::dispose();
}

FastenerComponentPtr FastenerObjectSystem::getSpatialFastenerById(MikanSpatialFastenerID fastenerId) const
{
	auto iter = m_fastenerComponents.find(fastenerId);
	if (iter != m_fastenerComponents.end())
	{
		return iter->second.lock();
	}

	return FastenerComponentPtr();
}

FastenerComponentPtr FastenerObjectSystem::getSpatialFastenerByName(const std::string& fastenerName) const
{
	for (auto it = m_fastenerComponents.begin(); it != m_fastenerComponents.end(); it++)
	{
		FastenerComponentPtr componentPtr = it->second.lock();

		if (componentPtr && componentPtr->getFastenerName() == fastenerName)
		{
			return componentPtr;
		}
	}

	return FastenerComponentPtr();
}

std::vector<MikanSpatialFastenerID> FastenerObjectSystem::getSpatialFastenersWithParent(
	const MikanFastenerParentType parentType,
	const MikanSpatialAnchorID parentObjectId) const
{
	std::vector<MikanSpatialFastenerID> result;

	for (auto it = m_fastenerComponents.begin(); it != m_fastenerComponents.end(); it++)
	{
		FastenerComponentPtr componentPtr = it->second.lock();

		if (componentPtr->getConfig()->getFastenerParentType() == parentType &&
			componentPtr->getConfig()->getParentObjectId() == parentObjectId)
		{
			result.push_back(componentPtr->getConfig()->getFastenerId());
		}
	}

	return result;
}

std::vector<MikanSpatialFastenerID> FastenerObjectSystem::getValidSpatialFastenerSnapTargets(
	const MikanSpatialFastenerID sourceFastenerId) const
{
	std::vector<MikanSpatialFastenerID> result;

	FastenerComponentPtr sourceFastener= getSpatialFastenerById(sourceFastenerId);
	if (sourceFastener != nullptr &&
		sourceFastener->getConfig()->getFastenerParentType() == MikanFastenerParentType_Stencil)
	{
		for (auto it = m_fastenerComponents.begin(); it != m_fastenerComponents.end(); it++)
		{
			FastenerComponentPtr otherFastenerPtr = it->second.lock();

			if (otherFastenerPtr->getConfig()->getFastenerId() != sourceFastenerId &&
				otherFastenerPtr->getConfig()->getFastenerParentType() == MikanFastenerParentType_SpatialAnchor)
			{
				result.push_back(otherFastenerPtr->getConfig()->getFastenerId());
			}
		}
	}

	return result;
}

FastenerComponentPtr FastenerObjectSystem::addNewFastener(const MikanSpatialFastenerInfo& fastenerInfo)
{
	FastenerObjectSystemConfigPtr fastenerSystemConfig = getFastenerSystemConfig();

	MikanSpatialFastenerID fastenerId = fastenerSystemConfig->addNewFastener(fastenerInfo);
	if (fastenerId != INVALID_MIKAN_ID)
	{
		FastenerConfigPtr fastenerConfig = fastenerSystemConfig->getSpatialFastenerConfig(fastenerId);
		assert(fastenerConfig != nullptr);

		return createFastenerObject(fastenerConfig);
	}

	return FastenerComponentPtr();
}

bool FastenerObjectSystem::removeFastener(MikanSpatialFastenerID FastenerId)
{
	getFastenerSystemConfig()->removeFastener(FastenerId);
	disposeFastenerObject(FastenerId);

	return false;
}

SceneComponentPtr FastenerObjectSystem::getFastenerParentSceneComponent(
	const MikanSpatialFastenerInfo& fastenerInfo)
{
	if (fastenerInfo.parent_object_id != INVALID_MIKAN_ID)
	{
		if (fastenerInfo.parent_object_type == MikanFastenerParentType_SpatialAnchor)
		{
			AnchorComponentPtr parentAnchor = 
				AnchorObjectSystem::getSystem()->getSpatialAnchorById(fastenerInfo.parent_object_id);

			if (parentAnchor)
			{
				return parentAnchor->getOwnerObject()->getRootComponent();
			}
		}

		// Attach to parent stencil, if any
		if (fastenerInfo.parent_object_type == MikanFastenerParentType_Stencil)
		{
			StencilComponentPtr parentStencil = 
				StencilObjectSystem::getSystem()->getStencilById(fastenerInfo.parent_object_id);

			if (parentStencil)
			{
				return parentStencil->getOwnerObject()->getRootComponent();
			}
		}
	}

	return SceneComponentPtr();
}

FastenerComponentPtr FastenerObjectSystem::createFastenerObject(FastenerConfigPtr fastenerConfig)
{
	MikanObjectPtr fastenerObject = newObject().lock();

	// Add a scene component to the Fastener
	SceneComponentPtr sceneComponentPtr = fastenerObject->addComponent<SceneComponent>();
	fastenerObject->setRootComponent(sceneComponentPtr);

	// Add spatial Fastener component to the object
	FastenerComponentPtr fastenerComponent = fastenerObject->addComponent<FastenerComponent>();
	fastenerComponent->setConfig(fastenerConfig);
	m_fastenerComponents.insert({fastenerConfig->getFastenerId(), fastenerComponent});

	// Attach to parent scene component
	const MikanSpatialFastenerInfo& fastenerInfo= fastenerConfig->getFastenerInfo();
	SceneComponentPtr parentSceneComponent= getFastenerParentSceneComponent(fastenerInfo);
	if (parentSceneComponent != nullptr)
	{
		sceneComponentPtr->attachToComponent(parentSceneComponent);
	}

	// Init the object once all components are added
	fastenerObject->init();

	return fastenerComponent;
}

void FastenerObjectSystem::disposeFastenerObject(MikanSpatialFastenerID fastenerId)
{
	auto it = m_fastenerComponents.find(fastenerId);
	if (it != m_fastenerComponents.end())
	{
		FastenerComponentPtr FastenerComponentPtr = it->second.lock();

		// Remove for component list
		m_fastenerComponents.erase(it);

		// Free the corresponding object
		deleteObject(FastenerComponentPtr->getOwnerObject());
	}
}

FastenerObjectSystemConfigConstPtr FastenerObjectSystem::getFastenerSystemConfigConst() const
{
	return App::getInstance()->getProfileConfig()->fastenerConfig;
}

FastenerObjectSystemConfigPtr FastenerObjectSystem::getFastenerSystemConfig()
{
	return std::const_pointer_cast<FastenerObjectSystemConfig>(getFastenerSystemConfigConst());
}