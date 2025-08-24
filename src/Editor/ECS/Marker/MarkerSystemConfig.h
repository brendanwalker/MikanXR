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

	// ArUco Settings
	static const std::string k_arucoMarkerListPropertyId;
	MarkerDefinitionPtr getMarkerConfig(MikanMarkerID markerId) const;
	MarkerDefinitionPtr getMarkerConfigByName(const std::string& MarkerName) const;
	MikanMarkerID addNewMarker(const std::string& markerName);
	bool removeMarker(MikanMarkerID markerId);

	static const std::string k_arucoDictionaryTypePropertyId;
	inline eCharucoDictionaryType getArucoDictionaryType() const { return m_arucoDictionaryType; }
	void setArucoDictionaryType(eCharucoDictionaryType dictionaryType);

	// ChArUco Settings
	static const std::string k_charucoRowsPropertyId;	
	inline int getCharucoRows() const { return m_charucoRows; }
	void setCharucoRows(int charucoRows);
	
	static const std::string k_charucoColsPropertyId;
	inline int getCharucoCols() const { return m_charucoCols; }
	void setCharucoCols(int charucoCols);
	
	static const std::string k_charucoSquareLengthMMPropertyId;
	inline float getCharucoSquareLengthMM() const { return m_charucoSquareLengthMM; }
	void setCharucoSquareLengthMM(float charucoSquareLengthMM);
	
	static const std::string k_charucoMarkerLengthMMPropertyId;
	inline float getCharucoMarkerLengthMM() const { return m_charucoMarkerLengthMM; }
	void setCharucoMarkerLengthMM(float charucoMarkerLengthMM);
	
	static const std::string k_charucoDictionaryTypePropertyId;
	inline eCharucoDictionaryType getCharucoDictionaryType() const { return m_charucoDictionaryType; }
	void setCharucoDictionaryType(eCharucoDictionaryType charucoDictionaryType);

protected:
	MikanMarkerID m_nextMarkerId = 0;

	// ArUco Common settings
	eCharucoDictionaryType m_arucoDictionaryType = DEFAULT_ARUCO_DICTIONARY_TYPE;

	// ChAruco Common settings
	int m_charucoRows = CHARUCO_PATTERN_H;
	int m_charucoCols = CHARUCO_PATTERN_W;
	float m_charucoSquareLengthMM = DEFAULT_CHARUCO_SQUARE_LEN_MM;
	float m_charucoMarkerLengthMM = DEFAULT_CHARUCO_MARKER_LEN_MM;
	eCharucoDictionaryType m_charucoDictionaryType = DEFAULT_CHARUCO_DICTIONARY_TYPE;

	// List of ArUco markers
	std::vector<MarkerDefinitionPtr> m_arucoMarkerList;
};