#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanComponentTypes.h"
#include "MikanMathTypes.h"
#include "SerializableList.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanTrackingVolumeTypes.rfkh.h"
#endif

#include <assert.h>

// Enums
enum ENUM(Serialization::CodeGenModule("MikanTrackingVolumeTypes")) MikanTrackingVolumeType
{
	MikanTrackingVolumeType_INVALID ENUMVALUE_STRING("INVALID") = -1,
	MikanTrackingVolumeType_marker ENUMVALUE_STRING("marker"),
	MikanTrackingVolumeType_vr ENUMVALUE_STRING("vr"),
};

enum ENUM(Serialization::CodeGenModule("MikanTrackingVolumeTypes")) MikanTrackingRuntime
{
	MikanTrackingRuntime_INVALID ENUMVALUE_STRING("INVALID") = -1,
	MikanTrackingRuntime_SteamVR ENUMVALUE_STRING("SteamVR"),
};

// Structures
struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanTrackingVolumeTypes")) MikanTrackingVolumeComponentValues :
	public MikanComponentValues
{
	FIELD()
	MikanMarkerID origin_marker_id;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanTrackingVolumeComponentValues_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanTrackingVolumeTypes")) MikanMarkerTrackingVolumeComponentValues :
	public MikanTrackingVolumeComponentValues
{
	static const char* k_componentClassName;
	static const char* k_ownerSystemName;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanMarkerTrackingVolumeComponentValues_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanTrackingVolumeTypes")) MikanVRTrackingVolumeComponentValues :
	public MikanTrackingVolumeComponentValues
{
	static const char* k_componentClassName;
	static const char* k_ownerSystemName;

	FIELD()
	MikanTrackingRuntime tracking_runtime;
	FIELD()
	MikanTrackingMountID charuco_mount_id;
	FIELD()
	MikanVector3f charuco_mount_offset_mm;
	FIELD()
	MikanMarkerID utility_marker_id;
	FIELD()
	Serialization::List<MikanTrackingMountID> tracking_mount_ids;
	FIELD()
	MikanMatrix4f vr_device_pose_offset;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVRTrackingVolumeComponentValues_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanTrackingVolumeTypes_GENERATED
#endif
