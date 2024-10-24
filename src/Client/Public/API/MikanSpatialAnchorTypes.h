#pragma once

#include "MikanExport.h"
#include "MikanAPITypes.h"
#include "MikanMathTypes.h"
#include "SerializableList.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanSpatialAnchorTypes.rfkh.h"
#endif

// Spatial Anchor Response Types
struct STRUCT() MikanSpatialAnchorList : public MikanResponse
{
	inline static const std::string k_typeName = "MikanSpatialAnchorList";

	FIELD()
	Serialization::List<MikanSpatialAnchorID> spatial_anchor_id_list;

	MikanSpatialAnchorList() : MikanResponse(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
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
	Serialization::String anchor_name;

	MikanSpatialAnchorInfo() : MikanResponse(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanSpatialAnchorInfo_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanSpatialAnchorTypes_GENERATED
#endif