#include "App.h"
#include "TrackingSystemConfig.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

// -- TrackingSystemDefinition -----
const std::string TrackingSystemDefinition::k_originMarkerPropertyId = "originMarker";

TrackingSystemDefinition::TrackingSystemDefinition() 
	: MikanComponentDefinition()
	, m_trackingSystemId(INVALID_MIKAN_ID)
	, m_originMarkeId(INVALID_MIKAN_ID)
{
}
TrackingSystemDefinition::TrackingSystemDefinition(
	MikanTrackingSystemID trackingSystemId,
	const std::string& trackingSystemName)
	: MikanComponentDefinition(trackingSystemName)
	, m_trackingSystemId(trackingSystemId)
	, m_originMarkeId(INVALID_MIKAN_ID)
{
}

configuru::Config TrackingSystemDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();
	
	pt["tracking_system_id"] = m_trackingSystemId;
	pt["origin_marker_id"] = m_originMarkeId;

	return pt;
}

void TrackingSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	m_trackingSystemId = pt.get_or<MikanTrackingSystemID>("tracking_system_id", m_trackingSystemId);
	m_originMarkeId = pt.get_or<MikanMarkerID>("origin_marker_id", m_originMarkeId);
}

void TrackingSystemDefinition::setOriginMarkerId(MikanMarkerID arucoId)
{
	if (arucoId != m_originMarkeId)
	{
		m_originMarkeId = arucoId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_originMarkerPropertyId));
	}
}

// -- VRTrackingSystemDefinition -----
const std::string VRTrackingSystemDefinition::k_charucoMountIdPropertyId= "charucoMountId";
const std::string VRTrackingSystemDefinition::k_utilityMarkerIdPropertyId= "utilityMarkerId";
const std::string VRTrackingSystemDefinition::k_trackingMountsPropertyId= "trackingMounts";

configuru::Config VRTrackingSystemDefinition::writeToJSON()
{
	configuru::Config pt = TrackingSystemDefinition::writeToJSON();

	pt["tracking_runtime"] = k_trackingRuntimeStrings[(int)m_trackingRuntime];
	pt["charuco_mount_id"] = m_charucoMountId;
	pt["utility_marker_id"] = m_utilityMarkerId;
	pt["next_tracking_mount_id"] = m_nextTrackingMountId;

	std::vector<configuru::Config> trackingMountConfigs;
	for (auto trackingMount : m_trackingMounts)
	{
		trackingMountConfigs.push_back(trackingMount->writeToJSON());
	}
	pt.insert_or_assign(std::string("tracking_mounts"), trackingMountConfigs);

	return pt;
}

void VRTrackingSystemDefinition::readFromJSON(const configuru::Config& pt)
{
	TrackingSystemDefinition::readFromJSON(pt);

	const std::string trackingRuntimeString = 
		pt.get_or<std::string>("tracking_runtime", k_trackingRuntimeStrings[0]);
	m_trackingRuntime = 
		StringUtils::FindEnumValue<eTrackingRuntime>(trackingRuntimeString, k_trackingRuntimeStrings);
	m_charucoMountId = pt.get_or<MikanTrackingMountID>("charuco_mount_id", INVALID_MIKAN_ID);
	m_utilityMarkerId = pt.get_or<MikanMarkerID>("utility_marker_id", INVALID_MIKAN_ID);

	if (pt.has_key("tracking_mounts"))
	{
		for (const configuru::Config& trackingMount_pt : pt["tracking_mounts"].as_array())
		{
			auto definitionPtr = std::make_shared<TrackingMountDefinition>();

			definitionPtr->readFromJSON(trackingMount_pt);
			m_trackingMounts.push_back(definitionPtr);

			addChildConfig(definitionPtr);
		}
	}
}

void VRTrackingSystemDefinition::setCharucoTrackingMountId(MikanTrackingMountID mountId)
{
	if (mountId != m_charucoMountId)
	{
		m_charucoMountId = mountId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_charucoMountIdPropertyId));
	}
}

void VRTrackingSystemDefinition::setUtilityMarkerId(MikanMarkerID markerId)
{
	if (markerId != m_utilityMarkerId)
	{
		m_utilityMarkerId = markerId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_utilityMarkerIdPropertyId));
	}
}

TrackingMountDefinitionConstPtr VRTrackingSystemDefinition::getTrackingMountDefinitionConst(
	MikanTrackingMountID mountId) const
{
	auto it=
		std::find_if(
			m_trackingMounts.begin(), m_trackingMounts.end(),
			[mountId](const TrackingMountDefinitionPtr& mountDef) {
				return mountDef->getTrackingMountId() == mountId;
			});

	if (it != m_trackingMounts.end())
	{
		return *it;
	}

	return TrackingMountDefinitionConstPtr();
}

TrackingMountDefinitionPtr VRTrackingSystemDefinition::getTrackingMountDefinition(MikanTrackingMountID mountId)
{
	return std::const_pointer_cast<TrackingMountDefinition>(getTrackingMountDefinitionConst(mountId));
}

MikanTrackingMountID VRTrackingSystemDefinition::addTrackingMount(
	const std::string& mountName)
{
	TrackingMountDefinitionPtr trackingMountDefinition =
		std::make_shared<TrackingMountDefinition>(m_nextTrackingMountId, mountName);
	m_nextTrackingMountId++;

	m_trackingMounts.push_back(trackingMountDefinition);
	addChildConfig(trackingMountDefinition);

	markDirty(ConfigPropertyChangeSet().addPropertyName(k_trackingMountsPropertyId));

	return trackingMountDefinition->getTrackingMountId();
}

bool VRTrackingSystemDefinition::removeTrackingMount(MikanTrackingMountID mountId)
{
	auto it = std::find_if(
		m_trackingMounts.begin(), m_trackingMounts.end(),
		[mountId](const TrackingMountDefinitionPtr& mountDef) {
			return mountDef->getTrackingMountId() == mountId;
		});

	if (it != m_trackingMounts.end())
	{
		removeChildConfig(*it);
		m_trackingMounts.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_trackingMountsPropertyId));

		return true;
	}

	return false;
}

// -- TrackingMountDefinition -----
const std::string TrackingMountDefinition::k_devicePathPropertyId = "devicePath";
const std::string TrackingMountDefinition::k_socketNamePropertyId = "socketName";

TrackingMountDefinition::TrackingMountDefinition() 
	: MikanComponentDefinition()
	, m_trackingMountId(INVALID_MIKAN_ID)
{}

TrackingMountDefinition::TrackingMountDefinition(
	MikanTrackingMountID trackingMountId,
	const std::string& markerName)
	: MikanComponentDefinition(markerName)
	, m_trackingMountId(trackingMountId)
{
}

configuru::Config TrackingMountDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();
	
	pt["tracking_mount_id"] = m_trackingMountId;
	pt["device_path"] = m_devicePath;
	pt["socket_name"] = m_socketName;

	return pt;
}

void TrackingMountDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	m_trackingMountId = pt.get_or<MikanTrackingMountID>("tracking_mount_id", m_trackingMountId);
	m_devicePath = pt.get_or<std::string>("device_path", m_devicePath);
	m_socketName = pt.get_or<std::string>("socket_name", m_socketName);
}

void TrackingMountDefinition::setDevicePath(const std::string& devicePath)
{
	if (devicePath != m_devicePath)
	{
		m_devicePath = devicePath;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_devicePathPropertyId));
	}
}

void TrackingMountDefinition::setSocketName(const std::string& socketName)
{
	if (socketName != m_socketName)
	{
		m_socketName = socketName;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_socketNamePropertyId));
	}
}

// -- TrackingSystemConfig -----
const std::string TrackingSystemsConfig::k_markerTrackingSystemListPropertyId= "markerTrackingSystemList";
const std::string TrackingSystemsConfig::k_vrTrackingSystemListPropertyId= "vrTrackingSystemList";

VRTrackingSystemDefinition::VRTrackingSystemDefinition(
	eTrackingRuntime trackingRuntime,
	MikanTrackingSystemID trackingSystemId,
	const std::string& trackingSystemName)
	: TrackingSystemDefinition(trackingSystemId, trackingSystemName)
	, m_trackingRuntime(trackingRuntime)
	, m_charucoMountId(INVALID_MIKAN_ID)
	, m_utilityMarkerId(INVALID_MIKAN_ID)
{
}

configuru::Config TrackingSystemsConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	std::vector<configuru::Config> markerSystemsConfigs;
	for (MarkerTrackingSystemDefinitionPtr systemDefinition : m_markerTrackingSystemList)
	{
		markerSystemsConfigs.push_back(systemDefinition->writeToJSON());
	}
	pt.insert_or_assign(std::string("markerTrackingSystems"), markerSystemsConfigs);

	std::vector<configuru::Config> vrSystemsConfigs;
	for (VRTrackingSystemDefinitionPtr systemDefinition : m_vrTrackingSystemList)
	{
		vrSystemsConfigs.push_back(systemDefinition->writeToJSON());
	}
	pt.insert_or_assign(std::string("vrTrackingSystems"), vrSystemsConfigs);

	pt["nextTrackingSystemId"] = m_nextTrackingSystemId;

	return pt;
}

void TrackingSystemsConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_nextTrackingSystemId = pt.get_or<int>("nextTrackingSystemId", m_nextTrackingSystemId);

	m_markerTrackingSystemList.clear();
	if (pt.has_key("markerTrackingSystems"))
	{
		for (const configuru::Config& trackingSystemConfig : pt["markerTrackingSystems"].as_array())
		{
			MarkerTrackingSystemDefinitionPtr trackingSystemDefinitionPtr = 
				std::make_shared<MarkerTrackingSystemDefinition>();

			trackingSystemDefinitionPtr->readFromJSON(trackingSystemConfig);
			m_markerTrackingSystemList.push_back(trackingSystemDefinitionPtr);

			addChildConfig(trackingSystemDefinitionPtr);
		}
	}

	m_vrTrackingSystemList.clear();
	if (pt.has_key("vrTrackingSystems"))
	{
		for (const configuru::Config& trackingSystemConfig : pt["vrTrackingSystems"].as_array())
		{
			VRTrackingSystemDefinitionPtr trackingSystemDefinitionPtr =
				std::make_shared<VRTrackingSystemDefinition>();

			trackingSystemDefinitionPtr->readFromJSON(trackingSystemConfig);
			m_vrTrackingSystemList.push_back(trackingSystemDefinitionPtr);

			addChildConfig(trackingSystemDefinitionPtr);
		}
	}
}

bool TrackingSystemsConfig::canAddTrackingSystemType(eTrackingSystemType systemType) const
{
	switch (systemType)
	{
		case eTrackingSystemType::vr:
			return m_vrTrackingSystemList.size() == 0;
		case eTrackingSystemType::marker:
			return true;
	}

	return false;
}

eTrackingSystemType TrackingSystemsConfig::getTrackingSystemType(MikanTrackingSystemID systemId) const
{
	MarkerTrackingSystemDefinitionConstPtr markerSystemPtr = getMarkerTrackingSystemConfigConst(systemId);
	if (markerSystemPtr)
	{
		return eTrackingSystemType::marker;
	}

	VRTrackingSystemDefinitionConstPtr vrSystemPtr = getVRTrackingSystemConfigConst(systemId);
	if (vrSystemPtr)
	{
		return eTrackingSystemType::vr;
	}

	return eTrackingSystemType::INVALID;
}

bool TrackingSystemsConfig::removeTrackingSystem(MikanTrackingSystemID systemId)
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


MarkerTrackingSystemDefinitionConstPtr TrackingSystemsConfig::getMarkerTrackingSystemConfigConst(MikanTrackingSystemID systemId) const
{
	auto iter = std::find_if(
		m_markerTrackingSystemList.begin(), m_markerTrackingSystemList.end(),
		[systemId](MarkerTrackingSystemDefinitionPtr configPtr) {
			return configPtr->getTrackingSystemId() == systemId;
		});

	if (iter != m_markerTrackingSystemList.end())
	{
		return MarkerTrackingSystemDefinitionConstPtr(*iter);
	}

	return MarkerTrackingSystemDefinitionConstPtr();
}

MarkerTrackingSystemDefinitionPtr TrackingSystemsConfig::getMarkerTrackingSystemConfig(MikanTrackingSystemID systemId)
{
	return std::const_pointer_cast<MarkerTrackingSystemDefinition>(getMarkerTrackingSystemConfigConst(systemId));
}

MikanTrackingSystemID TrackingSystemsConfig::addMarkerTrakingSystem(const std::string& trackingSystemName)
{
	if (!canAddTrackingSystemType(eTrackingSystemType::marker))
		return INVALID_MIKAN_ID;

	MarkerTrackingSystemDefinitionPtr configPtr = 
		std::make_shared<MarkerTrackingSystemDefinition>(
			m_nextTrackingSystemId, trackingSystemName);
	addChildConfig(configPtr);
	m_nextTrackingSystemId++;

	m_markerTrackingSystemList.push_back(configPtr);
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_markerTrackingSystemListPropertyId));

	return configPtr->getTrackingSystemId();
}

bool TrackingSystemsConfig::removeMarkerTrackingSystem(MikanTrackingSystemID systemId)
{
	auto it = std::find_if(
		m_markerTrackingSystemList.begin(), m_markerTrackingSystemList.end(),
		[systemId](MarkerTrackingSystemDefinitionPtr configPtr) {
			return configPtr->getTrackingSystemId() == systemId;
		});

	if (it != m_markerTrackingSystemList.end())
	{
		removeChildConfig(*it);

		m_markerTrackingSystemList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_markerTrackingSystemListPropertyId));

		return true;
	}

	return false;
}

VRTrackingSystemDefinitionConstPtr TrackingSystemsConfig::getVRTrackingSystemConfigConst(MikanTrackingSystemID systemId) const
{
	auto iter = std::find_if(
		m_vrTrackingSystemList.begin(), m_vrTrackingSystemList.end(),
		[systemId](VRTrackingSystemDefinitionPtr configPtr) {
			return configPtr->getTrackingSystemId() == systemId;
		});

	if (iter != m_vrTrackingSystemList.end())
	{
		return VRTrackingSystemDefinitionConstPtr(*iter);
	}

	return VRTrackingSystemDefinitionConstPtr();
}

VRTrackingSystemDefinitionPtr TrackingSystemsConfig::getVRTrackingSystemConfig(MikanTrackingSystemID systemId)
{
	return std::const_pointer_cast<VRTrackingSystemDefinition>(getVRTrackingSystemConfigConst(systemId));
}

MikanTrackingSystemID TrackingSystemsConfig::addVRTrackingSystem(
	eTrackingRuntime trackingRuntime,
	const std::string& trackingSystemName)
{
	if (!canAddTrackingSystemType(eTrackingSystemType::vr))
		return INVALID_MIKAN_ID;

	VRTrackingSystemDefinitionPtr configPtr =
		std::make_shared<VRTrackingSystemDefinition>(
			trackingRuntime,
			m_nextTrackingSystemId, 
			trackingSystemName);
	addChildConfig(configPtr);	
	m_nextTrackingSystemId++;

	m_vrTrackingSystemList.push_back(configPtr);
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_vrTrackingSystemListPropertyId));

	return configPtr->getTrackingSystemId();
}

bool TrackingSystemsConfig::removeVRTrackingSystem(MikanTrackingSystemID systemId)
{
	auto it = std::find_if(
		m_vrTrackingSystemList.begin(), m_vrTrackingSystemList.end(),
		[systemId](VRTrackingSystemDefinitionPtr configPtr) {
			return configPtr->getTrackingSystemId() == systemId;
		});

	if (it != m_vrTrackingSystemList.end())
	{
		removeChildConfig(*it);

		m_vrTrackingSystemList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_vrTrackingSystemListPropertyId));

		return true;
	}

	return false;
}