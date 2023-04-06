#pragma once

#include "CommonConfig.h"
#include "MikanClientTypes.h"

#include <memory>
#include <string>

#include <glm/ext/matrix_float4x4.hpp>

class AnchorObjectSystemConfig : public CommonConfig
{
public:
	AnchorObjectSystemConfig(const std::string& fnamebase = "AnchorObjectSystemConfig")
		: CommonConfig(fnamebase)
	{}

	virtual const configuru::Config writeToJSON();
	virtual void readFromJSON(const configuru::Config& pt);

	bool canAddAnchor() const;
	const MikanSpatialAnchorInfo* getSpatialAnchorInfo(MikanSpatialAnchorID anchorId) const;
	const MikanSpatialAnchorInfo* getSpatialAnchorInfoByName(const std::string& anchorName) const;
	MikanSpatialAnchorID addNewAnchor(const std::string& anchorName, const MikanMatrix4f& xform);
	bool removeAnchor(MikanSpatialAnchorID anchorId);

	std::string anchorVRDevicePath;
	std::vector<MikanSpatialAnchorInfo> spatialAnchorList;
	MikanSpatialAnchorID nextAnchorId;
	MikanSpatialAnchorID originAnchorId;
	bool debugRenderAnchors;
}; 
