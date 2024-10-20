#pragma once

#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableList.h"

#ifdef REFLECTION_CODE_BUILT
#include "MikanSpatialAnchorTypes.rfkh.h"
#endif

// Spatial Anchor Response Types
struct STRUCT() MikanSpatialAnchorList : public MikanResponse
{
	inline static const std::string k_typeName = "MikanSpatialAnchorList";

	FIELD()
	Serialization::List<MikanSpatialAnchorID> spatial_anchor_id_list;

	MikanSpatialAnchorList() : MikanResponse(k_typeName) {}

	#ifdef REFLECTION_CODE_BUILT
	MikanSpatialAnchorList_GENERATED
	#endif
};

struct STRUCT() MikanSpatialAnchorInfo : public MikanResponse
{
	inline static const std::string k_typeName = "MikanSpatialAnchorInfo";

	FIELD()
	MikanSpatialAnchorID anchor_id;
	FIELD()
	MikanTransform world_transform; // Transform in tracking system space
	FIELD()
	std::string anchor_name;

	MikanSpatialAnchorInfo() : MikanResponse(k_typeName) {}

	#ifdef REFLECTION_CODE_BUILT
	MikanSpatialAnchorInfo_GENERATED
	#endif
};

#ifdef REFLECTION_CODE_BUILT
File_MikanSpatialAnchorTypes_GENERATED
#endif