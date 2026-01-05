#pragma once

#include "MikanAPIExport.h"
#include "MikanAPITypes.h"
#include "MikanPropertyTypes.h"
#include "MikanVariantTypes.h"
#include "SerializationProperty.h"

#ifdef MIKANAPI_REFLECTION_ENABLED
#include "MikanPropertyRequests.rfkh.h"
#endif

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanPropertyRequests")) PropertySetValueRequest :
	public MikanRequest
{
public:
	PropertySetValueRequest()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(PropertySetValueRequest)
	}

	FIELD()
	Serialization::String ownerSystem;
	FIELD()
	int componentId;
	FIELD()
	Serialization::String fieldName;
	FIELD()
	MikanVariant fieldValue;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	PropertySetValueRequest_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanPropertyRequests")) PropertySetValueResponse :
	public MikanResponse
{
public:
	PropertySetValueResponse()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(PropertySetValueResponse)
	}

	#ifdef MIKANAPI_REFLECTION_ENABLED
	PropertySetValueResponse_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanPropertyRequests")) PropertyGetValueRequest :
	public MikanRequest
{
public:
	PropertyGetValueRequest()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(PropertyGetValueRequest)
	}

	FIELD()
	Serialization::String ownerSystem;
	FIELD()
	int componentId;
	FIELD()
	Serialization::String fieldName;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	PropertyGetValueRequest_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanPropertyRequests")) PropertyGetValueResponse :
	public MikanResponse
{
public:
	PropertyGetValueResponse()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(PropertyGetValueResponse)
	}

	FIELD()
	MikanPropertyValue propertyValue;

	#ifdef MIKANAPI_REFLECTION_ENABLED
	PropertyGetValueResponse_GENERATED
	#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanPropertyRequests")) SetPropertyNotifyMode :
	public MikanRequest
{
public:
	SetPropertyNotifyMode()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(SetPropertyNotifyMode)
	}

	FIELD()
	Serialization::String systemFilter;

	FIELD()
	Serialization::String componentFilter;

	FIELD()
	Serialization::String propertyFilter;

	FIELD()
	MikanPropertyNotifyMode notifyMode;

#ifdef MIKANAPI_REFLECTION_ENABLED
	SetPropertyNotifyMode_GENERATED
#endif
};

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanPropertyRequests")) GetPropertyDescriptors :
	public MikanRequest
{
public:
	GetPropertyDescriptors()
	{
		MIKAN_REQUEST_TYPE_INFO_INIT(GetPropertyDescriptors)
	}

	FIELD()
	Serialization::String systemFilter;

	FIELD()
	Serialization::String componentFilter;

	FIELD()
	Serialization::String propertyFilter;

#ifdef MIKANAPI_REFLECTION_ENABLED
	GetPropertyDescriptors_GENERATED
#endif
};

// Property Response Types
// ------

struct MIKAN_API STRUCT(Serialization::CodeGenModule("MikanPropertyRequests")) PropertyDescriptorResponse :
	public MikanResponse
{
	PropertyDescriptorResponse()
	{
		MIKAN_RESPONSE_TYPE_INFO_INIT(PropertyDescriptorResponse)
	}

	FIELD()
	Serialization::List<MikanPropertyDescriptor> descriptor_list;

#ifdef MIKANAPI_REFLECTION_ENABLED
	PropertyDescriptorResponse_GENERATED
#endif
};

#ifdef MIKANAPI_REFLECTION_ENABLED
File_MikanPropertyRequests_GENERATED
#endif
