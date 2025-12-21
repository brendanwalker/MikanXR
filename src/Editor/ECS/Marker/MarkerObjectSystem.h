#pragma once

#include "CommonConfig.h"
#include "ComponentFwd.h"
#include "MarkerComponent.h"
#include "MikanObjectSystem.h"
#include "MikanTypeFwd.h"
#include "MulticastDelegate.h"
#include "ObjectSystemConfigFwd.h"
#include "ObjectSystemFwd.h"
#include "ProjectConfigConstants.h"

#include <map>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp>

using MarkerMap = std::map<MikanMarkerID, MarkerComponentWeakPtr>;

class MarkerObjectSystemConfig : public MikanObjectSystemDefinition
{
public:
	MarkerObjectSystemConfig(const std::string& configName)
		: MikanObjectSystemDefinition(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	// ArUco Settings
	static const std::string k_arucoMarkerListPropertyId;
	MarkerDefinitionPtr getMarkerConfig(MikanMarkerID markerId) const;
	MarkerDefinitionPtr getMarkerConfigByName(const std::string& MarkerName) const;
	MikanMarkerID addNewMarker();
	MikanMarkerID addNewMarker(const std::string& markerName);
	bool removeMarker(MikanMarkerID markerId);
	const std::vector<MarkerDefinitionPtr>& getArucoMarkerList() const { return m_arucoMarkerList; }

	static const std::string k_arucoDictionaryTypePropertyId;
	inline eCharucoDictionaryType getArucoDictionaryType() const { return m_arucoDictionaryType; }
	void setArucoDictionaryType(eCharucoDictionaryType dictionaryType);

	static const std::string k_arucoIdListPropertyId;
	void getArucoIdList(std::vector<int>& outMarkerIdList) const;

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

	MikanMarkerID nextMarkerId = 0;

protected:
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

class MarkerObjectSystem : public MikanObjectSystem
{
public:
	MarkerObjectSystem(class ProjectManager* ownerObjectSystemManager) : MikanObjectSystem(ownerObjectSystemManager) {}

	inline static const std::string k_objectSystemClassName = "MarkerObjectSystem";
	virtual std::string getObjectSystemClassName() const { return k_objectSystemClassName; }

	virtual bool init() override;
	virtual void dispose() override;
	virtual void deleteObjectConfig(MikanObjectPtr objectPtr) override;

	virtual MikanObjectSystemDefinitionConstPtr getObjectSystemConfigConst() const override {
		return getMarkerSystemConfigConst();
	}
	MarkerObjectSystemConfigConstPtr getMarkerSystemConfigConst() const;
	MarkerObjectSystemConfigPtr getMarkerSystemConfig();

	const MarkerMap& getMarkerMap() const { return m_markerComponents; }
	MarkerComponentPtr getMarkerById(MikanMarkerID markerId) const;
	MarkerComponentPtr getMarkerByName(const std::string& markerName) const;
	MarkerComponentPtr addNewMarker();
	bool removeMarker(MikanMarkerID markerId);

	// -- IRmlPropertyInterface ----
	static void getRmlPropertyDescriptors(std::vector<RmlPropertyDescriptorConstPtr>& outDescriptors);
	virtual bool getPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, Rml::Variant& outValue) const override;
	virtual bool setPropertyValueFromRml(RmlPropertyDescriptorConstPtr propertyDesc, const Rml::Variant& inValue) override;

	// -- IRmlFunctionInterface ----
	static const std::string k_printCharucoMarkerFunctionId;
	static void getRmlFunctionDescriptors(std::vector<RmlFunctionDescriptorConstPtr>& outDescriptors);
	virtual bool invokeFunctionFromRml(RmlFunctionDescriptorConstPtr functionDesc) override;

protected:
	MarkerComponentPtr createMarkerObject(MarkerDefinitionPtr markerConfig);
	void disposeMarkerObject(MikanMarkerID markerId);

	void printMarker();

	MarkerMap m_markerComponents;
};