#pragma once

#include "CommonConfig.h"

#include <map>
#include <memory>
#include <string>

#include <glm/glm.hpp>
#include <glm/ext/matrix_float4x4.hpp>

class CameraConfig : public CommonConfig
{
public:
	CameraConfig(const std::string& configName)
		: CommonConfig(configName)
	{}

	virtual configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	AnchorDefinitionPtr getSpatialAnchorConfig(MikanSpatialAnchorID anchorId) const;
	AnchorDefinitionPtr getSpatialAnchorConfigByName(const std::string& anchorName) const;
	MikanSpatialAnchorID addNewAnchor(const std::string& anchorName, const struct MikanTransform& xform);
	bool removeAnchor(MikanSpatialAnchorID anchorId);

	static const std::string k_anchorVRDevicePathPropertyId;
	std::string anchorVRDevicePath;

	static const std::string k_anchorListPropertyId;
	std::vector<AnchorDefinitionPtr> spatialAnchorList;

	MikanSpatialAnchorID nextAnchorId = 0;
};
