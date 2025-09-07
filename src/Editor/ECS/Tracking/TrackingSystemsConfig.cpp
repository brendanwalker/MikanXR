#include "TrackingSystemsConfig.h"
#include "App.h"
#include "MathTypeConversion.h"
#include "MikanObject.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ProjectConfig.h"
#include "SelectionComponent.h"
#include "StringUtils.h"

// -- TrackingSystemsConfig -----
const std::string TrackingSystemsConfig::k_markerTrackingSystemListPropertyId= "markerTrackingSystemList";
const std::string TrackingSystemsConfig::k_vrTrackingSystemListPropertyId= "vrTrackingSystemList";

TrackingSystemsConfigPtr TrackingSystemsConfig::getSystemConfig()
{
	return App::getInstance()->getProfileConfig()->trackingSystemsConfig;
}

configuru::Config TrackingSystemsConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	std::vector<configuru::Config> markerSystemsConfigs;
	for (MarkerTrackingAPIDefinitionPtr systemDefinition : m_markerTrackingAPIList)
	{
		markerSystemsConfigs.push_back(systemDefinition->writeToJSON());
	}
	pt.insert_or_assign(std::string("markerTrackingSystems"), markerSystemsConfigs);

	std::vector<configuru::Config> vrSystemsConfigs;
	for (VRTrackingAPIDefinitionPtr systemDefinition : m_vrTrackingAPIList)
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

	m_markerTrackingAPIList.clear();
	if (pt.has_key("markerTrackingSystems"))
	{
		for (const configuru::Config& trackingSystemConfig : pt["markerTrackingSystems"].as_array())
		{
			MarkerTrackingAPIDefinitionPtr trackingSystemDefinitionPtr = 
				std::make_shared<MarkerTrackingAPIDefinition>();

			trackingSystemDefinitionPtr->readFromJSON(trackingSystemConfig);
			m_markerTrackingAPIList.push_back(trackingSystemDefinitionPtr);

			addChildConfig(trackingSystemDefinitionPtr);
		}
	}

	m_vrTrackingAPIList.clear();
	if (pt.has_key("vrTrackingSystems"))
	{
		for (const configuru::Config& trackingSystemConfig : pt["vrTrackingSystems"].as_array())
		{
			VRTrackingAPIDefinitionPtr trackingSystemDefinitionPtr =
				std::make_shared<VRTrackingAPIDefinition>();

			trackingSystemDefinitionPtr->readFromJSON(trackingSystemConfig);
			m_vrTrackingAPIList.push_back(trackingSystemDefinitionPtr);

			addChildConfig(trackingSystemDefinitionPtr);
		}
	}
}

bool TrackingSystemsConfig::canAddTrackingSystemType(eTrackingSystemType systemType) const
{
	switch (systemType)
	{
		case eTrackingSystemType::vr:
			return m_vrTrackingAPIList.size() == 0;
		case eTrackingSystemType::marker:
			return true;
	}

	return false;
}

TrackingAPIDefinitionConstPtr TrackingSystemsConfig::getTrackingAPIDefinitionConst(
	MikanTrackingSystemID systemId) const
{
	MarkerTrackingAPIDefinitionConstPtr markerSystemPtr = 
		getMarkerTrackingAPIDefinitionConst(systemId);
	if (markerSystemPtr)
	{
		return markerSystemPtr;
	}

	VRTrackingAPIDefinitionConstPtr vrSystemPtr = 
		getVRTrackingAPIDefinitionConst(systemId);
	if (vrSystemPtr)
	{
		return vrSystemPtr;
	}

	return TrackingAPIDefinitionPtr();
}

TrackingAPIDefinitionPtr TrackingSystemsConfig::getTrackingAPIDefinition(
	MikanTrackingSystemID systemId)
{
	return 
		std::const_pointer_cast<TrackingAPIDefinition>(
			getTrackingAPIDefinitionConst(systemId));
}

eTrackingSystemType TrackingSystemsConfig::getTrackingSystemType(MikanTrackingSystemID systemId) const
{
	TrackingAPIDefinitionConstPtr trackingSystem= getTrackingAPIDefinitionConst(systemId);
	if (trackingSystem)
	{
		return trackingSystem->getTrackingSystemType();
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

MarkerTrackingAPIDefinitionConstPtr TrackingSystemsConfig::getMarkerTrackingAPIDefinitionConst(MikanTrackingSystemID systemId) const
{
	auto iter = std::find_if(
		m_markerTrackingAPIList.begin(), m_markerTrackingAPIList.end(),
		[systemId](MarkerTrackingAPIDefinitionPtr configPtr) {
			return configPtr->getTrackingSystemId() == systemId;
		});

	if (iter != m_markerTrackingAPIList.end())
	{
		return MarkerTrackingAPIDefinitionConstPtr(*iter);
	}

	return MarkerTrackingAPIDefinitionConstPtr();
}

MarkerTrackingAPIDefinitionPtr TrackingSystemsConfig::getMarkerTrackingAPIDefinition(MikanTrackingSystemID systemId)
{
	return std::const_pointer_cast<MarkerTrackingAPIDefinition>(getMarkerTrackingAPIDefinitionConst(systemId));
}

MikanTrackingSystemID TrackingSystemsConfig::addMarkerTrakingSystem()
{
	if (!canAddTrackingSystemType(eTrackingSystemType::marker))
		return INVALID_MIKAN_ID;

	const std::string systemName = StringUtils::stringify("Marker System ", m_nextTrackingSystemId);
	MarkerTrackingAPIDefinitionPtr configPtr = 
		std::make_shared<MarkerTrackingAPIDefinition>(
			m_nextTrackingSystemId, systemName);
	addChildConfig(configPtr);
	m_nextTrackingSystemId++;

	m_markerTrackingAPIList.push_back(configPtr);
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_markerTrackingSystemListPropertyId));

	return configPtr->getTrackingSystemId();
}

bool TrackingSystemsConfig::removeMarkerTrackingSystem(MikanTrackingSystemID systemId)
{
	auto it = std::find_if(
		m_markerTrackingAPIList.begin(), m_markerTrackingAPIList.end(),
		[systemId](MarkerTrackingAPIDefinitionPtr configPtr) {
			return configPtr->getTrackingSystemId() == systemId;
		});

	if (it != m_markerTrackingAPIList.end())
	{
		removeChildConfig(*it);

		m_markerTrackingAPIList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_markerTrackingSystemListPropertyId));

		return true;
	}

	return false;
}

VRTrackingAPIDefinitionConstPtr TrackingSystemsConfig::getVRTrackingAPIDefinitionConst(MikanTrackingSystemID systemId) const
{
	auto iter = std::find_if(
		m_vrTrackingAPIList.begin(), m_vrTrackingAPIList.end(),
		[systemId](VRTrackingAPIDefinitionPtr configPtr) {
			return configPtr->getTrackingSystemId() == systemId;
		});

	if (iter != m_vrTrackingAPIList.end())
	{
		return VRTrackingAPIDefinitionConstPtr(*iter);
	}

	return VRTrackingAPIDefinitionConstPtr();
}

VRTrackingAPIDefinitionPtr TrackingSystemsConfig::getVRTrackingAPIDefinition(MikanTrackingSystemID systemId)
{
	return std::const_pointer_cast<VRTrackingAPIDefinition>(getVRTrackingAPIDefinitionConst(systemId));
}

MikanTrackingSystemID TrackingSystemsConfig::addVRTrackingSystem(
	eTrackingRuntime trackingRuntime)
{
	if (!canAddTrackingSystemType(eTrackingSystemType::vr))
		return INVALID_MIKAN_ID;

	const std::string systemName = StringUtils::stringify("VR System ", m_nextTrackingSystemId);
	VRTrackingAPIDefinitionPtr configPtr =
		std::make_shared<VRTrackingAPIDefinition>(
			trackingRuntime,
			m_nextTrackingSystemId, 
			systemName);
	addChildConfig(configPtr);	
	m_nextTrackingSystemId++;

	m_vrTrackingAPIList.push_back(configPtr);
	markDirty(ConfigPropertyChangeSet().addPropertyName(k_vrTrackingSystemListPropertyId));

	return configPtr->getTrackingSystemId();
}

bool TrackingSystemsConfig::removeVRTrackingSystem(MikanTrackingSystemID systemId)
{
	auto it = std::find_if(
		m_vrTrackingAPIList.begin(), m_vrTrackingAPIList.end(),
		[systemId](VRTrackingAPIDefinitionPtr configPtr) {
			return configPtr->getTrackingSystemId() == systemId;
		});

	if (it != m_vrTrackingAPIList.end())
	{
		removeChildConfig(*it);

		m_vrTrackingAPIList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_vrTrackingSystemListPropertyId));

		return true;
	}

	return false;
}
