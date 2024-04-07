#pragma once

#include "MikanAPITypes.h"
#include "MikanMathTypes.h"

#define ORIGIN_SPATIAL_ANCHOR_NAME		"Origin"

// Spatial Anchor Response Types
struct MikanSpatialAnchorList : public MikanResponse
{
	inline static const std::string k_typeName = "MikanSpatialAnchorList";

	std::vector<MikanSpatialAnchorID> spatial_anchor_id_list;

	MikanSpatialAnchorList() : MikanResponse(k_typeName) {}
};

struct MikanSpatialAnchorInfo : public MikanResponse
{
	inline static const std::string k_typeName = "MikanSpatialAnchorInfo";

	MikanSpatialAnchorID anchor_id;
	MikanTransform world_transform; // Transform in tracking system space
	std::string anchor_name;

	MikanSpatialAnchorInfo() : MikanResponse(k_typeName) {}
};