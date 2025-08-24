#pragma once

#include "CommonConfig.h"
#include "MarkerDefinition.h"
#include "ProjectConfigConstants.h"

#include <map>
#include <memory>
#include <string>

class MarkerSystemConfig : public CommonConfig
{
public:
	MarkerSystemConfig(const std::string& configName)
		: CommonConfig(configName)
	{}

	static MarkerSystemConfigPtr getSystemConfig();

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	static const std::string k_markerListPropertyId;
	MarkerDefinitionPtr getMarkerConfig(MikanMarkerID markerId) const;
	MarkerDefinitionPtr getMarkerConfigByName(const std::string& MarkerName) const;
	MikanMarkerID addNewMarker(const std::string& markerName);
	bool removeMarker(MikanMarkerID markerId);

	static const std::string k_dictionaryTypePropertyId;
	inline eCharucoDictionaryType getDictionaryType() const { return m_dictionaryType; }
	void setDictionaryType(eCharucoDictionaryType dictionaryType);

protected:
	MikanMarkerID m_nextMarkerId = 0;
	eCharucoDictionaryType m_dictionaryType = eCharucoDictionaryType::DICT_6X6;
	std::vector<MarkerDefinitionPtr> m_markerList;
};