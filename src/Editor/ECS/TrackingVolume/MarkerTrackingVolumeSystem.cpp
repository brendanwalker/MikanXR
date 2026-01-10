#include "MarkerTrackingVolumeSystem.h"
#include "Logger.h"
#include "MarkerTrackingVolumeComponent.h"
#include "MikanObject.h"
#include "MikanPropertyDatabase.h"
#include "ProjectConfig.h"
#include "ProjectManager.h"
#include "StringUtils.h"

// -- MarkerTrackingVolumeSystemConfig -----
const std::string MarkerTrackingVolumeSystemDefinition::k_markerTrackingVolumeListPropertyId = "marker_tracking_volume_ids";

configuru::Config MarkerTrackingVolumeSystemDefinition::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	std::vector<configuru::Config> markerVolumeConfigs;
	for (MarkerTrackingVolumeDefinitionPtr volumeDefinition : m_markerTrackingVolumeList)
	{
		markerVolumeConfigs.push_back(volumeDefinition->writeToJSON());
	}
	pt.insert_or_assign(std::string("markerTrackingVolumes"), markerVolumeConfigs);

	pt["nextTrackingVolumeId"] = m_nextTrackingVolumeId;

	return pt;
}

void MarkerTrackingVolumeSystemDefinition::readFromJSON(const configuru::Config& pt)
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
}

MarkerTrackingVolumeDefinitionConstPtr MarkerTrackingVolumeSystemDefinition::getMarkerTrackingVolumeDefinitionConst(
	MikanTrackingVolumeID trackingVolumeId) const
{
	auto iter = std::find_if(
		m_markerTrackingVolumeList.begin(), m_markerTrackingVolumeList.end(),
		[trackingVolumeId](MarkerTrackingVolumeDefinitionPtr configPtr) {
			return configPtr->getTrackingVolumeId() == trackingVolumeId;
		});

	if (iter != m_markerTrackingVolumeList.end())
	{
		return MarkerTrackingVolumeDefinitionConstPtr(*iter);
	}

	return MarkerTrackingVolumeDefinitionConstPtr();
}

MarkerTrackingVolumeDefinitionPtr MarkerTrackingVolumeSystemDefinition::getMarkerTrackingVolumeDefinition(
	MikanTrackingVolumeID trackingVolumeId)
{
	return std::const_pointer_cast<MarkerTrackingVolumeDefinition>(
		getMarkerTrackingVolumeDefinitionConst(trackingVolumeId));
}

MikanTrackingVolumeID MarkerTrackingVolumeSystemDefinition::addMarkerTrackingVolume()
{
	const std::string volumeName = StringUtils::stringify("Marker System ", m_nextTrackingVolumeId);
	MarkerTrackingVolumeDefinitionPtr configPtr =
		std::make_shared<MarkerTrackingVolumeDefinition>(
			m_nextTrackingVolumeId, volumeName);
	addChildConfig(configPtr);
	m_nextTrackingVolumeId++;

	m_markerTrackingVolumeList.push_back(configPtr);
	notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_markerTrackingVolumeListPropertyId));

	return configPtr->getTrackingVolumeId();
}

bool MarkerTrackingVolumeSystemDefinition::removeMarkerTrackingVolume(MikanTrackingVolumeID trackingVolumeId)
{
	auto it = std::find_if(
		m_markerTrackingVolumeList.begin(), m_markerTrackingVolumeList.end(),
		[trackingVolumeId](MarkerTrackingVolumeDefinitionPtr configPtr) {
			return configPtr->getTrackingVolumeId() == trackingVolumeId;
		});

	if (it != m_markerTrackingVolumeList.end())
	{
		removeChildConfig(*it);

		m_markerTrackingVolumeList.erase(it);
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_markerTrackingVolumeListPropertyId));

		return true;
	}

	return false;
}

// -- MarkerTrackingVolumeSystem -----
bool MarkerTrackingVolumeSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
	if (!MikanObjectSystem::init(definitionPtr))
		return false;

	MarkerTrackingVolumeSystemDefinitionPtr config = getMarkerTrackingVolumeSystemConfig();
	if (config == nullptr)
		return false;

	// Create tracking volume components for all existing definitions
	for (MarkerTrackingVolumeDefinitionPtr markerDef : config->getMarkerTrackingVolumeList())
	{
		createMarkerTrackingVolumeObject(markerDef);
	}

	return true;
}

void MarkerTrackingVolumeSystem::dispose()
{
	// Clean up all tracking volume components
	m_markerTrackingVolumeComponents.clear();

	MikanObjectSystem::dispose();
}

MikanComponentPtr MarkerTrackingVolumeSystem::getComponentById(int componentId) const
{
	return getMarkerTrackingVolumeById(componentId);
}

MarkerTrackingVolumeSystemDefinitionConstPtr MarkerTrackingVolumeSystem::getMarkerTrackingVolumeSystemConfigConst() const
{
	return std::static_pointer_cast<const MarkerTrackingVolumeSystemDefinition>(getDefinitionConst());
}

MarkerTrackingVolumeSystemDefinitionPtr MarkerTrackingVolumeSystem::getMarkerTrackingVolumeSystemConfig()
{
	return std::static_pointer_cast<MarkerTrackingVolumeSystemDefinition>(getDefinition());
}

MarkerTrackingVolumeIdList MarkerTrackingVolumeSystem::getTrackingVolumeIdList() const
{
	MarkerTrackingVolumeIdList trackingVolumeIdList;
	for (const auto& it : m_markerTrackingVolumeComponents)
	{
		trackingVolumeIdList.push_back(it.first);
	}

	return trackingVolumeIdList;
}

MarkerTrackingVolumeComponentPtr MarkerTrackingVolumeSystem::getMarkerTrackingVolumeById(
	MikanTrackingVolumeID trackingVolumeId) const
{
	auto it = m_markerTrackingVolumeComponents.find(trackingVolumeId);
	if (it != m_markerTrackingVolumeComponents.end())
	{
		return it->second.lock();
	}

	return MarkerTrackingVolumeComponentPtr();
}

MarkerTrackingVolumeComponentPtr MarkerTrackingVolumeSystem::addNewMarkerTrackingVolume()
{
	MarkerTrackingVolumeSystemDefinitionPtr config = getMarkerTrackingVolumeSystemConfig();
	if (config == nullptr)
		return MarkerTrackingVolumeComponentPtr();

	MikanTrackingVolumeID volumeId = config->addMarkerTrackingVolume();
	if (volumeId == INVALID_MIKAN_ID)
		return MarkerTrackingVolumeComponentPtr();

	MarkerTrackingVolumeDefinitionPtr markerDef = config->getMarkerTrackingVolumeDefinition(volumeId);
	MarkerTrackingVolumeComponentPtr trackingVolume = createMarkerTrackingVolumeObject(markerDef);

	return trackingVolume;
}

bool MarkerTrackingVolumeSystem::removeMarkerTrackingVolume(MikanTrackingVolumeID trackingVolumeId)
{
	MarkerTrackingVolumeSystemDefinitionPtr config = getMarkerTrackingVolumeSystemConfig();
	if (config == nullptr)
		return false;

	// Remove the component first
	disposeMarkerTrackingVolumeObject(trackingVolumeId);

	// Then remove from config
	return config->removeMarkerTrackingVolume(trackingVolumeId);
}

MarkerTrackingVolumeComponentPtr MarkerTrackingVolumeSystem::createMarkerTrackingVolumeObject(
	MarkerTrackingVolumeDefinitionPtr trackingVolumeConfig)
{
	MikanTrackingVolumeID trackingVolumeId = trackingVolumeConfig->getTrackingVolumeId();

	MikanObjectPtr mikanObject = newObject();
	mikanObject->setName(trackingVolumeConfig->getComponentName());

	// Create marker tracking volume component
	MarkerTrackingVolumeComponentPtr componentPtr = mikanObject->addComponent<MarkerTrackingVolumeComponent>();
	componentPtr->setDefinition(trackingVolumeConfig);

	if (componentPtr != nullptr)
	{
		m_markerTrackingVolumeComponents.insert({ trackingVolumeId, componentPtr });

		// Init the object once all components are added
		mikanObject->init();
	}

	return componentPtr;
}

void MarkerTrackingVolumeSystem::disposeMarkerTrackingVolumeObject(MikanTrackingVolumeID trackingVolumeId)
{
	auto it = m_markerTrackingVolumeComponents.find(trackingVolumeId);
	if (it != m_markerTrackingVolumeComponents.end())
	{
		MarkerTrackingVolumeComponentPtr componentPtr = it->second.lock();

		// Remove from component list
		m_markerTrackingVolumeComponents.erase(it);

		// Free the corresponding object
		deleteObject(componentPtr->getOwnerObject());
	}
}

void MarkerTrackingVolumeSystem::registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase)
{
	propertyDatabase->registerPropertiesForSystem<MarkerTrackingVolumeSystem>();
	propertyDatabase->registerPropertiesForComponent<MarkerTrackingVolumeSystem, MarkerTrackingVolumeComponent>();
}
