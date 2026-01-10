#include "VRTrackingVolumeSystem.h"
#include "Logger.h"
#include "VRTrackingVolumeComponent.h"
#include "MikanObject.h"
#include "MikanPropertyDatabase.h"
#include "ProjectConfig.h"
#include "ProjectManager.h"
#include "StringUtils.h"
#include "VRObjectSystem.h"

// -- VRTrackingVolumeSystemConfig -----
const std::string VRTrackingVolumeSystemDefinition::k_vrTrackingVolumeListPropertyId = "vr_tracking_volume_ids";

configuru::Config VRTrackingVolumeSystemDefinition::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	std::vector<configuru::Config> vrVolumeConfigs;
	for (VRTrackingVolumeDefinitionPtr volumeDefinition : m_vrTrackingVolumeList)
	{
		vrVolumeConfigs.push_back(volumeDefinition->writeToJSON());
	}
	pt.insert_or_assign(std::string("vrTrackingVolumes"), vrVolumeConfigs);

	pt["nextTrackingVolumeId"] = m_nextTrackingVolumeId;

	return pt;
}

void VRTrackingVolumeSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_nextTrackingVolumeId = pt.get_or<int>("nextTrackingVolumeId", m_nextTrackingVolumeId);

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

VRTrackingVolumeDefinitionConstPtr VRTrackingVolumeSystemDefinition::getVRTrackingVolumeDefinitionConst(
	MikanTrackingVolumeID trackingVolumeId) const
{
	auto iter = std::find_if(
		m_vrTrackingVolumeList.begin(), m_vrTrackingVolumeList.end(),
		[trackingVolumeId](VRTrackingVolumeDefinitionPtr configPtr) {
			return configPtr->getTrackingVolumeId() == trackingVolumeId;
		});

	if (iter != m_vrTrackingVolumeList.end())
	{
		return VRTrackingVolumeDefinitionConstPtr(*iter);
	}

	return VRTrackingVolumeDefinitionConstPtr();
}

VRTrackingVolumeDefinitionPtr VRTrackingVolumeSystemDefinition::getVRTrackingVolumeDefinition(
	MikanTrackingVolumeID trackingVolumeId)
{
	return std::const_pointer_cast<VRTrackingVolumeDefinition>(
		getVRTrackingVolumeDefinitionConst(trackingVolumeId));
}

MikanTrackingVolumeID VRTrackingVolumeSystemDefinition::addVRTrackingVolume(eTrackingRuntime trackingRuntime)
{
	const std::string volumeName = StringUtils::stringify("VR System ", m_nextTrackingVolumeId);
	VRTrackingVolumeDefinitionPtr configPtr =
		std::make_shared<VRTrackingVolumeDefinition>(
			trackingRuntime,
			m_nextTrackingVolumeId,
			volumeName);
	addChildConfig(configPtr);
	m_nextTrackingVolumeId++;

	m_vrTrackingVolumeList.push_back(configPtr);
	notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_vrTrackingVolumeListPropertyId));

	return configPtr->getTrackingVolumeId();
}

bool VRTrackingVolumeSystemDefinition::removeVRTrackingVolume(MikanTrackingVolumeID trackingVolumeId)
{
	auto it = std::find_if(
		m_vrTrackingVolumeList.begin(), m_vrTrackingVolumeList.end(),
		[trackingVolumeId](VRTrackingVolumeDefinitionPtr configPtr) {
			return configPtr->getTrackingVolumeId() == trackingVolumeId;
		});

	if (it != m_vrTrackingVolumeList.end())
	{
		removeChildConfig(*it);

		m_vrTrackingVolumeList.erase(it);
		notifyPropertyChanged(ConfigPropertyChangeSet().addPropertyName(k_vrTrackingVolumeListPropertyId));

		return true;
	}

	return false;
}

// -- VRTrackingVolumeSystem -----
bool VRTrackingVolumeSystem::init(MikanObjectSystemDefinitionPtr definitionPtr)
{
	if (!MikanObjectSystem::init(definitionPtr))
		return false;

	VRTrackingVolumeSystemDefinitionPtr config = getVRTrackingVolumeSystemConfig();
	if (config == nullptr)
		return false;

	// Create tracking volume components for all existing definitions
	auto vrObjectSystem = getOwnerProjectManager()->getSystemOfType<VRObjectSystem>();
	for (VRTrackingVolumeDefinitionPtr vrDef : config->getVRTrackingVolumeList())
	{
		// Ensure the VRObjectSystem has a runtime for this type
		vrObjectSystem->createTrackingRuntime(vrDef->getTrackingRuntime());

		createVRTrackingVolumeObject(vrDef);
	}

	return true;
}

void VRTrackingVolumeSystem::dispose()
{
	// Clean up all tracking volume components
	m_vrTrackingVolumeComponents.clear();

	MikanObjectSystem::dispose();
}

MikanComponentPtr VRTrackingVolumeSystem::getComponentById(int componentId) const
{
	return getVRTrackingVolumeById(componentId);
}

VRTrackingVolumeSystemDefinitionConstPtr VRTrackingVolumeSystem::getVRTrackingVolumeSystemConfigConst() const
{
	return std::static_pointer_cast<const VRTrackingVolumeSystemDefinition>(getDefinitionConst());
}

VRTrackingVolumeSystemDefinitionPtr VRTrackingVolumeSystem::getVRTrackingVolumeSystemConfig()
{
	return std::static_pointer_cast<VRTrackingVolumeSystemDefinition>(getDefinition());
}

VRTrackingVolumeIdList VRTrackingVolumeSystem::getTrackingVolumeIdList() const
{
	VRTrackingVolumeIdList trackingVolumeIdList;
	for (const auto& it : m_vrTrackingVolumeComponents)
	{
		trackingVolumeIdList.push_back(it.first);
	}

	return trackingVolumeIdList;
}

VRTrackingVolumeComponentPtr VRTrackingVolumeSystem::getVRTrackingVolumeById(
	MikanTrackingVolumeID trackingVolumeId) const
{
	auto it = m_vrTrackingVolumeComponents.find(trackingVolumeId);
	if (it != m_vrTrackingVolumeComponents.end())
	{
		return it->second.lock();
	}

	return VRTrackingVolumeComponentPtr();
}

VRTrackingVolumeComponentPtr VRTrackingVolumeSystem::addNewVRTrackingVolume(eTrackingRuntime trackingRuntime)
{
	// Ensure the VRObjectSystem has a runtime for this type
	auto vrObjectSystem = getOwnerProjectManager()->getSystemOfType<VRObjectSystem>();
	if (!vrObjectSystem->createTrackingRuntime(trackingRuntime))
	{
		return VRTrackingVolumeComponentPtr();
	}

	VRTrackingVolumeSystemDefinitionPtr config = getVRTrackingVolumeSystemConfig();
	if (config == nullptr)
		return VRTrackingVolumeComponentPtr();

	MikanTrackingVolumeID volumeId = config->addVRTrackingVolume(trackingRuntime);
	if (volumeId == INVALID_MIKAN_ID)
		return VRTrackingVolumeComponentPtr();

	VRTrackingVolumeDefinitionPtr vrDef = config->getVRTrackingVolumeDefinition(volumeId);
	VRTrackingVolumeComponentPtr trackingVolume = createVRTrackingVolumeObject(vrDef);

	return trackingVolume;
}

bool VRTrackingVolumeSystem::removeVRTrackingVolume(MikanTrackingVolumeID trackingVolumeId)
{
	VRTrackingVolumeSystemDefinitionPtr config = getVRTrackingVolumeSystemConfig();
	if (config == nullptr)
		return false;

	// Remove the component first
	disposeVRTrackingVolumeObject(trackingVolumeId);

	// Then remove from config
	return config->removeVRTrackingVolume(trackingVolumeId);
}

VRTrackingVolumeComponentPtr VRTrackingVolumeSystem::createVRTrackingVolumeObject(
	VRTrackingVolumeDefinitionPtr trackingVolumeConfig)
{
	MikanTrackingVolumeID trackingVolumeId = trackingVolumeConfig->getTrackingVolumeId();

	MikanObjectPtr mikanObject = newObject();
	mikanObject->setName(trackingVolumeConfig->getComponentName());

	// Create VR tracking volume component
	VRTrackingVolumeComponentPtr componentPtr = mikanObject->addComponent<VRTrackingVolumeComponent>();
	componentPtr->setDefinition(trackingVolumeConfig);

	if (componentPtr != nullptr)
	{
		m_vrTrackingVolumeComponents.insert({ trackingVolumeId, componentPtr });

		// Init the object once all components are added
		mikanObject->init();
	}

	return componentPtr;
}

void VRTrackingVolumeSystem::disposeVRTrackingVolumeObject(MikanTrackingVolumeID trackingVolumeId)
{
	auto it = m_vrTrackingVolumeComponents.find(trackingVolumeId);
	if (it != m_vrTrackingVolumeComponents.end())
	{
		VRTrackingVolumeComponentPtr componentPtr = it->second.lock();

		// Remove from component list
		m_vrTrackingVolumeComponents.erase(it);

		// Free the corresponding object
		deleteObject(componentPtr->getOwnerObject());
	}
}

void VRTrackingVolumeSystem::registerPropertyDescriptors(MikanPropertyDatabasePtr propertyDatabase)
{
	propertyDatabase->registerPropertiesForSystem<VRTrackingVolumeSystem>();
	propertyDatabase->registerPropertiesForComponent<VRTrackingVolumeSystem, VRTrackingVolumeComponent>();
}
