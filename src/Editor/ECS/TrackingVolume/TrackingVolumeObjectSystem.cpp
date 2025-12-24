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
#include "ProjectConfig.h"
#include "ProjectManager.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

// -- TrackingVolumeObjectSystemConfig -----
const std::string TrackingVolumeObjectSystemConfig::k_markerTrackingVolumeListPropertyId= "marker_tracker_volume_ids";
const std::string TrackingVolumeObjectSystemConfig::k_vrTrackingVolumeListPropertyId= "vr_tracker_volume_ids";

configuru::Config TrackingVolumeObjectSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	std::vector<configuru::Config> markerSystemsConfigs;
	for (MarkerTrackingVolumeDefinitionPtr systemDefinition : m_markerTrackingVolumeList)
	{
		markerSystemsConfigs.push_back(systemDefinition->writeToJSON());
	}
	pt.insert_or_assign(std::string("markerTrackingVolumes"), markerSystemsConfigs);

	std::vector<configuru::Config> vrSystemsConfigs;
	for (VRTrackingVolumeDefinitionPtr systemDefinition : m_vrTrackingVolumeList)
	{
		vrSystemsConfigs.push_back(systemDefinition->writeToJSON());
	}
	pt.insert_or_assign(std::string("vrTrackingVolumes"), vrSystemsConfigs);

	pt["nextTrackingVolumeId"] = m_nextTrackingVolumeId;

	return pt;
}

void TrackingVolumeObjectSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_nextTrackingVolumeId = pt.get_or<int>("nextTrackingVolumeId", m_nextTrackingVolumeId);

	m_markerTrackingVolumeList.clear();
	if (pt.has_key("markerTrackingVolumes"))
	{
		for (const configuru::Config& trackingVolumeConfig : pt["markerTrackingVolumes"].as_array())
		{
			MarkerTrackingVolumeDefinitionPtr trackingVolumeDefinitionPtr = 
				std::make_shared<MarkerTrackingVolumeDefinition>();

			trackingVolumeDefinitionPtr->readFromJSON(trackingVolumeConfig);
			m_markerTrackingVolumeList.push_back(trackingVolumeDefinitionPtr);

			addChildConfig(trackingVolumeDefinitionPtr);
		}
	}

	m_vrTrackingVolumeList.clear();
	if (pt.has_key("vrTrackingVolumes"))
	{
		for (const configuru::Config& trackingVolumeConfig : pt["vrTrackingVolumes"].as_array())
		{
			VRTrackingVolumeDefinitionPtr trackingVolumeDefinitionPtr =
				std::make_shared<VRTrackingVolumeDefinition>();

			trackingVolumeDefinitionPtr->readFromJSON(trackingVolumeConfig);
			m_vrTrackingVolumeList.push_back(trackingVolumeDefinitionPtr);

			addChildConfig(trackingVolumeDefinitionPtr);
		}
	}
}

bool TrackingVolumeObjectSystemConfig::canAddTrackingVolumeType(eTrackingVolumeType systemType) const
{
	switch (systemType)
	{
		case eTrackingVolumeType::vr:
			return m_vrTrackingVolumeList.size() == 0;
		case eTrackingVolumeType::marker:
			return true;
	}

	return false;
}

TrackingVolumeDefinitionConstPtr TrackingVolumeObjectSystemConfig::getTrackingVolumeDefinitionConst(
	MikanTrackingVolumeID systemId) const
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
	MikanTrackingVolumeID systemId)
{
	return 
		std::const_pointer_cast<TrackingVolumeDefinition>(
			getTrackingVolumeDefinitionConst(systemId));
}

eTrackingVolumeType TrackingVolumeObjectSystemConfig::getTrackingVolumeType(MikanTrackingVolumeID systemId) const
{
	TrackingVolumeDefinitionConstPtr trackingVolume= getTrackingVolumeDefinitionConst(systemId);
	if (trackingVolume)
	{
		return trackingVolume->getTrackingVolumeType();
	}

	return eTrackingVolumeType::INVALID;
}

bool TrackingVolumeObjectSystemConfig::removeTrackingVolume(MikanTrackingVolumeID systemId)
{
	switch (getTrackingVolumeType(systemId))
	{
		case eTrackingVolumeType::marker:
			return removeMarkerTrackingVolume(systemId);
			break;
		case eTrackingVolumeType::vr:
			return removeVRTrackingVolume(systemId);
			break;
	}

	return false;
}

MarkerTrackingVolumeDefinitionConstPtr TrackingVolumeObjectSystemConfig::getMarkerTrackingVolumeDefinitionConst(MikanTrackingVolumeID systemId) const
{
	auto iter = std::find_if(
		m_markerTrackingVolumeList.begin(), m_markerTrackingVolumeList.end(),
		[systemId](MarkerTrackingVolumeDefinitionPtr configPtr) {
			return configPtr->getTrackingVolumeId() == systemId;
		});

	if (iter != m_markerTrackingVolumeList.end())
	{
		return MarkerTrackingVolumeDefinitionConstPtr(*iter);
	}

	return MarkerTrackingVolumeDefinitionConstPtr();
}

MarkerTrackingVolumeDefinitionPtr TrackingVolumeObjectSystemConfig::getMarkerTrackingVolumeDefinition(MikanTrackingVolumeID systemId)
{
	return std::const_pointer_cast<MarkerTrackingVolumeDefinition>(getMarkerTrackingVolumeDefinitionConst(systemId));
}

MikanTrackingVolumeID TrackingVolumeObjectSystemConfig::addMarkerTrakingSystem()
{
	if (!canAddTrackingVolumeType(eTrackingVolumeType::marker))
		return INVALID_MIKAN_ID;

	const std::string systemName = StringUtils::stringify("Marker System ", m_nextTrackingVolumeId);
	MarkerTrackingVolumeDefinitionPtr configPtr = 
		std::make_shared<MarkerTrackingVolumeDefinition>(
			m_nextTrackingVolumeId, systemName);
	addChildConfig(configPtr);
	m_nextTrackingVolumeId++;

	m_markerTrackingVolumeList.push_back(configPtr);
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_markerTrackingVolumeListPropertyId));

	return configPtr->getTrackingVolumeId();
}

bool TrackingVolumeObjectSystemConfig::removeMarkerTrackingVolume(MikanTrackingVolumeID systemId)
{
	auto it = std::find_if(
		m_markerTrackingVolumeList.begin(), m_markerTrackingVolumeList.end(),
		[systemId](MarkerTrackingVolumeDefinitionPtr configPtr) {
			return configPtr->getTrackingVolumeId() == systemId;
		});

	if (it != m_markerTrackingVolumeList.end())
	{
		removeChildConfig(*it);

		m_markerTrackingVolumeList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_markerTrackingVolumeListPropertyId));

		return true;
	}

	return false;
}

VRTrackingVolumeDefinitionConstPtr TrackingVolumeObjectSystemConfig::getVRTrackingVolumeDefinitionConst(MikanTrackingVolumeID systemId) const
{
	auto iter = std::find_if(
		m_vrTrackingVolumeList.begin(), m_vrTrackingVolumeList.end(),
		[systemId](VRTrackingVolumeDefinitionPtr configPtr) {
			return configPtr->getTrackingVolumeId() == systemId;
		});

	if (iter != m_vrTrackingVolumeList.end())
	{
		return VRTrackingVolumeDefinitionConstPtr(*iter);
	}

	return VRTrackingVolumeDefinitionConstPtr();
}

VRTrackingVolumeDefinitionPtr TrackingVolumeObjectSystemConfig::getVRTrackingVolumeDefinition(MikanTrackingVolumeID systemId)
{
	return std::const_pointer_cast<VRTrackingVolumeDefinition>(getVRTrackingVolumeDefinitionConst(systemId));
}

MikanTrackingVolumeID TrackingVolumeObjectSystemConfig::addVRTrackingVolume(
	eTrackingRuntime trackingRuntime)
{
	if (!canAddTrackingVolumeType(eTrackingVolumeType::vr))
		return INVALID_MIKAN_ID;

	const std::string volumeName = StringUtils::stringify("VR System ", m_nextTrackingVolumeId);
	VRTrackingVolumeDefinitionPtr configPtr =
		std::make_shared<VRTrackingVolumeDefinition>(
			trackingRuntime,
			m_nextTrackingVolumeId, 
			volumeName);
	addChildConfig(configPtr);	
	m_nextTrackingVolumeId++;

	m_vrTrackingVolumeList.push_back(configPtr);
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_vrTrackingVolumeListPropertyId));

	return configPtr->getTrackingVolumeId();
}

bool TrackingVolumeObjectSystemConfig::removeVRTrackingVolume(MikanTrackingVolumeID systemId)
{
	auto it = std::find_if(
		m_vrTrackingVolumeList.begin(), m_vrTrackingVolumeList.end(),
		[systemId](VRTrackingVolumeDefinitionPtr configPtr) {
			return configPtr->getTrackingVolumeId() == systemId;
		});

	if (it != m_vrTrackingVolumeList.end())
	{
		removeChildConfig(*it);

		m_vrTrackingVolumeList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_vrTrackingVolumeListPropertyId));

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
	return getProjectConfig()->trackingVolumeSystemConfig;
}

TrackingVolumeObjectSystemConfigPtr TrackingVolumeObjectSystem::getTrackingVolumeSystemConfig()
{
	return getProjectConfig()->trackingVolumeSystemConfig;
}

TrackingVolumeIdList TrackingVolumeObjectSystem::getTrackingVolumeIdList() const
{
	TrackingVolumeIdList trackingVolumeIdList;
	for (const auto& it : m_trackingVolumeComponents)
	{
		trackingVolumeIdList.push_back(it.first);
	}

	return trackingVolumeIdList;
}

TrackingVolumeComponentPtr TrackingVolumeObjectSystem::getTrackingVolumeById(MikanTrackingVolumeID trackingVolumeId) const
{
	auto it = m_trackingVolumeComponents.find(trackingVolumeId);
	if (it != m_trackingVolumeComponents.end())
	{
		return it->second.lock();
	}

	return TrackingVolumeComponentPtr();
}

MarkerTrackingVolumeComponentPtr TrackingVolumeObjectSystem::getMarkerTrackingVolumeById(MikanTrackingVolumeID trackingVolumeId) const
{
	TrackingVolumeComponentPtr trackingVolume = getTrackingVolumeById(trackingVolumeId);
	return std::dynamic_pointer_cast<MarkerTrackingVolumeComponent>(trackingVolume);
}

VRTrackingVolumeComponentPtr TrackingVolumeObjectSystem::getVRTrackingVolumeById(MikanTrackingVolumeID trackingVolumeId) const
{
	TrackingVolumeComponentPtr trackingVolume = getTrackingVolumeById(trackingVolumeId);
	return std::dynamic_pointer_cast<VRTrackingVolumeComponent>(trackingVolume);
}

MarkerTrackingVolumeComponentPtr TrackingVolumeObjectSystem::addNewMarkerTrackingVolume()
{
	TrackingVolumeObjectSystemConfigPtr config = getTrackingVolumeSystemConfig();
	if (config == nullptr)
		return MarkerTrackingVolumeComponentPtr();

	MikanTrackingVolumeID volumeId = config->addMarkerTrakingSystem();
	if (volumeId == INVALID_MIKAN_ID)
		return MarkerTrackingVolumeComponentPtr();

	MarkerTrackingVolumeDefinitionPtr markerDef = config->getMarkerTrackingVolumeDefinition(volumeId);
	TrackingVolumeComponentPtr trackingVolume = createTrackingVolumeObject(std::static_pointer_cast<TrackingVolumeDefinition>(markerDef));

	return std::dynamic_pointer_cast<MarkerTrackingVolumeComponent>(trackingVolume);
}

VRTrackingVolumeComponentPtr TrackingVolumeObjectSystem::addNewVRTrackingVolume(eTrackingRuntime trackingRuntime)
{
	TrackingVolumeObjectSystemConfigPtr config = getTrackingVolumeSystemConfig();
	if (config == nullptr)
		return VRTrackingVolumeComponentPtr();

	MikanTrackingVolumeID volumeId = config->addVRTrackingVolume(trackingRuntime);
	if (volumeId == INVALID_MIKAN_ID)
		return VRTrackingVolumeComponentPtr();

	VRTrackingVolumeDefinitionPtr vrDef = config->getVRTrackingVolumeDefinition(volumeId);
	TrackingVolumeComponentPtr trackingVolume = createTrackingVolumeObject(std::static_pointer_cast<TrackingVolumeDefinition>(vrDef));

	return std::dynamic_pointer_cast<VRTrackingVolumeComponent>(trackingVolume);
}

bool TrackingVolumeObjectSystem::removeTrackingVolume(MikanTrackingVolumeID trackingVolumeId)
{
	TrackingVolumeObjectSystemConfigPtr config = getTrackingVolumeSystemConfig();
	if (config == nullptr)
		return false;

	// Remove the component first
	disposeTrackingVolumeObject(trackingVolumeId);

	// Then remove from config
	return config->removeTrackingVolume(trackingVolumeId);
}

TrackingVolumeComponentPtr TrackingVolumeObjectSystem::createTrackingVolumeObject(TrackingVolumeDefinitionPtr trackingVolumeConfig)
{
	MikanTrackingVolumeID trackingVolumeId = trackingVolumeConfig->getTrackingVolumeId();

	MikanObjectPtr mikanObject = newObject();
	mikanObject->setName(trackingVolumeConfig->getComponentName());

	// Create appropriate component type based on definition type
	TrackingVolumeComponentPtr componentPtr;
	switch (trackingVolumeConfig->getTrackingVolumeType())
	{
		case eTrackingVolumeType::marker:
		{
			MarkerTrackingVolumeDefinitionPtr componentDefinition =
				std::static_pointer_cast<MarkerTrackingVolumeDefinition>(trackingVolumeConfig);
			componentPtr = mikanObject->addComponent<MarkerTrackingVolumeComponent>();
			componentPtr->setDefinition(componentDefinition);

			break;
		}
		case eTrackingVolumeType::vr:
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

void TrackingVolumeObjectSystem::disposeTrackingVolumeObject(MikanTrackingVolumeID trackingVolumeId)
{
	auto it = m_trackingVolumeComponents.find(trackingVolumeId);
	if (it != m_trackingVolumeComponents.end())
	{
		TrackingVolumeComponentPtr componentPtr = it->second.lock();

		// Remove from component list
		m_trackingVolumeComponents.erase(it);

		// Free the corresponding object
		deleteObject(componentPtr->getOwnerObject());
	}
}
