#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanLightTypes.h"
#include "SerializableList.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanLightRequests.rfkh.h"
#endif

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightRequests")) SetLightDMXDataSubcription
	: public MikanRequest
{
	SetLightDMXDataSubcription(){MIKAN_REQUEST_TYPE_INFO_INIT(SetLightDMXDataSubcription)}

	FIELD() MikanLightID light_id= INVALID_MIKAN_ID;

	FIELD() bool subscribe= false;

#ifdef MIKANAPI_REFLECTION_ENABLED
	SetLightDMXDataSubcription_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightRequests")) SetLightDMXData : public MikanRequest
{
	SetLightDMXData(){MIKAN_REQUEST_TYPE_INFO_INIT(SetLightDMXData)}

	FIELD() MikanLightID light_id= INVALID_MIKAN_ID;

	FIELD() MikanDMXData dmx_data;

#ifdef MIKANAPI_REFLECTION_ENABLED
	SetLightDMXData_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightRequests")) GetLightDMXData : public MikanRequest
{
	GetLightDMXData(){MIKAN_REQUEST_TYPE_INFO_INIT(GetLightDMXData)}

	FIELD() MikanLightID light_id= INVALID_MIKAN_ID;

#ifdef MIKANAPI_REFLECTION_ENABLED
	GetLightDMXData_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanLightRequests")) MikanLightDMXDataResponse
	: public MikanResponse
{
	MikanLightDMXDataResponse(){MIKAN_RESPONSE_TYPE_INFO_INIT(MikanLightDMXDataResponse)}

	FIELD() MikanLightID light_id= INVALID_MIKAN_ID;

	FIELD() MikanDMXData dmx_data;

#ifdef MIKANAPI_REFLECTION_ENABLED
	MikanLightDMXDataResponse_GENERATED
#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanLightRequests_GENERATED
#endif
