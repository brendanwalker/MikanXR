#include "App.h"
#include "MarkerSystemConfig.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ProjectConfig.h"
#include "StringUtils.h"

// -- MarkerConfig -----
const std::string MarkerDefinition::k_arucoIdPropertyId= "aruco_id";
const std::string MarkerDefinition::k_lengthMMPropertyId= "length_mm";

MarkerDefinition::MarkerDefinition()
	: MikanComponentDefinition()
{
	m_markerId = INVALID_MIKAN_ID;
}

MarkerDefinition::MarkerDefinition(
	MikanMarkerID markerId,
	const std::string& markerName)
	: MikanComponentDefinition(markerName)
	, m_markerId(markerId)
{}

configuru::Config MarkerDefinition::writeToJSON()
{
	configuru::Config pt = MikanComponentDefinition::writeToJSON();

	pt["id"] = m_markerId;
	pt["aruco_id"] = m_arucoId;
	pt["length_mm"] = m_lengthMM;

	return pt;
}

void MarkerDefinition::readFromJSON(const configuru::Config& pt)
{
	MikanComponentDefinition::readFromJSON(pt);

	if (pt.has_key("id"))
	{
		m_markerId = pt.get<int>("id");
		m_arucoId = pt.get_or<int>("aruco_id", 0);
		m_lengthMM = pt.get_or<float>("length_mm", 100.0f); // Default length is 100mm
		m_configName = StringUtils::stringify("Marker_", m_markerId);
	}
}

void MarkerDefinition::setArucoId(int arucoId)
{
	if (arucoId != m_arucoId)
	{
		m_arucoId = arucoId;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_arucoIdPropertyId));
	}
}

void MarkerDefinition::setLengthMM(float lengthMM)
{
	if (lengthMM != m_lengthMM)
	{
		m_lengthMM = lengthMM;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_lengthMMPropertyId));
	}
}

// -- MarkerSystemConfig -----
const std::string MarkerSystemConfig::k_markerListPropertyId= "Markers";

configuru::Config MarkerSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["nextMarkerId"] = nextMarkerId;

	std::vector<configuru::Config> markerConfigs;
	for (MarkerDefinitionPtr MarkerDefinitionPtr : MarkerList)
	{
		markerConfigs.push_back(MarkerDefinitionPtr->writeToJSON());
	}
	pt.insert_or_assign(std::string("markers"), markerConfigs);

	return pt;
}

void MarkerSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	nextMarkerId = pt.get_or<int>("nextMarkerId", nextMarkerId);

	// Read in the spatial markers
	MarkerList.clear();
	if (pt.has_key("markers"))
	{
		for (const configuru::Config& marker_pt : pt["markers"].as_array())
		{
			MarkerDefinitionPtr MarkerDefinitionPtr = std::make_shared<MarkerDefinition>();

			MarkerDefinitionPtr->readFromJSON(marker_pt);
			MarkerList.push_back(MarkerDefinitionPtr);

			addChildConfig(MarkerDefinitionPtr);
		}
	}
}

MarkerDefinitionPtr MarkerSystemConfig::getMarkerConfig(MikanMarkerID markerId) const
{
	auto it = std::find_if(
		MarkerList.begin(), MarkerList.end(),
		[markerId](MarkerDefinitionPtr configPtr) {
			return configPtr->getMarkerId() == markerId;
		});

	if (it != MarkerList.end())
	{
		return MarkerDefinitionPtr(*it);
	}

	return MarkerDefinitionPtr();
}

MarkerDefinitionPtr MarkerSystemConfig::getMarkerConfigByName(const std::string& markerName) const
{
	auto it = std::find_if(
		MarkerList.begin(), MarkerList.end(),
		[markerName](MarkerDefinitionPtr configPtr) {
			return configPtr->getComponentName() == markerName;
		});

	if (it != MarkerList.end())
	{
		return MarkerDefinitionPtr(*it);
	}

	return MarkerDefinitionPtr();
}

MikanMarkerID MarkerSystemConfig::addNewMarker(const std::string& markerName)
{
	MarkerDefinitionPtr MarkerDefinitionPtr = std::make_shared<MarkerDefinition>(nextMarkerId, markerName);
	nextMarkerId++;

	MarkerList.push_back(MarkerDefinitionPtr);
	addChildConfig(MarkerDefinitionPtr);

	markDirty(ConfigPropertyChangeSet().addPropertyName(k_markerListPropertyId));

	return MarkerDefinitionPtr->getMarkerId();
}

bool MarkerSystemConfig::removeMarker(MikanMarkerID markerId)
{
	auto it = std::find_if(
		MarkerList.begin(), MarkerList.end(),
		[markerId](MarkerDefinitionPtr configPtr) {
		return configPtr->getMarkerId() == markerId;
	});

	if (it != MarkerList.end())
	{
		removeChildConfig(*it);

		MarkerList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_markerListPropertyId));

		return true;
	}

	return false;
}