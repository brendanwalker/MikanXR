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

TrackingSystemDefinitionConstPtr TrackingSystemsConfig::getTrackingSystemDefinitionConst(
	MikanTrackingSystemID systemId) const
{
	MarkerTrackingSystemDefinitionConstPtr markerSystemPtr = 
		getMarkerTrackingSystemConfigConst(systemId);
	if (markerSystemPtr)
	{
		return markerSystemPtr;
	}

	VRTrackingSystemDefinitionConstPtr vrSystemPtr = 
		getVRTrackingSystemConfigConst(systemId);
	if (vrSystemPtr)
	{
		return vrSystemPtr;
	}

	return TrackingSystemDefinitionPtr();
}

TrackingSystemDefinitionPtr TrackingSystemsConfig::getTrackingSystemDefinition(
	MikanTrackingSystemID systemId)
{
	return 
		std::const_pointer_cast<TrackingSystemDefinition>(
			getTrackingSystemDefinitionConst(systemId));
}

eTrackingSystemType TrackingSystemsConfig::getTrackingSystemType(MikanTrackingSystemID systemId) const
{
	TrackingSystemDefinitionConstPtr trackingSystem= getTrackingSystemDefinitionConst(systemId);
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

MikanTrackingSystemID TrackingSystemsConfig::addMarkerTrakingSystem()
{
	if (!canAddTrackingSystemType(eTrackingSystemType::marker))
		return INVALID_MIKAN_ID;

	const std::string systemName = StringUtils::stringify("Marker System ", m_nextTrackingSystemId);
	MarkerTrackingSystemDefinitionPtr configPtr = 
		std::make_shared<MarkerTrackingSystemDefinition>(
			m_nextTrackingSystemId, systemName);
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
	eTrackingRuntime trackingRuntime)
{
	if (!canAddTrackingSystemType(eTrackingSystemType::vr))
		return INVALID_MIKAN_ID;

	const std::string systemName = StringUtils::stringify("VR System ", m_nextTrackingSystemId);
	VRTrackingSystemDefinitionPtr configPtr =
		std::make_shared<VRTrackingSystemDefinition>(
			trackingRuntime,
			m_nextTrackingSystemId, 
			systemName);
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
