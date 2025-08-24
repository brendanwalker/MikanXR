#include "MarkerSystemConfig.h"
#include "App.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ProjectConfig.h"
#include "StringUtils.h"

// -- MarkerSystemConfig -----
const std::string MarkerSystemConfig::k_arucoMarkerListPropertyId= "arucoMarkers";
const std::string MarkerSystemConfig::k_arucoDictionaryTypePropertyId = "dictionaryType";
const std::string MarkerSystemConfig::k_charucoRowsPropertyId = "charucoRows";
const std::string MarkerSystemConfig::k_charucoColsPropertyId = "charucoCols";
const std::string MarkerSystemConfig::k_charucoSquareLengthMMPropertyId = "charucoSquareLengthMM";
const std::string MarkerSystemConfig::k_charucoMarkerLengthMMPropertyId = "charucoMarkerLengthMM";
const std::string MarkerSystemConfig::k_charucoDictionaryTypePropertyId = "charucoDictionaryType";

MarkerSystemConfigPtr MarkerSystemConfig::getSystemConfig()
{
	return App::getInstance()->getProfileConfig()->markerSystemConfig;
}

configuru::Config MarkerSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["nextMarkerId"] = m_nextMarkerId;

	// ArUco settings
	pt["dictionaryType"] = k_charucoDictionaryStrings[(int)m_arucoDictionaryType];
	std::vector<configuru::Config> markerConfigs;
	for (MarkerDefinitionPtr MarkerDefinitionPtr : m_arucoMarkerList)
	{
		markerConfigs.push_back(MarkerDefinitionPtr->writeToJSON());
	}
	pt.insert_or_assign(std::string("arucoMarkers"), markerConfigs);

	// ChArUco settings
	pt["charucoRows"] = m_charucoRows;
	pt["charucoCols"] = m_charucoCols;
	pt["charucoSquareLengthMM"] = m_charucoSquareLengthMM;
	pt["charucoMarkerLengthMM"] = m_charucoMarkerLengthMM;
	pt["charucoDictionaryType"] = k_charucoDictionaryStrings[(int)m_charucoDictionaryType];

	return pt;
}

void MarkerSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_nextMarkerId = pt.get_or<int>("nextMarkerId", m_nextMarkerId);

	// Read in the ArUco settings
	const std::string charcuoDictionaryString =
		pt.get_or<std::string>(
			"dictionaryType",
			k_charucoDictionaryStrings[(int)eCharucoDictionaryType::DICT_6X6]);
	m_arucoDictionaryType =
		StringUtils::FindEnumValue<eCharucoDictionaryType>(
			charcuoDictionaryString,
			k_charucoDictionaryStrings);

	m_arucoMarkerList.clear();
	if (pt.has_key("arucoMarkers"))
	{
		for (const configuru::Config& marker_pt : pt["arucoMarkers"].as_array())
		{
			MarkerDefinitionPtr MarkerDefinitionPtr = std::make_shared<MarkerDefinition>();

			MarkerDefinitionPtr->readFromJSON(marker_pt);
			m_arucoMarkerList.push_back(MarkerDefinitionPtr);

			addChildConfig(MarkerDefinitionPtr);
		}
	}

	// Read ChAruco settings
	m_charucoRows = pt.get_or<int>("charucoRows", m_charucoRows);
	m_charucoCols = pt.get_or<int>("charucoCols", m_charucoCols);
	m_charucoSquareLengthMM = pt.get_or<float>("charucoSquareLengthMM", m_charucoSquareLengthMM);
	m_charucoMarkerLengthMM = pt.get_or<float>("charucoMarkerLengthMM", m_charucoMarkerLengthMM);

	const std::string charucoDictionaryString =
		pt.get_or<std::string>(
			"charucoDictionaryType",
			k_charucoDictionaryStrings[(int)eCharucoDictionaryType::DICT_6X6]);
	m_charucoDictionaryType =
		StringUtils::FindEnumValue<eCharucoDictionaryType>(
			charucoDictionaryString,
			k_charucoDictionaryStrings);
}

MarkerDefinitionPtr MarkerSystemConfig::getMarkerConfig(MikanMarkerID markerId) const
{
	auto it = std::find_if(
		m_arucoMarkerList.begin(), m_arucoMarkerList.end(),
		[markerId](MarkerDefinitionPtr configPtr) {
			return configPtr->getMarkerId() == markerId;
		});

	if (it != m_arucoMarkerList.end())
	{
		return MarkerDefinitionPtr(*it);
	}

	return MarkerDefinitionPtr();
}

MarkerDefinitionPtr MarkerSystemConfig::getMarkerConfigByName(const std::string& markerName) const
{
	auto it = std::find_if(
		m_arucoMarkerList.begin(), m_arucoMarkerList.end(),
		[markerName](MarkerDefinitionPtr configPtr) {
			return configPtr->getComponentName() == markerName;
		});

	if (it != m_arucoMarkerList.end())
	{
		return MarkerDefinitionPtr(*it);
	}

	return MarkerDefinitionPtr();
}

MikanMarkerID MarkerSystemConfig::addNewMarker(const std::string& markerName)
{
	MarkerDefinitionPtr MarkerDefinitionPtr = 
		std::make_shared<MarkerDefinition>(m_nextMarkerId, markerName);
	m_nextMarkerId++;

	m_arucoMarkerList.push_back(MarkerDefinitionPtr);
	addChildConfig(MarkerDefinitionPtr);

	markDirty(ConfigPropertyChangeSet().addPropertyName(k_arucoMarkerListPropertyId));

	return MarkerDefinitionPtr->getMarkerId();
}

bool MarkerSystemConfig::removeMarker(MikanMarkerID markerId)
{
	auto it = std::find_if(
		m_arucoMarkerList.begin(), m_arucoMarkerList.end(),
		[markerId](MarkerDefinitionPtr configPtr) {
		return configPtr->getMarkerId() == markerId;
	});

	if (it != m_arucoMarkerList.end())
	{
		removeChildConfig(*it);

		m_arucoMarkerList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_arucoMarkerListPropertyId));

		return true;
	}

	return false;
}

void MarkerSystemConfig::setArucoDictionaryType(eCharucoDictionaryType dictionaryType)
{
	if (dictionaryType != m_arucoDictionaryType)
	{
		m_arucoDictionaryType = dictionaryType;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_arucoDictionaryTypePropertyId));
	}
}

void MarkerSystemConfig::setCharucoRows(int charucoRows)
{
	if (charucoRows != m_charucoRows)
	{
		m_charucoRows = charucoRows;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_charucoRowsPropertyId));
	}
}

void MarkerSystemConfig::setCharucoCols(int charucoCols)
{
	if (charucoCols != m_charucoCols)
	{
		m_charucoCols = charucoCols;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_charucoColsPropertyId));
	}
}

void MarkerSystemConfig::setCharucoSquareLengthMM(float charucoSquareLengthMM)
{
	if (charucoSquareLengthMM != m_charucoSquareLengthMM)
	{
		m_charucoSquareLengthMM = charucoSquareLengthMM;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_charucoSquareLengthMMPropertyId));
	}
}

void MarkerSystemConfig::setCharucoMarkerLengthMM(float charucoMarkerLengthMM)
{
	if (charucoMarkerLengthMM != m_charucoMarkerLengthMM)
	{
		m_charucoMarkerLengthMM = charucoMarkerLengthMM;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_charucoMarkerLengthMMPropertyId));
	}
}

void MarkerSystemConfig::setCharucoDictionaryType(eCharucoDictionaryType charucoDictionaryType)
{
	if (charucoDictionaryType != m_charucoDictionaryType)
	{
		m_charucoDictionaryType = charucoDictionaryType;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_charucoDictionaryTypePropertyId));
	}
}
