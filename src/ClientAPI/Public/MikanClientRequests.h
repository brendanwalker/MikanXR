#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanClientTypes.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanClientRequests.rfkh.h"
#endif

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanClientRequests")) ConnectRequest :
	public MikanRequest
{
public:
	FIELD()
	MikanClientInfo clientInfo;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	ConnectRequest_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanClientRequests")) DisconnectRequest :
	public MikanRequest
{
public:
	#ifdef MIKANAPI_REFLECTION_ENABLED
	DisconnectRequest_GENERATED
	#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanClientRequests_GENERATED
#endif