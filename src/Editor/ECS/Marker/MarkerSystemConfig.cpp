#include "MarkerSystemConfig.h"
#include "App.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "ProjectConfig.h"
#include "StringUtils.h"

// -- MarkerSystemConfig -----
const std::string MarkerSystemConfig::k_markerListPropertyId= "markers";
const std::string MarkerSystemConfig::k_dictionaryTypePropertyId = "dictionaryType";

MarkerSystemConfigPtr MarkerSystemConfig::getSystemConfig()
{
	return App::getInstance()->getProfileConfig()->markerSystemConfig;
}

configuru::Config MarkerSystemConfig::writeToJSON()
{
	configuru::Config pt = CommonConfig::writeToJSON();

	pt["nextMarkerId"] = m_nextMarkerId;
	pt["dictionaryType"] = k_charucoDictionaryStrings[(int)m_dictionaryType];

	std::vector<configuru::Config> markerConfigs;
	for (MarkerDefinitionPtr MarkerDefinitionPtr : m_markerList)
	{
		markerConfigs.push_back(MarkerDefinitionPtr->writeToJSON());
	}
	pt.insert_or_assign(std::string("markers"), markerConfigs);

	return pt;
}

void MarkerSystemConfig::readFromJSON(const configuru::Config& pt)
{
	CommonConfig::readFromJSON(pt);

	m_nextMarkerId = pt.get_or<int>("nextMarkerId", m_nextMarkerId);

	const std::string charcuoDictionaryString =
		pt.get_or<std::string>(
			"dictionaryType",
			k_charucoDictionaryStrings[(int)eCharucoDictionaryType::DICT_6X6]);
	m_dictionaryType =
		StringUtils::FindEnumValue<eCharucoDictionaryType>(
			charcuoDictionaryString,
			k_charucoDictionaryStrings);


	// Read in the spatial markers
	m_markerList.clear();
	if (pt.has_key("markers"))
	{
		for (const configuru::Config& marker_pt : pt["markers"].as_array())
		{
			MarkerDefinitionPtr MarkerDefinitionPtr = std::make_shared<MarkerDefinition>();

			MarkerDefinitionPtr->readFromJSON(marker_pt);
			m_markerList.push_back(MarkerDefinitionPtr);

			addChildConfig(MarkerDefinitionPtr);
		}
	}
}

MarkerDefinitionPtr MarkerSystemConfig::getMarkerConfig(MikanMarkerID markerId) const
{
	auto it = std::find_if(
		m_markerList.begin(), m_markerList.end(),
		[markerId](MarkerDefinitionPtr configPtr) {
			return configPtr->getMarkerId() == markerId;
		});

	if (it != m_markerList.end())
	{
		return MarkerDefinitionPtr(*it);
	}

	return MarkerDefinitionPtr();
}

MarkerDefinitionPtr MarkerSystemConfig::getMarkerConfigByName(const std::string& markerName) const
{
	auto it = std::find_if(
		m_markerList.begin(), m_markerList.end(),
		[markerName](MarkerDefinitionPtr configPtr) {
			return configPtr->getComponentName() == markerName;
		});

	if (it != m_markerList.end())
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

	m_markerList.push_back(MarkerDefinitionPtr);
	addChildConfig(MarkerDefinitionPtr);

	markDirty(ConfigPropertyChangeSet().addPropertyName(k_markerListPropertyId));

	return MarkerDefinitionPtr->getMarkerId();
}

bool MarkerSystemConfig::removeMarker(MikanMarkerID markerId)
{
	auto it = std::find_if(
		m_markerList.begin(), m_markerList.end(),
		[markerId](MarkerDefinitionPtr configPtr) {
		return configPtr->getMarkerId() == markerId;
	});

	if (it != m_markerList.end())
	{
		removeChildConfig(*it);

		m_markerList.erase(it);
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_markerListPropertyId));

		return true;
	}

	return false;
}

void MarkerSystemConfig::setDictionaryType(eCharucoDictionaryType dictionaryType)
{
	if (dictionaryType != m_dictionaryType)
	{
		m_dictionaryType = dictionaryType;
		markDirty(ConfigPropertyChangeSet().addPropertyName(k_dictionaryTypePropertyId));
	}
}