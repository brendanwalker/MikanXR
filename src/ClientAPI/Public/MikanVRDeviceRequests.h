#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanVRDeviceTypes.h"
#include "SerializationProperty.h"
#include "SerializableList.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanVRDeviceRequests.rfkh.h"
#endif

// VR Device Request Types
// ------

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanScriptRequest")) GetVRDeviceList :
	public MikanRequest
{
public:
	inline static const char* k_typeName = "GetVRDeviceList";
	GetVRDeviceList() : MikanRequest(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetVRDeviceList_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanScriptRequest")) GetVRDeviceInfo :
	public MikanRequest
{
public:
	inline static const char* k_typeName = "GetVRDeviceInfo";
	GetVRDeviceInfo() : MikanRequest(k_typeName) {}

	FIELD()
	MikanVRDeviceID deviceId;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	GetVRDeviceInfo_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanScriptRequest")) SubscribeToVRDevicePoseUpdates :
	public MikanRequest
{
public:
	inline static const char* k_typeName = "SubscribeToVRDevicePoseUpdates";
	SubscribeToVRDevicePoseUpdates() : MikanRequest(k_typeName) {}

	FIELD()
	MikanVRDeviceID deviceId;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	SubscribeToVRDevicePoseUpdates_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanScriptRequest")) UnsubscribeFromVRDevicePoseUpdates :
	public MikanRequest
{
public:
	inline static const char* k_typeName = "UnsubscribeFromVRDevicePoseUpdates";
	UnsubscribeFromVRDevicePoseUpdates() : MikanRequest(k_typeName) {}

	FIELD()
	MikanVRDeviceID deviceId;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	UnsubscribeFromVRDevicePoseUpdates_GENERATED
	#endif
};

// VR Device Response Types
// ------

struct STRUCT(Serialization::CodeGenModule("MikanScriptRequest")) MikanVRDeviceListResponse : 
	public MikanResponse
{
	inline static const std::string k_typeName = "MikanVRDeviceListResponse";

	FIELD()
	Serialization::List<MikanVRDeviceID> vr_device_id_list;

	MikanVRDeviceListResponse() : MikanResponse(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVRDeviceListResponse_GENERATED
	#endif
};

struct STRUCT(Serialization::CodeGenModule("MikanScriptRequest")) MikanVRDeviceInfoResponse : 
	public MikanResponse
{
	inline static const std::string k_typeName = "MikanVRDeviceInfo";

	FIELD()
	MikanVRDeviceInfo vr_device_info;

	MikanVRDeviceInfoResponse() : MikanResponse(k_typeName) {}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanVRDeviceInfoResponse_GENERATED
	#endif
};


#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanVRDeviceRequests_GENERATED
#endif