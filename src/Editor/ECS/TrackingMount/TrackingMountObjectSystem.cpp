#include "TrackingMountObjectSystem.h"
#include "App.h"
#include "Logger.h"
#include "TrackingMountComponent.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "MikanObject.h"
#include "MikanPropertyDatabase.h"
#include "ProjectConfig.h"
#include "ProjectManager.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

// -- TrackingMountObjectSystemConfig -----
const std::string TrackingMountObjectSystemDefinition::k_trackingMountListPropertyId = "trackingMounts";

configuru::Config TrackingMountObjectSystemDefinition::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["nextTrackingMountId"] = m_nextTrackingMountId;

	// TrackingMount settings
	std::vector<configuru::Config> trackingMountConfigs;
	for (TrackingMountDefinitionPtr trackingMountDefinitionPtr : m_trackingMountList)
	{
		trackingMountConfigs.push_back(trackingMountDefinitionPtr->writeToJSON());
	}
	pt.insert_or_assign(std::string("trackingMounts"), trackingMountConfigs);

	return pt;
}

void TrackingMountObjectSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_nextTrackingMountId = pt.get_or<MikanTrackingMountID>("nextTrackingMountId", m_nextTrackingMountId);

	// TrackingMount settings
	if (pt.has_key("trackingMounts"))
	{
		const configuru::Config& trackingMountConfigs = pt["trackingMounts"];

		if (trackingMountConfigs.is_array())
		{
			for (const auto& trackingMountConfig : trackingMountConfigs.as_array())
			{
				TrackingMountDefinitionPtr trackingMountDefinition = TrackingMountDefinitionPtr(new TrackingMountDefinition());
				trackingMountDefinition->readFromJSON(trackingMountConfig);

				m_trackingMountList.push_back(trackingMountDefinition);
				addChildConfig(trackingMountDefinition);
			}
		}
	}
}

TrackingMountDefinitionPtr TrackingMountObjectSystemDefinition::getTrackingMountConfig(MikanTrackingMountID trackingMountId) const
{
	for (auto trackingMountDefinitionPtr : m_trackingMountList)
	{
		if (trackingMountDefinitionPtr->getTrackingMountId() == trackingMountId)
		{
			return trackingMountDefinitionPtr;
		}
	}

	return TrackingMountDefinitionPtr();
}

TrackingMountDefinitionPtr TrackingMountObjectSystemDefinition::getTrackingMountConfigByName(const std::string& trackingMountName) const
{
	for (auto trackingMountDefinitionPtr : m_trackingMountList)
	{
		if (trackingMountDefinitionPtr->getComponentName() == trackingMountName)
		{
			return trackingMountDefinitionPtr;
		}
	}

	return TrackingMountDefinitionPtr();
}

MikanTrackingMountID TrackingMountObjectSystemDefinition::addNewTrackingMount()
{
	MikanTrackingMountID newTrackingMountId = m_nextTrackingMountId;
	const std::string trackingMountName = StringUtils::stringify("mount_", m_nextTrackingMountId);
	TrackingMountDefinitionPtr trackingMountDefinition =
		TrackingMountDefinitionPtr(new TrackingMountDefinition(newTrackingMountId, trackingMountName));

	m_trackingMountList.push_back(trackingMountDefinition);
	addChildConfig(trackingMountDefinition);
	++m_nextTrackingMountId;

	return newTrackingMountId;
}

bool TrackingMountObjectSystemDefinition::removeTrackingMountID(MikanTrackingMountID trackingMountId)
{
	auto it = std::find_if(
		m_trackingMountList.begin(), m_trackingMountList.end(),
		[trackingMountId](const TrackingMountDefinitionPtr trackingMountDefinition) {
			return trackingMountDefinition->getTrackingMountId() == trackingMountId;
		});

	if (it != m_trackingMountList.end())
	{
		removeChildConfig(*it);

		m_trackingMountList.erase(it);
		return true;
	}

	return false;
}

// -- TrackingMountObjectSystem ----
bool TrackingMountObjectSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
	if (MikanObjectSystem::init(definitionPtr))
	{
		TrackingMountObjectSystemDefinitionPtr config = getTrackingMountSystemConfig();

		// Create tracking mount objects from config
		for (TrackingMountDefinitionPtr trackingMountDefinition : config->getTrackingMountList())
		{
			createTrackingMountObject(trackingMountDefinition);
		}

		return true;
	}

	return false;
}

void TrackingMountObjectSystem::dispose()
{
	m_trackingMountComponents.clear();

	MikanObjectSystem::dispose();
}

MikanComponentPtr TrackingMountObjectSystem::getComponentById(int componentId) const
{
	return getTrackingMountById(componentId);
}

void TrackingMountObjectSystem::deleteObjectConfig(MikanObjectPtr objectPtr)
{
	MikanObjectSystem::deleteObjectConfig(objectPtr);
}

TrackingMountComponentPtr TrackingMountObjectSystem::getTrackingMountById(MikanTrackingMountID trackingMountId) const
{
	auto it = m_trackingMountComponents.find(trackingMountId);

	return (it != m_trackingMountComponents.end()) ? it->second.lock() : TrackingMountComponentPtr();
}

TrackingMountComponentPtr TrackingMountObjectSystem::getTrackingMountByName(const std::string& trackingMountName) const
{
	for (auto& pair : m_trackingMountComponents)
	{
		TrackingMountComponentPtr trackingMountComponent = pair.second.lock();

		if (trackingMountComponent)
		{
			TrackingMountDefinitionPtr trackingMountDefinition = trackingMountComponent->getTrackingMountDefinition();

			if (trackingMountDefinition && trackingMountDefinition->getComponentName() == trackingMountName)
			{
				return trackingMountComponent;
			}
		}
	}

	return TrackingMountComponentPtr();
}

TrackingMountComponentPtr TrackingMountObjectSystem::addNewTrackingMount()
{
	TrackingMountObjectSystemDefinitionPtr config = getTrackingMountSystemConfig();
	MikanTrackingMountID newTrackingMountId = config->addNewTrackingMount();
	TrackingMountDefinitionPtr trackingMountDefinition = config->getTrackingMountConfig(newTrackingMountId);
	TrackingMountComponentPtr trackingMount= createTrackingMountObject(trackingMountDefinition);

	config->notifyPropertyChanged(
		ConfigPropertyChangeSet().addPropertyName(TrackingMountObjectSystemDefinition::k_trackingMountListPropertyId));

	return trackingMount;
}

bool TrackingMountObjectSystem::removeTrackingMountID(MikanTrackingMountID trackingMountId)
{
	TrackingMountObjectSystemDefinitionPtr config = getTrackingMountSystemConfig();

	disposeTrackingMountObject(trackingMountId);

	return config->removeTrackingMountID(trackingMountId);
}

TrackingMountComponentPtr TrackingMountObjectSystem::createTrackingMountObject(
	TrackingMountDefinitionPtr trackingMountConfig)
{
	MikanObjectPtr mikanObject = newObject();
	mikanObject->setName(trackingMountConfig->getComponentName());

	// Add marker component to the object
	TrackingMountComponentPtr componentPtr = mikanObject->addComponent<TrackingMountComponent>();
	componentPtr->setDefinition(trackingMountConfig);
	m_trackingMountComponents.insert({ trackingMountConfig->getTrackingMountId(), componentPtr });

	// Init the object once all components are added
	mikanObject->init();

	return componentPtr;
}

void TrackingMountObjectSystem::disposeTrackingMountObject(MikanTrackingMountID trackingMountId)
{
	auto it = m_trackingMountComponents.find(trackingMountId);
	if (it != m_trackingMountComponents.end())
	{
		TrackingMountComponentPtr componentPtr = it->second.lock();

		// Remove from component list
		m_trackingMountComponents.erase(it);

		// Free the corresponding object
		deleteObject(componentPtr->getOwnerObject());
	}
}

TrackingMountObjectSystemDefinitionConstPtr TrackingMountObjectSystem::getTrackingMountSystemConfigConst() const
{
	return getProjectConfig()->trackingMountSystemConfig;
}

TrackingMountObjectSystemDefinitionPtr TrackingMountObjectSystem::getTrackingMountSystemConfig()
{
	return std::const_pointer_cast<TrackingMountObjectSystemDefinition>(getTrackingMountSystemConfigConst());
}

void TrackingMountObjectSystem::registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase)
{
	propertyDatabase->registerPropertiesForSystem<TrackingMountObjectSystem>();
	propertyDatabase->registerPropertiesForComponent<TrackingMountObjectSystem, TrackingMountComponent>();
}