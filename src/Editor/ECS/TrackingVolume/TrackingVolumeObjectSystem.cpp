#include "TrackingVolumeObjectSystem.h"
#include "App.h"
#include "Logger.h"
#include "TrackingVolumeComponent.h"
#include "MarkerTrackingVolumeComponent.h"
#include "VRTrackingVolumeComponent.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ObjectSystemManager.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

// -- TrackingSystemsConfig -----
const std::string TrackingVolumeObjectSystemConfig::k_markerTrackingSystemListPropertyId= "markerTrackingSystemList";
const std::string TrackingVolumeObjectSystemConfig::k_vrTrackingSystemListPropertyId= "vrTrackingSystemList";

TrackingVolumeObjectSystemConfigPtr TrackingVolumeObjectSystemConfig::getSystemConfig()
{
	return App::getInstance()->getProfileConfig()->trackingVolumeSystemConfig;
}

configuru::Config TrackingVolumeObjectSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	std::vector<configuru::Config> markerSystemsConfigs;
	for (MarkerTrackingVolumeDefinitionPtr systemDefinition : m_markerTrackingVolumeList)
	{
		markerSystemsConfigs.push_back(systemDefinition->writeToJSON());
	}
	pt.insert_or_assign(std::string("markerTrackingSystems"), markerSystemsConfigs);

	std::vector<configuru::Config> vrSystemsConfigs;
	for (VRTrackingVolumeDefinitionPtr systemDefinition : m_vrTrackingVolumeList)
	{
		vrSystemsConfigs.push_back(systemDefinition->writeToJSON());
	}
	pt.insert_or_assign(std::string("vrTrackingSystems"), vrSystemsConfigs);

	pt["nextTrackingSystemId"] = m_nextTrackingSystemId;

	return pt;
}

void TrackingVolumeObjectSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_nextTrackingSystemId = pt.get_or<int>("nextTrackingSystemId", m_nextTrackingSystemId);

	m_markerTrackingVolumeList.clear();
	if (pt.has_key("markerTrackingSystems"))
	{
		for (const configuru::Config& trackingSystemConfig : pt["markerTrackingSystems"].as_array())
		{
			MarkerTrackingVolumeDefinitionPtr trackingSystemDefinitionPtr = 
				std::make_shared<MarkerTrackingVolumeDefinition>();

			trackingSystemDefinitionPtr->readFromJSON(trackingSystemConfig);
			m_markerTrackingVolumeList.push_back(trackingSystemDefinitionPtr);

			addChildConfig(trackingSystemDefinitionPtr);
		}
	}

	m_vrTrackingVolumeList.clear();
	if (pt.has_key("vrTrackingSystems"))
	{
		for (const configuru::Config& trackingSystemConfig : pt["vrTrackingSystems"].as_array())
		{
			VRTrackingVolumeDefinitionPtr trackingSystemDefinitionPtr =
				std::make_shared<VRTrackingVolumeDefinition>();

			trackingSystemDefinitionPtr->readFromJSON(trackingSystemConfig);
			m_vrTrackingVolumeList.push_back(trackingSystemDefinitionPtr);

			addChildConfig(trackingSystemDefinitionPtr);
		}
	}
}

bool TrackingVolumeObjectSystemConfig::canAddTrackingSystemType(eTrackingSystemType systemType) const
{
	switch (systemType)
	{
		case eTrackingSystemType::vr:
			return m_vrTrackingVolumeList.size() == 0;
		case eTrackingSystemType::marker:
			return true;
	}

	return false;
}

TrackingVolumeDefinitionConstPtr TrackingVolumeObjectSystemConfig::getTrackingVolumeDefinitionConst(
	MikanTrackingSystemID systemId) const
{
	MarkerTrackingVolumeDefinitionConstPtr markerSystemPtr = 
		getMarkerTrackingVolumeDefinitionConst(systemId);
	if (markerSystemPtr)
	{
		return markerSystemPtr;
	}

	VRTrackingVolumeDefinitionConstPtr vrSystemPtr = 
		getVRTrackingVolumeDefinitionConst(systemId);
	if (vrSystemPtr)
	{
		return vrSystemPtr;
	}

	return TrackingVolumeDefinitionPtr();
}

TrackingVolumeDefinitionPtr TrackingVolumeObjectSystemConfig::getTrackingVolumeDefinition(
	MikanTrackingSystemID systemId)
{
	return 
		std::const_pointer_cast<TrackingVolumeDefinition>(
			getTrackingVolumeDefinitionConst(systemId));
}

eTrackingSystemType TrackingVolumeObjectSystemConfig::getTrackingSystemType(MikanTrackingSystemID systemId) const
{
	TrackingVolumeDefinitionConstPtr trackingSystem= getTrackingVolumeDefinitionConst(systemId);
	if (trackingSystem)
	{
		return trackingSystem->getTrackingSystemType();
	}

	return eTrackingSystemType::INVALID;
}

bool TrackingVolumeObjectSystemConfig::removeTrackingSystem(MikanTrackingSystemID systemId)
{
	switch (getTrackingSystemType(systemId))
	{
		case eTrackingSystemType::marker:
			return removeMarkerTrackingSystem(systemId);
			break;
		case eTrackingSystemType::vr:
			return removeVRTrackingSystem(systemId);
			break;
	}

	return false;
}

MarkerTrackingVolumeDefinitionConstPtr TrackingVolumeObjectSystemConfig::getMarkerTrackingVolumeDefinitionConst(MikanTrackingSystemID systemId) const
{
	auto iter = std::find_if(
		m_markerTrackingVolumeList.begin(), m_markerTrackingVolumeList.end(),
		[systemId](MarkerTrackingVolumeDefinitionPtr configPtr) {
			return configPtr->getTrackingSystemId() == systemId;
		});

	if (iter != m_markerTrackingVolumeList.end())
	{
		return MarkerTrackingVolumeDefinitionConstPtr(*iter);
	}

	return MarkerTrackingVolumeDefinitionConstPtr();
}

MarkerTrackingVolumeDefinitionPtr TrackingVolumeObjectSystemConfig::getMarkerTrackingVolumeDefinition(MikanTrackingSystemID systemId)
{
	return std::const_pointer_cast<MarkerTrackingVolumeDefinition>(getMarkerTrackingVolumeDefinitionConst(systemId));
}

MikanTrackingSystemID TrackingVolumeObjectSystemConfig::addMarkerTrakingSystem()
{
	if (!canAddTrackingSystemType(eTrackingSystemType::marker))
		return INVALID_MIKAN_ID;

	const std::string systemName = StringUtils::stringify("Marker System ", m_nextTrackingSystemId);
	MarkerTrackingVolumeDefinitionPtr configPtr = 
		std::make_shared<MarkerTrackingVolumeDefinition>(
			m_nextTrackingSystemId, systemName);
	addChildConfig(configPtr);
	m_nextTrackingSystemId++;

	m_markerTrackingVolumeList.push_back(configPtr);
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_markerTrackingSystemListPropertyId));

	return configPtr->getTrackingSystemId();
}

bool TrackingVolumeObjectSystemConfig::removeMarkerTrackingSystem(MikanTrackingSystemID systemId)
{
	auto it = std::find_if(
		m_markerTrackingVolumeList.begin(), m_markerTrackingVolumeList.end(),
		[systemId](MarkerTrackingVolumeDefinitionPtr configPtr) {
			return configPtr->getTrackingSystemId() == systemId;
		});

	if (it != m_markerTrackingVolumeList.end())
	{
		removeChildConfig(*it);

		m_markerTrackingVolumeList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_markerTrackingSystemListPropertyId));

		return true;
	}

	return false;
}

VRTrackingVolumeDefinitionConstPtr TrackingVolumeObjectSystemConfig::getVRTrackingVolumeDefinitionConst(MikanTrackingSystemID systemId) const
{
	auto iter = std::find_if(
		m_vrTrackingVolumeList.begin(), m_vrTrackingVolumeList.end(),
		[systemId](VRTrackingVolumeDefinitionPtr configPtr) {
			return configPtr->getTrackingSystemId() == systemId;
		});

	if (iter != m_vrTrackingVolumeList.end())
	{
		return VRTrackingVolumeDefinitionConstPtr(*iter);
	}

	return VRTrackingVolumeDefinitionConstPtr();
}

VRTrackingVolumeDefinitionPtr TrackingVolumeObjectSystemConfig::getVRTrackingVolumeDefinition(MikanTrackingSystemID systemId)
{
	return std::const_pointer_cast<VRTrackingVolumeDefinition>(getVRTrackingVolumeDefinitionConst(systemId));
}

MikanTrackingSystemID TrackingVolumeObjectSystemConfig::addVRTrackingSystem(
	eTrackingRuntime trackingRuntime)
{
	if (!canAddTrackingSystemType(eTrackingSystemType::vr))
		return INVALID_MIKAN_ID;

	const std::string systemName = StringUtils::stringify("VR System ", m_nextTrackingSystemId);
	VRTrackingVolumeDefinitionPtr configPtr =
		std::make_shared<VRTrackingVolumeDefinition>(
			trackingRuntime,
			m_nextTrackingSystemId, 
			systemName);
	addChildConfig(configPtr);	
	m_nextTrackingSystemId++;

	m_vrTrackingVolumeList.push_back(configPtr);
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_vrTrackingSystemListPropertyId));

	return configPtr->getTrackingSystemId();
}

bool TrackingVolumeObjectSystemConfig::removeVRTrackingSystem(MikanTrackingSystemID systemId)
{
	auto it = std::find_if(
		m_vrTrackingVolumeList.begin(), m_vrTrackingVolumeList.end(),
		[systemId](VRTrackingVolumeDefinitionPtr configPtr) {
			return configPtr->getTrackingSystemId() == systemId;
		});

	if (it != m_vrTrackingVolumeList.end())
	{
		removeChildConfig(*it);

		m_vrTrackingVolumeList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_vrTrackingSystemListPropertyId));

		return true;
	}

	return false;
}

// -- TrackingVolumeObjectSystem -----
bool TrackingVolumeObjectSystem::init()
{
	TrackingVolumeObjectSystemConfigPtr config = getTrackingVolumeSystemConfig();
	if (config == nullptr)
		return false;

	// Create tracking volume components for all existing definitions
	for (MarkerTrackingVolumeDefinitionPtr markerDef : config->getMarkerTrackingVolumeList())
	{
		createTrackingVolumeObject(std::static_pointer_cast<TrackingVolumeDefinition>(markerDef));
	}

	for (VRTrackingVolumeDefinitionPtr vrDef : config->getVRTrackingVolumeList())
	{
		createTrackingVolumeObject(std::static_pointer_cast<TrackingVolumeDefinition>(vrDef));
	}

	return MikanObjectSystem::init();
}

void TrackingVolumeObjectSystem::dispose()
{
	// Clean up all tracking volume components
	m_trackingVolumeComponents.clear();

	MikanObjectSystem::dispose();
}

void TrackingVolumeObjectSystem::deleteObjectConfig(MikanObjectPtr objectPtr)
{
	// Implementation similar to TrackingMountObjectSystem
	MikanObjectSystem::deleteObjectConfig(objectPtr);
}

TrackingVolumeObjectSystemConfigConstPtr TrackingVolumeObjectSystem::getTrackingVolumeSystemConfigConst() const
{
	return App::getInstance()->getProfileConfig()->trackingVolumeSystemConfig;
}

TrackingVolumeObjectSystemConfigPtr TrackingVolumeObjectSystem::getTrackingVolumeSystemConfig()
{
	return App::getInstance()->getProfileConfig()->trackingVolumeSystemConfig;
}

TrackingVolumeComponentPtr TrackingVolumeObjectSystem::getTrackingVolumeById(MikanTrackingSystemID trackingSystemId) const
{
	auto it = m_trackingVolumeComponents.find(trackingSystemId);
	if (it != m_trackingVolumeComponents.end())
	{
		return it->second.lock();
	}

	return TrackingVolumeComponentPtr();
}

MarkerTrackingVolumeComponentPtr TrackingVolumeObjectSystem::getMarkerTrackingVolumeById(MikanTrackingSystemID trackingSystemId) const
{
	TrackingVolumeComponentPtr trackingVolume = getTrackingVolumeById(trackingSystemId);
	return std::dynamic_pointer_cast<MarkerTrackingVolumeComponent>(trackingVolume);
}

VRTrackingVolumeComponentPtr TrackingVolumeObjectSystem::getVRTrackingVolumeById(MikanTrackingSystemID trackingSystemId) const
{
	TrackingVolumeComponentPtr trackingVolume = getTrackingVolumeById(trackingSystemId);
	return std::dynamic_pointer_cast<VRTrackingVolumeComponent>(trackingVolume);
}

MarkerTrackingVolumeComponentPtr TrackingVolumeObjectSystem::addNewMarkerTrackingVolume()
{
	TrackingVolumeObjectSystemConfigPtr config = getTrackingVolumeSystemConfig();
	if (config == nullptr)
		return MarkerTrackingVolumeComponentPtr();

	MikanTrackingSystemID systemId = config->addMarkerTrakingSystem();
	if (systemId == INVALID_MIKAN_ID)
		return MarkerTrackingVolumeComponentPtr();

	MarkerTrackingVolumeDefinitionPtr markerDef = config->getMarkerTrackingVolumeDefinition(systemId);
	TrackingVolumeComponentPtr trackingVolume = createTrackingVolumeObject(std::static_pointer_cast<TrackingVolumeDefinition>(markerDef));

	return std::dynamic_pointer_cast<MarkerTrackingVolumeComponent>(trackingVolume);
}

VRTrackingVolumeComponentPtr TrackingVolumeObjectSystem::addNewVRTrackingVolume(eTrackingRuntime trackingRuntime)
{
	TrackingVolumeObjectSystemConfigPtr config = getTrackingVolumeSystemConfig();
	if (config == nullptr)
		return VRTrackingVolumeComponentPtr();

	MikanTrackingSystemID systemId = config->addVRTrackingSystem(trackingRuntime);
	if (systemId == INVALID_MIKAN_ID)
		return VRTrackingVolumeComponentPtr();

	VRTrackingVolumeDefinitionPtr vrDef = config->getVRTrackingVolumeDefinition(systemId);
	TrackingVolumeComponentPtr trackingVolume = createTrackingVolumeObject(std::static_pointer_cast<TrackingVolumeDefinition>(vrDef));

	return std::dynamic_pointer_cast<VRTrackingVolumeComponent>(trackingVolume);
}

bool TrackingVolumeObjectSystem::removeTrackingSystem(MikanTrackingSystemID trackingSystemId)
{
	TrackingVolumeObjectSystemConfigPtr config = getTrackingVolumeSystemConfig();
	if (config == nullptr)
		return false;

	// Remove the component first
	disposeTrackingVolumeObject(trackingSystemId);

	// Then remove from config
	return config->removeTrackingSystem(trackingSystemId);
}

TrackingVolumeComponentPtr TrackingVolumeObjectSystem::createTrackingVolumeObject(TrackingVolumeDefinitionPtr trackingVolumeConfig)
{
	MikanTrackingSystemID trackingVolumeId = trackingVolumeConfig->getTrackingSystemId();

	MikanObjectPtr mikanObject = newObject();
	mikanObject->setName(trackingVolumeConfig->getComponentName());

	// Create appropriate component type based on definition type
	TrackingVolumeComponentPtr componentPtr;
	switch (trackingVolumeConfig->getTrackingSystemType())
	{
		case eTrackingSystemType::marker:
		{
			MarkerTrackingVolumeDefinitionPtr componentDefinition =
				std::static_pointer_cast<MarkerTrackingVolumeDefinition>(trackingVolumeConfig);
			componentPtr = mikanObject->addComponent<MarkerTrackingVolumeComponent>();
			componentPtr->setDefinition(componentDefinition);

			break;
		}
		case eTrackingSystemType::vr:
		{
			VRTrackingVolumeDefinitionPtr componentDefinition =
				std::static_pointer_cast<VRTrackingVolumeDefinition>(trackingVolumeConfig);
			componentPtr = mikanObject->addComponent<VRTrackingVolumeComponent>();
			componentPtr->setDefinition(componentDefinition);
			break;
		}
	}

	if (componentPtr != nullptr)
	{
		m_trackingVolumeComponents.insert({ trackingVolumeId, componentPtr });

		// Init the object once all components are added
		mikanObject->init();
	}

	return componentPtr;
}

void TrackingVolumeObjectSystem::disposeTrackingVolumeObject(MikanTrackingSystemID trackingSystemId)
{
	auto it = m_trackingVolumeComponents.find(trackingSystemId);
	if (it != m_trackingVolumeComponents.end())
	{
		TrackingVolumeComponentPtr componentPtr = it->second.lock();

		// Remove from component list
		m_trackingVolumeComponents.erase(it);

		// Free the corresponding object
		deleteObject(componentPtr->getOwnerObject());
	}
}
