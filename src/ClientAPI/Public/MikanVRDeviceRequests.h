#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanVRDeviceTypes.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanVRDeviceRequests.rfkh.h"
#endif

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

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanVRDeviceRequests_GENERATED
#endif